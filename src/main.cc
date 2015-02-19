#include "Proxy.h"

#include <iostream>
#include "Proxy.h"
#include <unistd.h> // Close, Fork

int main(int argc, char* argv[]){
  
  char port[6];
  std::vector<std::string> bad_words;
  
  // ---- Port configuration, if no port is set as argument, automatically set one ---- //
  if(argc < 2) {
    strcpy( port, "9000" );
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
  
  // ---- Insert all bad words in argument into vector ---- //
  if(argc >= 3){
    for( unsigned int i = 3; i < argc; ++ i){
      bad_words.push_back(argv[i]);
    }
  }
  
  Proxy proxy(bad_words);
  
  // ---- Setup lister ---- //
  int socket_listner;
  if ( (socket_listner = proxy.StartListen(port) ) == -1 ){
    return 2;
  }
  
  // ---- Eternity loop, parent process never leaves this---- //
  while(true){
    
    // ---- Parent process waiting to accept connections... ---- //
    int socket_client = proxy.AcceptConnections(socket_listner); 
    
    // ---- If connection went good, fork child to handle the request ---- //
    if(socket_client > -1){
      if(!fork()){ 
	
	close(socket_listner); // Child wont need this..
	
	// ---- Child request handle ---- //
	if ( proxy.HandleRequest( socket_client ) != 0 ){
	  std::cout << "ERROR: Failed to handle clients request!\n";
	  exit(1);
	}
	
	// ---- Child is done here and is killed ---- //
	exit(0);
      }
    }
    // ---- Parent dont need clients socket so that is closed ---- //
     close(socket_client);
     
  } // End endless while loop;
  
  return 0;
}

