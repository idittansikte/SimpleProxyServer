#include "ProxyClient.h"




int ProxyClient::EstablishConnection(int &socket_server, const std::string address, char* port){
  
  struct addrinfo hints, *servinfo, *p;
  char s[INET6_ADDRSTRLEN];

  memset(&hints, 0, sizeof hints);
  
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  
  char* host = (char*)address.c_str();
  
  int error = getaddrinfo(host, "80", &hints, &servinfo);
  
  if (error != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
    return 1;
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
    return 2;
  }

  inet_ntop(p->ai_family, Proxy::get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
  printf("client: connecting to %s\n", s);

  freeaddrinfo(servinfo); // all done with this structure

}

int ProxyClient::Send( const int socket_server, const std::string &message, const int MAX_SIZE ){

  const char * buffer = message.c_str();
  
  
  //char exa[] = "GET / HTTP/1.1\r\nHost: www.ida.liu.se\r\nUser-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:34.0) Gecko/20100101 Firefox/34.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nAccept-Language: sv-SE,sv;q=0.8,en-US;q=0.5,en;q=0.3\r\nAccept-Encoding: gzip, deflate\r\nDNT: 1\r\nConnection: close\r\n\r\n";

  //std::cout << "Clent: send() message = \n" << buffer << '\n';
  size_t size= message.size();
  int bytesent = send( socket_server, buffer, size , MSG_NOSIGNAL );
  
  if( bytesent == -1 ){
    perror("client: send");
    return 1;
  }

  std::cout << "Client: Send successfully!\n";
}

int ProxyClient::Recieve(int socket_server, std::string &message, const int MAX_SIZE){
  int numbytes{};
  int totbytes{};
  int MAX_BUFFER_SIZE = 4096;  
  char buffer[MAX_BUFFER_SIZE];
  message.clear();

  std::cout <<  "Recieving...\n";
  while( (numbytes = recv(socket_server, buffer, sizeof buffer, 0)) > 0){
    
    if ( numbytes == -1 ) {
      perror("client: recv");
      exit(1);
    }
    
    totbytes += numbytes; 
    std::cout << "Bytes recieved: " << totbytes << '\n';
    message += std::string(buffer, buffer+numbytes);
  };
  
  message += '\0';
  
  //printf("client: received '%s'\n",message);
  
  close(socket_server);
  std::cout << "Client: Message recieved!\n";
  return 0;
  
}
