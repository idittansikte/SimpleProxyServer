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

int ProxyClient::Recieve(int socket_server, std::string &message){


  int numbytes{};
  int totbytes{};
  int MAX_BUFFER_SIZE = 4096;  
  char buffer[MAX_BUFFER_SIZE];
  
  message.clear();

  std::cout <<  "Recieving...\n";
  while( (numbytes = recv(socket_server, buffer, sizeof buffer, 0)) > 0){
    
    if ( numbytes == -1 ) {
      perror("client: recv");
      return -1;
    }
    
    totbytes += numbytes; 
    std::cout << "Bytes recieved: " << totbytes << '\n';
    message += std::string(buffer, buffer+numbytes);
      
  }
  
  message += '\0';
  
  std::cout << "Client: Message recieved!\n";
  return 0;
  
}

int ProxyClient::FormatHttpResponse( std::string &message ){
  
//    std::string str, get, host, usr_agent, accept, accept_lang, accept_enc, dnt, referer, connection = "";
//  
//    std::istringstream ss(message);
//    
//    message.clear();
//    
//    while(ss >> str){
//      
//      //std::cout << i++ << str << '\n';
//      std::string tmp{""};
//      std::string pooop;
//      
//      std::getline(ss, tmp, '\r');
//      
//      if(str == "GET"){
//	std::string http;
//	std::size_t pos1 = tmp.find("http://");
//	if(pos1 != std::string::npos){
//	  std::size_t pos2 = tmp.find_first_of("/",pos1+7);
//	  if(pos2 != std::string::npos){
//	    tmp.erase(pos1, pos2-pos1);
//	  }
//	}
//	pooop = str + tmp;
//	
//      }
//      else if(str == "HOST:" || str == "Host:" || str == "host:"){
//	pooop = str + tmp;
//	
//      }
////      else if(str == "User-Agent:" || str == "user-agent:" || str == "USER-AGENT:"){
////	pooop = str + tmp;
////      }
////      else if(str == "Accept:" || str == "accept:" || str == "ACCEPT:"){
////	pooop = str + tmp;
////      }
////      else if(str == "Accept-Language:" || str == "accept-language:" || str == "ACCEPT-LANGUAGE:"){
////	pooop = str + tmp;
////      }
////      else if(str == "Accept-Encoding:" || str == "accept-encoding:" || str == "ACCEPT-ENCODING:"){
////	pooop = str + tmp;
////      }
////      else if(str == "DNT:" || str == "dnt:" || str == "Dnt:"){
////	
////	pooop = str + tmp;
////      }
////      else if(str == "Referer:" || str == "referer:" || str == "REFERER:"){
////	pooop = str + tmp;
////      }
//      else if(str == "Connection:" || str == "connection:" || str == "CONNECTION:"){
//	pooop = str + " close";
//      }
//      
//      if(!pooop.empty())
//	message.append(pooop+"\r\n");
//	
//      pooop = "";
//      getline(ss, tmp);
//      str.clear();
//    }
//    message.append("\r\n");
//    //std::cout << buffer << std::endl;

  return 0;
}

std::string ProxyClient::SendAndRecieveRequest( const std::string &host, char *port, const std::string &client_request_message ){
  
  std::string server_response_message = "";
  
  //if( server->FormatHttpRequest( client_request_message, host ) == -1 ){
  //  std::cout << "Bad request format!\n";
  //  return -1;
  //}
  
  int socket_server = EstablishConnection( host, port);
  if( socket_server == -1 ){
    close(socket_server);
    return "";
  }
  
  Send( socket_server, client_request_message );
  
  if( Recieve(socket_server, server_response_message) == -1 ){
    close(socket_server);
    return "";
  }
  
  close(socket_server);
  return server_response_message;
  
}
