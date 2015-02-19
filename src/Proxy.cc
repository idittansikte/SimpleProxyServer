#include "Proxy.h"

//#define HTTP_PORT "80"

Proxy::Proxy()
{}

// get sockaddr, IPv4 or IPv6:
void *Proxy::get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void Proxy::SendAll(const int socket, const std::string &http_message ){
  const char * buffer = http_message.c_str();
  
  size_t size = http_message.size();
  size_t i = 0;
  while( i < size ){
    int bytesent = send( socket, buffer, size - i , MSG_NOSIGNAL );
    
    if( bytesent == -1 ){
      perror("send");
      exit(1);
    }
    else{
      i += bytesent;
    }
  }
}
