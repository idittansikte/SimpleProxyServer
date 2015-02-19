#include "Proxy.h"

#include <iostream>
#include "ProxyServer.h"
#include "ProxyClient.h"
#include <unistd.h> // Close, Fork

//#define HTTP_PORT 80
std::string BAD_CONTENT = "HTTP/1.1 301 Moved Permanently\r\nLocation: http://www.ida.liu.se/~TDTS04/labs/2011/ass2/error2.html\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n";
std::string BAD_URL = "HTTP/1.1 301 Moved Permanently\r\nLocation: http://www.ida.liu.se/~TDTS04/labs/2011/ass2/error1.html\r\nConnection: close\r\n\r\n";

bool SearchBadWords( const std::string & message );

bool SendAndRecieveRequest( const std::string &host, char *port, const int socket_client, const std::string &client_request_message );

int plainText( const std::string &message );

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
	  std::cout << "OBS! OBS! BADWORDS IN URL!!! " << "\n";
	  server->Send(socket_client, BAD_URL );
	  exit(0);
	}
	std::string host = "";
	if( server->FormatHttpRequest( client_request_message, host ) == -1 ){
	  std::cout << "Aborting: Bad request format!\n";
	  return -1;
	}
	
	if( !SendAndRecieveRequest( host, HTTP_PORT, socket_client, client_request_message ) ){
	  return 3;
	}
	
	//if( SearchBadWords( server_response_message ) ){
	//  RedirectToBadContent( server_response_message );
	//}
	
	//server->Send(socket_client, server_response_message);
	
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
  
  std::vector<std::string> badwords{"yolo", "spongebob", "kalleanka", "swag"};
  
  for( auto it : badwords){
    std::size_t found = str.find(it);
    if( found != std::string::npos ){
      return true;
    }
  }
  return false;
}

bool SendAndRecieveRequest( const std::string &host, char *port, const int socket_client, const std::string &client_request_message ){
  
  std::string server_response_message = "";
  
  ProxyClient client;
  ProxyServer server;
  
  int socket_server = client.EstablishConnection( host, port);
  if( socket_server == -1 ){
    close(socket_server);
    return false;
  }
  
  client.Send( socket_server, client_request_message );
  
  bool doOnce = false;
  int isText = 0;
  int totbrecieved{};
  std::string textbuffer{};
  
  while( true ){
    
    int recievedbytes = client.Recieve(socket_server, server_response_message);
    
    if( recievedbytes == -1 ){
      break;
    }
    
    totbrecieved += recievedbytes;
    
    //------ Check if contentype => text. ------//
    if(doOnce == false){
      doOnce = true;
      isText = plainText(server_response_message);
      if( isText == -1 ){
	return false;
      }
    }
    
    if( isText == 1 ){
      textbuffer += server_response_message;
      std::cout << "Buffering text... Bytes: " << textbuffer.size() << "\n";
      if( recievedbytes != 0 ){
	continue;
      }
      
      std::cout << "Searching in text for bad words..\n";
      if( SearchBadWords( textbuffer ) ){
	std::cout << "OBS! OBS! BADWORDS IN CONTENT! " << "\n";
	server.Send(socket_client, BAD_CONTENT);
	return true;
      }
      
      server.Send(socket_client, textbuffer);
      break;
    }
    
    if( recievedbytes == 0 ){
      break;
    }
    
    //std::cout << "Sending: " << server_response_message << '\n';
    server.Send(socket_client, server_response_message);
    
  } // End of while loop...
  std::cout << "Everything went fine with request! Bytes delivered: " << totbrecieved << "\n"; 
  close(socket_server);
  close(socket_client);
  return true;
  
} // End of function

int plainText( const std::string &server_response_message ){
  
  int isText = 0;
  std::size_t start = server_response_message.find("Content-Type:") + 1;
  if( start != std::string::npos ){
    std::size_t end = server_response_message.find("\r\n", start);
    if( end != std::string::npos ){
      std::string str = server_response_message.substr(start, end - start);
      std::size_t found = str.find("text");
      if( found != std::string::npos ){
	isText = 1;
      }
    }
  }
  else{
    std::cerr << "Bad Request: Could not find any Content-Type. Aborted!\n";
    return -1;
  }
  
  return isText;
}
