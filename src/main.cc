#include "Proxy.h"

#include <iostream>
#include "ProxyServer.h"
#include "ProxyClient.h"
#include <unistd.h> // Close, Fork

int main(int argc, char* argv[]){
  
  char* port;
  
  if(argc < 2) {
    port = (char*)"8549";
    std::cout << ">> Portnumber automatically set to " << port << ". \n";
  }
  else{
    int i = atoi(argv[1]);
    if(i < 1024 && i > 9999){
      std::cerr << "Invalid portnumber! Set a port between 1024 and 9999.\n";
    }
    else{
      port = argv[1];
      std::cout << ">> Portnumber set to " << port << ". \n";
    }
  }
  
  //Proxy proxy(argv[1]);
  //
  //if( proxy.Init() == 0){
  //  proxy.Run();
  //}
  
  ProxyServer* server = new ProxyServer();
  
  int socket_listner = server->StartListen("3521");
  
  while(true){
    int socket_client = server->AcceptConnections(socket_listner); // this one does fork every time someone connect
    
    if(socket_client > -1){
      if(!fork()){ // Start for all child processes
	close(socket_listner); // Child wont need this..
    
	int MAX_SIZE = 4194304;
	std::string client_message = "";
	std::string server_response_message = "";
	int socket_server = -1;
	char HTTP_PORT[] = "80";
	ProxyClient* client = new ProxyClient();
	
	server->RecieveMessage(client_message, socket_client);
	server->CheckBadWords( client_message );
	
	std::string address;
	client->EditGETLine( client_message, address );
	client->EstablishConnection(socket_server, address, HTTP_PORT);
	  
	client->Send( socket_server, client_message, MAX_SIZE );
	client->Recieve(socket_server, server_response_message, MAX_SIZE);
	
	if( client->CheckBadWords( server_response_message ) ){
	  client->EditGETLine( server_response_message, address );
	  client->EstablishConnection(socket_server, address, HTTP_PORT);
	  client->Send( socket_server, server_response_message, MAX_SIZE );
	  client->Recieve(socket_server, server_response_message, MAX_SIZE);
	}
	
	server->Send(socket_client, server_response_message, MAX_SIZE);
	
	delete client;
	
	return 0;
      }
    }
     close(socket_client);
  } // End endless while loop;
  
  delete server;
  
  return 0;
}
