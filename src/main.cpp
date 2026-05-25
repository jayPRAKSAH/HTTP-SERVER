#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
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

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  #ifdef _WIN32
  WSADATA wsa_data;
  if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
    std::cerr << "WSAStartup failed\n";
    return 1;
  }
  #endif

  // You can use print statements as follows for debugging, they'll be visible
  // when running tests.
  std::cout << "Logs from your program will appear here!\n";

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    std::cerr << "Failed to create server socket\n";
    return 1;
  }

  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) <
      0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) !=
      0) {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }

  std::cout << "Server listening on port 4221...\n";

  while (true) {
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                           (socklen_t *)&client_addr_len);
    if (client_fd < 0) {
      std::cerr << "Failed to accept client connection\n";
      continue;
    }

    std::cout << "Client connected\n";

    // Read the HTTP request
    char buffer[1024] = {0};
    recv(client_fd, buffer, sizeof(buffer), 0);

    // Parse the request line to extract the path
    // Format: GET /path HTTP/1.1\r\n
    std::string request(buffer);
    size_t first_space = request.find(' ');
    size_t second_space = request.find(' ', first_space + 1);
    std::string path = request.substr(first_space + 1, second_space - first_space - 1);

    // Determine response based on path
    const char *response;
    if (path == "/") {
      response = "HTTP/1.1 200 OK\r\n\r\n";
    } else {
      response = "HTTP/1.1 404 Not Found\r\n\r\n";
    }

    send(client_fd, response, strlen(response), 0);

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
