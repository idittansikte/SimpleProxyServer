#ifndef PROXYCLIENT_H
#define PROXYCLIENT_H

#include <string>
#include "Proxy.h"

class ProxyClient : public Proxy {
public:
    
  int EstablishConnection( const std::string host, char* port);

  void *get_in_addr(struct sockaddr *sa);
  // Returns the recieved char* message 
  int Send( const int socket_server, const std::string &message );

  int Recieve(int socket_server, std::string &message);
  
  int FormatHttpResponse( std::string &message );
  
  std::string SendAndRecieveRequest( const std::string &host, char *port, const std::string &client_request_message );

private:
  //char* m_sock_fd;
};

#endif
