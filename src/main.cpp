#include <algorithm>
#include <cctype>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

volatile bool server_running = true;

void signal_handler(int signal)
{
  std::cout << "\nShutting down server...\n";
  server_running = false;
}

struct Header
{
  std::string name;
  std::string value;
};

std::string to_lower(const std::string &input)
{
  std::string output = input;
  std::transform(output.begin(), output.end(), output.begin(),
                 [](unsigned char c)
                 { return static_cast<char>(std::tolower(c)); });
  return output;
}

void trim_in_place(std::string &value)
{
  size_t start = value.find_first_not_of(" \t");
  if (start == std::string::npos)
  {
    value.clear();
    return;
  }
  size_t end = value.find_last_not_of(" \t");
  value = value.substr(start, end - start + 1);
}

int main(int argc, char **argv)
{
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

#ifdef _WIN32
  WSADATA wsa_data;
  if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
  {
    std::cerr << "WSAStartup failed\n";
    return 1;
  }
#else
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);
#endif

  // You can use print statements as follows for debugging, they'll be visible
  // when running tests.
  std::cout << "Logs from your program will appear here!\n";

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0)
  {
    std::cerr << "Failed to create server socket\n";
    return 1;
  }

  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) <
      0)
  {
    std::cerr << "setsockopt failed\n";
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) !=
      0)
  {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0)
  {
    std::cerr << "listen failed\n";
    return 1;
  }

  std::cout << "Server listening on port 4221...\n";
  std::cout << "Press Ctrl+C to stop the server\n";

  while (server_running)
  {
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                           (socklen_t *)&client_addr_len);
    if (client_fd < 0)
    {
      if (server_running)
      {
        std::cerr << "Failed to accept client connection\n";
      }
      continue;
    }

    std::cout << "Client connected\n";

    // Read the HTTP request until headers are complete
    std::string request;
    char buffer[1024] = {0};
    int bytes_received = 0;
    bool read_failed = false;
    bool send_bad_request = false;

    while (true)
    {
      bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
      if (bytes_received <= 0)
      {
        read_failed = true;
        send_bad_request = !request.empty();
        break;
      }

      request.append(buffer, bytes_received);
      if (request.find("\r\n\r\n") != std::string::npos)
      {
        break;
      }

      if (request.size() > 16384)
      {
        read_failed = true;
        send_bad_request = true;
        break;
      }
    }

    if (read_failed)
    {
      std::cerr << "Failed to read request from client\n";
      if (send_bad_request)
      {
        const char *error_response = "HTTP/1.1 400 Bad Request\r\n\r\n";
        send(client_fd, error_response, strlen(error_response), 0);
      }
#ifdef _WIN32
      closesocket(client_fd);
#else
      close(client_fd);
#endif
      continue;
    }

    size_t header_end = request.find("\r\n\r\n");
    size_t line_end = request.find("\r\n");

    if (header_end == std::string::npos || line_end == std::string::npos || line_end > header_end)
    {
      std::cerr << "Malformed HTTP request\n";
      const char *error_response = "HTTP/1.1 400 Bad Request\r\n\r\n";
      send(client_fd, error_response, strlen(error_response), 0);
#ifdef _WIN32
      closesocket(client_fd);
#else
      close(client_fd);
#endif
      continue;
    }

    std::string request_line = request.substr(0, line_end);
    size_t first_space = request_line.find(' ');
    size_t second_space = request_line.find(' ', first_space + 1);

    if (first_space == std::string::npos || second_space == std::string::npos)
    {
      std::cerr << "Malformed HTTP request\n";
      const char *error_response = "HTTP/1.1 400 Bad Request\r\n\r\n";
      send(client_fd, error_response, strlen(error_response), 0);
#ifdef _WIN32
      closesocket(client_fd);
#else
      close(client_fd);
#endif
      continue;
    }

    std::string path = request_line.substr(first_space + 1, second_space - first_space - 1);

    std::vector<Header> header_list;
    std::unordered_map<std::string, std::string> headers;

    size_t header_start = line_end + 2;
    std::string headers_block = request.substr(header_start, header_end - header_start);
    size_t pos = 0;

    while (pos < headers_block.size())
    {
      size_t line_break = headers_block.find("\r\n", pos);
      if (line_break == std::string::npos)
      {
        line_break = headers_block.size();
      }

      std::string line = headers_block.substr(pos, line_break - pos);
      if (!line.empty())
      {
        size_t colon = line.find(':');
        if (colon != std::string::npos)
        {
          std::string name = line.substr(0, colon);
          std::string value = line.substr(colon + 1);
          trim_in_place(name);
          trim_in_place(value);
          if (!name.empty())
          {
            header_list.push_back({name, value});
            headers[to_lower(name)] = value;
          }
        }
      }

      if (line_break == headers_block.size())
      {
        break;
      }
      pos = line_break + 2;
    }

    std::string user_agent;
    auto user_agent_it = headers.find("user-agent");
    if (user_agent_it != headers.end())
    {
      user_agent = user_agent_it->second;
    }

    long content_length_value = 0;
    auto content_length_it = headers.find("content-length");
    if (content_length_it != headers.end())
    {
      char *endptr = nullptr;
      content_length_value = std::strtol(content_length_it->second.c_str(), &endptr, 10);
      if (endptr == content_length_it->second.c_str() || content_length_value < 0)
      {
        content_length_value = 0;
      }
    }

    // Prepare response based on path
    std::string response_body;
    std::string status_line;
    std::string content_type = "text/html";

    if (path == "/")
    {
      status_line = "HTTP/1.1 200 OK\r\n";
      response_body = "<h1>Welcome to HTTP Server</h1>\n<p>This is the home page.</p>";
      if (!user_agent.empty())
      {
        response_body += "\n<p><strong>User-Agent:</strong> " + user_agent + "</p>";
      }
      if (content_length_value > 0)
      {
        response_body += "\n<p><strong>Content-Length:</strong> " +
                         std::to_string(content_length_value) + "</p>";
      }
      if (!header_list.empty())
      {
        response_body += "\n<h2>Headers</h2>\n<pre>";
        for (const auto &header : header_list)
        {
          response_body += header.name + ": " + header.value + "\n";
        }
        response_body += "</pre>";
      }
      content_type = "text/html";
    }
    else if (path == "/api")
    {
      status_line = "HTTP/1.1 200 OK\r\n";
      response_body = "{\"message\": \"Hello from API\", \"status\": \"success\"}";
      content_type = "application/json";
    }
    else
    {
      status_line = "HTTP/1.1 404 Not Found\r\n";
      response_body = "<h1>404 Not Found</h1>\n<p>The requested path does not exist.</p>";
      content_type = "text/html";
    }

    // Build complete response with headers
    std::string content_length = std::to_string(response_body.length());
    std::string full_response = status_line +
                                "Content-Type: " + content_type + "\r\n" +
                                "Content-Length: " + content_length + "\r\n" +
                                "\r\n" +
                                response_body;

    send(client_fd, full_response.c_str(), full_response.length(), 0);

// Close only the client connection, NOT the server
#ifdef _WIN32
    closesocket(client_fd);
#else
    close(client_fd);
#endif

    std::cout << "Client disconnected, waiting for next client...\n";
  }

// Cleanup (never reached in infinite loop)
#ifdef _WIN32
  closesocket(server_fd);
  WSACleanup();
#else
  close(server_fd);
#endif

  return 0;
}
