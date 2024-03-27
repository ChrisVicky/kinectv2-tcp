#ifndef TCP_HPP
#define TCP_HPP
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

class TCPClient {
private:
  int sockfd;
  struct sockaddr_in serv_addr;
  bool is_initialized;
  bool is_connected;
  std::string address;
  int port;

public:
  TCPClient() : sockfd(-1), is_initialized(false), is_connected(false) {
    memset(&serv_addr, 0, sizeof(serv_addr));
  }

  bool Init() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
      std::cerr << "ERROR opening socket" << std::endl;
      return false;
    }
    is_initialized = true;
    return true;
  }

  bool Connect(const std::string &address, int port) {
    this->address = address;
    this->port = port;
    if (!is_initialized) {
      std::cerr << "Socket not initialized." << std::endl;
      return false;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address.c_str(), &serv_addr.sin_addr) <= 0) {
      std::cerr << "Invalid address/ Address not supported" << std::endl;
      return false;
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
      std::cerr << "Connection Failed" << std::endl;
      return false;
    }

    std::cout << "Connected to " << address << ":" << port << std::endl;
    is_connected = true;
    return true;
  }

  bool Put(const void *image, size_t size) {
    if (!is_connected) {
      std::cout << "TCP Client is not connected to a server." << std::endl;
      return false;
    }

    if (send(sockfd, image, size, 0) < 0) {
      std::cerr << "Failed to send image data." << std::endl;

      std::cout << "Perform Auto Reconnetion" << std::endl;
      Connect(address, port);

      return false;
    }
    return true;
  }

  ~TCPClient() {
    if (sockfd >= 0) {
      close(sockfd);
    }
  }
};
#endif // !TCP_HPP
