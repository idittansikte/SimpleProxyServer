#include "ProxyServer.h"

#define MAX_DATASIZE 4194304

#define PORT "4475"

ProxyServer::~ProxyServer()
{
    
}

void ProxyServer::sigchld_handler(int s)
{
  while(waitpid(-1, NULL, WNOHANG) > 0);
}

int ProxyServer::StartListen(const char* port){
  
  int socket_listner;
  
  // Create address struct
  struct addrinfo hints, *servinfo, *res;
  struct sigaction sa;
  
  // Load address struct
  memset(&hints, 0, sizeof hints); // To make sure everything is set to 0
  hints.ai_family = AF_UNSPEC; // IPv4 or IPv6  
  hints.ai_socktype = SOCK_STREAM; // TCP
  hints.ai_flags = AI_PASSIVE; // IP
  hints.ai_protocol = 0;
  
  int error = getaddrinfo(NULL, PORT, &hints, &servinfo);
  if(error != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
    exit(1);
  }

  // loop through all results and bind the first we can...
  for(res = servinfo; res != nullptr; res = res->ai_next){
    
    socket_listner = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if( socket_listner == -1 ){
      perror("server: socket");
      continue;
    }
    
    
    // Set socket options
    int yes = 1;
    int sockopt = setsockopt(socket_listner, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if(sockopt == -1){
      perror("setsockopt");
      exit(1);
    }
    
    int b = bind(socket_listner, res->ai_addr, res->ai_addrlen);
    if(b == -1)
      {
	close(socket_listner);
	perror("server: bind");
	continue;
      }
    
    break;
    
  }
  
  if( res == NULL ){
    fprintf(stderr, "server: failed to bind\n");
    return 3;
  }
  
  freeaddrinfo(servinfo); // all done with this structure
  
  if( listen( socket_listner, 10) == -1 ){ // 10 ==  how many pending connections queue will hold
    perror("listen");
    return 4;
  }
  
  
  
  std::cout << ">> Listener successfully created.\n";
  return socket_listner;
}

int ProxyServer::AcceptConnections(int socket_listner){

  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  char s[INET6_ADDRSTRLEN];
  int socket_client;
    
  std::cout << ">> Waiting for connections...\n";
    

  sin_size = sizeof their_addr;
  
  socket_client = accept( socket_listner, (struct sockaddr *)&their_addr, &sin_size );
  
  if( socket_client == -1){
    perror("accept");
    return -1;
  }
  
  inet_ntop( their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s );
  printf("server: got connection from %s\n", s);
    

  return socket_client;

} // End of function

void ProxyServer::RecieveMessage( std::string &message, const int client_socket ){
  
  int numbytes{};
  char buffer[MAX_DATASIZE];
  numbytes = recv(client_socket, buffer, MAX_DATASIZE-1, 0);
  
  if( numbytes == -1 ){
    perror("recv");
    //return char();
  }

  if(numbytes == 0){ // Connection is closed
    close(client_socket);
    exit(1);
  }

  buffer[numbytes] = '\0';

  
  message = std::string(buffer);
  
  //std::cout << "Number of bytes receved from client: " << numbytes << "| Buffer string: " << buffer << '\n';
  
  if( !IsValid(buffer) ){
    std::cerr << "Cencured material!\n";
    //return char();
  }
  
  std::cout << "Server: Recieve successfully!\n";
  //return buffer;

}

bool ProxyServer::IsValid( const char* message ){
  
  return true;

}

int ProxyServer::Send( const int socket_client, const std::string &message, const int MAX_SIZE ){
  
  //std::cout << "Server: Send: size of message = " << size << " \n Content:\n" << message << '\n';
  const char* buffer = message.c_str();
  size_t size = message.size();
  
  int error = send( socket_client, buffer, size, 0 );
  //int error = send( socket_client, "HEY!", 4, 0 );
  if( error == -1 ){
    perror("send");
    return 1;
  }
  close(socket_client);
  std::cout << "Server: Send Successfully!\n"; 
  
  return 0;
}
