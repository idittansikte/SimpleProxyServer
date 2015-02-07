#ifndef PROXY_H
#define PROXY_H

#include <string>
#include <stdlib.h>
#include <iostream>
#include <cstring> //Memset
#include <string>
#include <sstream>
#include <algorithm>

#include <unistd.h> // Close
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> // Already has addrinfo???
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

class Proxy {
public:
  Proxy();

  void *get_in_addr(struct sockaddr *sa);
  
  bool CheckBadWords( std::string & message );
  
  int EditGETLine( std::string &message, std::string &address );
  
private:
    bool SearchBadWords( const std::string & message );
};

#endif
