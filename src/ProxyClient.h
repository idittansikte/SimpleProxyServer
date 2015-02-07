#ifndef PROXYCLIENT_H
#define PROXYCLIENT_H

#include <string>
#include "Proxy.h"

class ProxyClient : public Proxy {
public:
    
  int EstablishConnection(int &socket_server, const std::string address, char* port);

  void *get_in_addr(struct sockaddr *sa);
  // Returns the recieved char* message 
  int Send( const int socket_server, const std::string &message, const int MAX_SIZE );

  int Recieve(int socket_server, std::string &message, const int MAX_SIZE);

private:
  //char* m_sock_fd;
};

#endif
