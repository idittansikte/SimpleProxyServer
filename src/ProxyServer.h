#ifndef PROXYSERVER_H
#define PROXYSERVER_H

#include <string>
#include "Proxy.h"

class ProxyServer : public Proxy {
public:

    ~ProxyServer();
    // Establish listening socket...
    int StartListen(const char* port);

  // Listen and return responding socket connections...
  int AcceptConnections(int socket_listner);
  
  // Get the client message
  void RecieveMessage( std::string &message, const int client_socket );
  
  // Send response to client
  int Send( const int socket_client, const std::string &message, const int MAX_SIZE  );
  

private:    
    void sigchld_handler(int s);

    bool IsValid(const char* message);
};

#endif
