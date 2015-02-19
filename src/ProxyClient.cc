#include "ProxyClient.h"




int ProxyClient::EstablishConnection( const std::string host, char* port ){
  
  struct addrinfo hints, *servinfo, *p;
  char s[INET6_ADDRSTRLEN];
  int socket_server;

  memset(&hints, 0, sizeof hints);
  
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  
  char* chost = (char*)host.c_str();
  
  int error = getaddrinfo(chost, "80", &hints, &servinfo);
  
  if (error != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
    return -1;
  }

  // loop through all the results and connect to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next) {
    socket_server = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if ( socket_server == -1 ) {
      perror("client: socket");
      continue;
    }

    if (connect(socket_server, p->ai_addr, p->ai_addrlen) == -1) {
      close(socket_server);
      perror("client: connect");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return -1;
  }

  inet_ntop(p->ai_family, Proxy::get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
  printf("client: connecting to %s\n", s);

  freeaddrinfo(servinfo); // all done with this structure
  
  return socket_server;
}

int ProxyClient::Send( const int socket_server, const std::string &message){

  SendAll(socket_server, message);
}

int ProxyClient::Recieve(int socket_server, std::string &strbuffer){


  int numbytes{};
  //int totbytes{};
  int MAX_BUFFER_SIZE = 4096;  
  char buffer[MAX_BUFFER_SIZE];
  
  strbuffer.clear();

  //std::cout <<  "Recieving from server...\n";
  numbytes = recv(socket_server, buffer, sizeof buffer, 0);
    
    if ( numbytes == -1 ) {
      perror("client: recv");
      return -1;
    }
    
    //totbytes += numbytes; 
    //std::cout << "Bytes recieved: " << totbytes << '\n';
    strbuffer += std::string(buffer, buffer+numbytes);
    //strbuffer += '\0';
      
  
  //std::cout << "Client: Message recieved!\n";
  return numbytes;
  
}

int ProxyClient::FormatHttpResponse( std::string &message ){
  

  return 0;
}
