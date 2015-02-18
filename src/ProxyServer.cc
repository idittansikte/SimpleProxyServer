#include "ProxyServer.h"

#define MAX_DATASIZE 4194304

ProxyServer::~ProxyServer()
{
    
}

void sigchld_handler(int s)
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
  
  int error = getaddrinfo(NULL, port, &hints, &servinfo);
  if(error != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
    return -1;
  }

  // loop through all results and bind the first we can...
  for(res = servinfo; res != NULL; res = res->ai_next){
    
    socket_listner = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if( socket_listner == -1 ){
      perror("server: socket");
      continue;
    }
    
    
    // Set socket options
    int yes = 1;
    int sockopt = setsockopt(socket_listner, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
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
    return -1;
  }
  
  freeaddrinfo(servinfo); // all done with this structure
  
  if( listen( socket_listner, 10) == -1 ){ // 10 ==  how many pending connections queue will hold
    perror("listen");
    return -1;
  }
  
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
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

std::string ProxyServer::RecieveMessage( const int client_socket ){
  
  int numbytes{};
  int MAX_BUFFER_SIZE = 4096;  
  char buffer[MAX_BUFFER_SIZE];
  std::string message = "";
  

  std::cout <<  "Recieving...\n";
  numbytes = recv(client_socket, buffer, sizeof buffer, 0);
    
  if ( numbytes == -1 ) {
    perror("server: recv");
    return "";
  }
    
  std::cout << "Bytes recieved from client: " << numbytes << '\n';
    
  message += std::string(buffer, buffer+numbytes);
  message += '\0';
  
  std::cout << "Server: Recieve successfully!\n";

  return message;

}


int ProxyServer::Send( const int socket_client, const std::string &message ){
  
  SendAll( socket_client, message );
  
  close(socket_client);
  std::cout << "Server: Send Successfully!\n"; 
  
  return 0;
}

int ProxyServer::FormatHttpRequest( std::string &message, std::string &address ){
    
    std::string header = "Host:";
    size_t pos = message.find(header);
    if( pos != std::string::npos ){

      char hostname[50];
      std::size_t end = message.find_first_of('\r', pos);
      std::size_t len = message.copy(hostname, end - pos - header.size() - 1 ,pos + header.size() + 1 );
      hostname[len] = '\0';
      address = std::string(hostname, hostname+len);
      std::cout << "Hostname found: " << address << '\n'; 
      header = "GET";
      pos = message.find(header);
      if( pos != std::string::npos ){

	end = message.find_first_of('\r', pos);
	char array[200];
	len = message.copy(array, end - pos - header.size() - 1, pos + header.size() + 1 );
	array[len] = '\0';
	std::string tmp = std::string(array, array+len);
	std::cout << "GET line: " << tmp << '\n'; 

	std::size_t check = tmp.find(address);
	if( check != std::string::npos ){
	  message.replace( pos + header.size() + 1, check + address.size() - pos, "" );
	}
	else{
	  std::cerr << "No "<< address << " after GET.\n";
	  return -1;
	}
      }
      else{
	std::cerr << "Could not find any \"GET\" in HTTP request...!\n";
	return -1;
      }
    }
    else{
      std::cerr << "Could not find any hostname...!\n";
      return -1;
    }

    // Set connection to close...
    header = "Connection:";
    pos = message.find(header);
    if( pos != std::string::npos ){
      size_t end = message.find_first_of("\r", pos);
      message.replace( pos+header.size() + 1, end - pos - header.size() - 1, "close");
    }
    else{
      std::cerr << "Could not find Connection header in request...\n";
      return -1;
    }

    std::cout << "Here come the whole message: \n" << message;

    // message.clear();
    // message = "";
    
    // while(ss >> str){
      
    //   std::string tmp{""};
    //   std::string header;
      
    //   std::getline(ss, tmp, '\r');
      

    //   if(str == "GET" || str == "POST"){
    // 	std::string http;
    // 	std::size_t pos1 = tmp.find("http://");
    // 	if(pos1 != std::string::npos){
    // 	  std::size_t pos2 = tmp.find_first_of("/",pos1+7);
    // 	  if(pos2 != std::string::npos){
    // 	    tmp.erase(pos1, pos2-pos1);
    // 	  }
    // 	}
    // 	header = str + tmp;
    //   }
    //   else if(str == "HOST:" || str == "Host:" || str == "host:"){
    // 	header = str + tmp;
    // 	tmp.erase(remove_if(tmp.begin(), tmp.end(), isspace),tmp.end());
    // 	address = tmp;
	
    //   }
    //   else if(str == "User-Agent:" || str == "user-agent:" || str == "USER-AGENT:"){
    // 	header = str + tmp;
    //   }
    //   else if(str == "Accept:" || str == "accept:" || str == "ACCEPT:"){
    // 	header = str + tmp;
    //   }
    //   else if(str == "Accept-Language:" || str == "accept-language:" || str == "ACCEPT-LANGUAGE:"){
    // 	header = str + tmp;
    //   }
    //   else if(str == "Accept-Encoding:" || str == "accept-encoding:" || str == "ACCEPT-ENCODING:"){
    // 	header = str + tmp;
    //   }
    //   else if(str == "DNT:" || str == "dnt:" || str == "Dnt:"){
    // 	header = str + " 1";
    //   }
    //   else if(str == "Cookie:" || str == "cookie:" || str == "COOKIE:"){
    // 	header = str + tmp;
    //   }
    //   else if(str == "Referer:" || str == "referer:" || str == "REFERER:"){
    // 	header = str + tmp;
    //   }
    //   else if(str == "Content-Length:" || str == "content-lenght:"){
    // 	header = str + tmp;
    //   }
    //   else if(str == "Content-Type:" || str == "content-type:"){
    // 	header = str + tmp;
    //   }
    //   else if(str == "Connection:" || str == "connection:" || str == "CONNECTION:"){
    // 	header = str + " close";
    //   }
    //   else if(str == "\r\n" ){
    // 	message += str;
    // 	break;
    //   }
      
    //   message += header + "\r\n";
      
    //   getline(ss, tmp);
    //   str.clear();
    // }
    
    // if(!ss.eof()){
    //   message += ss.str();
    // }
    
    //message += "\r\n";
    
    if( address.empty() || message.size() < 8)
      return -1;
    //std::cout << buffer << std::endl;

  return 0;
}
