#include "Proxy.h"

#include <iostream>
#include "ProxyServer.h"
#include "ProxyClient.h"
#include <unistd.h> // Close, Fork

//#define HTTP_PORT 80

void RedirectToBadURL( std::string & message );

void RedirectToBadContent( std::string & message );

bool SearchBadWords( const std::string & message );

int main(int argc, char* argv[]){
  
  char port[5];
  
  if(argc < 2) {
    strcpy(port, "8080");
    std::cout << ">> Portnumber automatically set to " << port << ". \n";
  }
  else{
    int i = atoi(argv[1]);
    if(i < 1024 && i > 9999){
      std::cerr << "Invalid portnumber! Set a port between 1024 and 9999.\n";
      return 1;
    }
    else{
      strcpy(port, argv[1]);
      std::cout << ">> Portnumber set to " << port << ". \n";
    }
  }
  
  ProxyServer* server = new ProxyServer();
  
  int socket_listner;
  if ( (socket_listner = server->StartListen(argv[1])) == -1 ){
    return 2;
  }
  
  while(true){
    int socket_client = server->AcceptConnections(socket_listner); // this one does fork every time someone connect
    
    if(socket_client > -1){
      if(!fork()){ // Start for all child processes
	
	close(socket_listner); // Child wont need this..
	ProxyClient client;
	char HTTP_PORT[] = "80";
	
	std::string client_request_message = server->RecieveMessage(socket_client);
	
	if( SearchBadWords( client_request_message ) ){
	  RedirectToBadURL( client_request_message );
	}
	
	std::string host = "";
	if( server->FormatHttpRequest( client_request_message, host ) == -1 ){
	  std::cout << "Bad request format!\n";
	  return -1;
	}
	
	std::string server_response_message = client.SendAndRecieveRequest( host, HTTP_PORT, client_request_message );
	if( server_response_message == ""){
	  return 3;
	}
	
	if( SearchBadWords( server_response_message ) ){
	  RedirectToBadContent( server_response_message );
	  host = "";
	  if( server->FormatHttpRequest( client_request_message, host ) == -1 ){
	  std::cout << "Bad request format!\n";
	  return -1;
	  }
	  
	  server_response_message = client.SendAndRecieveRequest( host, HTTP_PORT, client_request_message );
	  
	  if( server_response_message == ""){
	    return 4;
	  }
	  
	}
	
	server->Send(socket_client, server_response_message);
	
	exit(0);
      }
    }
     close(socket_client);
  } // End endless while loop;
  
  delete server;
  
  return 0;
}

bool SearchBadWords( const std::string & message ){
  
  std::string str(message);
  transform(str.begin(), str.end(), str.begin(), tolower);
  str.erase(remove_if(str.begin(), str.end(), isspace),str.end());
  
  std::vector<std::string> badwords{"yolo", "nigger", "kalleanka", "swag"};
  
  for( auto it : badwords){
    std::size_t found = str.find(it);
    if( found != std::string::npos ){
      std::cout << "OBS! OBS! BADWORDS IN CONTENT/URL!!! " << it << "\n";
      return true;
    }
  }
  return false;
}

void RedirectToBadURL( std::string & message ){
    message.clear();
    message = "GET /~TDTS04/labs/2011/ass2/error1.html HTTP/1.1\r\nHost: www.ida.liu.se\r\nConnection: close\r\n\r\n";
}

void RedirectToBadContent( std::string & message ){
    message.clear();
    message = "GET /~TDTS04/labs/2011/ass2/error2.html HTTP/1.1\r\nHost: www.ida.liu.se\r\nConnection: close\r\n\r\n";
}