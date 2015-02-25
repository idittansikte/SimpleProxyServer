#include "Proxy.h"

Proxy::Proxy(std::vector<std::string> v)
  : badwords(v)
{
  BAD_CONTENT = "HTTP/1.1 301 Moved Permanently\r\nLocation: http://www.ida.liu.se/~TDTS04/labs/2011/ass2/error2.html\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n";
  BAD_URL = "HTTP/1.1 301 Moved Permanently\r\nLocation: http://www.ida.liu.se/~TDTS04/labs/2011/ass2/error1.html\r\nConnection: close\r\n\r\n";
}

// get sockaddr, IPv4 or IPv6:
void *Proxy::get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void sigchld_handler(int s)
{
  while(waitpid(-1, NULL, WNOHANG) > 0);
}

int Proxy::StartListen(const char* port){
  
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
  
    /* ---- Child processes becomes zombies and are not accually
    * removed/killed from system, this takes care of that... ---- */
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

int Proxy::AcceptConnections(int socket_listner){

  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  char s[INET6_ADDRSTRLEN];
  int socket_client;
    
  std::cout << ">> Waiting for connections...\n";
    

  sin_size = sizeof their_addr;
  // ---- Wait for a connection to accept ---- //
  socket_client = accept( socket_listner, (struct sockaddr *)&their_addr, &sin_size );
  
  if( socket_client == -1){
    perror("accept");
    return -1;
  }
  
  inet_ntop( their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s );
  printf("server: got connection from %s\n", s);
    

  return socket_client;

} // End of function

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

int Proxy::FormatRequestHeaders( std::string &message, std::string &address ){
  std::cout << "1\n";
    // ---- Find host headers position ---- //
    std::string header = "Host:";
    size_t pos = message.find(header);
    if( pos != std::string::npos ){
      // ---- Find end of row and then locate and save hostname ---- //
      char hostname[100];
      std::cout << "2\n";
      std::size_t end = message.find_first_of('\r', pos);
      std::size_t len = message.copy(hostname, end - pos - header.size() - 1 ,pos + header.size() + 1 );
      hostname[len] = '\0';
      address = std::string(hostname, hostname+len);
      // ---- Find GET header position---- //
      header = "GET";
      pos = message.find(header);
      if( pos != std::string::npos ){
	// ---- Find end of row and copy whole line into a new string for searching purposes ---- //
	end = message.find_first_of('\r', pos);
	std::cout << "3\n";
	char array[2000];
	len = message.copy(array, end - pos - header.size() - 1, pos + header.size() + 1 );
	array[len] = '\0';
	std::string tmp = std::string(array, array+len);
	// ---- Find hostname in GET row ---- //
	std::size_t check = tmp.find(address);
	if( check != std::string::npos ){
	  // ---- Now we know hostname exist there. Erase whole URL in GET row in request ---- //
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
    std::cout << "4\n";
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
    
    // ---- Maybe an unnesessary check... But just to make sure. ---- //
    if( address.empty() || message.size() < 8)
      return -1;

  return 0;
}

bool Proxy::SearchBadWords( const std::string & message ){
  
  std::string str(message);
  // ---- Make all characters lowercase for easier handling ---- //
  transform(str.begin(), str.end(), str.begin(), tolower);
  // ---- Erase all spaces to make it easier and to make find() work properly for our puropse ---- //
  //str.erase(remove_if(str.begin(), str.end(), isspace),str.end());
  
  std::vector<std::string> v{"spongebob", "britney spears", "britneyspears" ,"norrköping"};

  // ---- Iterate through all bad words and seek through message ---- //
  for( auto it : v){
    std::size_t found = str.find(it);
    if( found != std::string::npos ){
      return true;
    }
  }
  return false;
}

int Proxy::HandleRequest( const int socket_client ){
  
  int numbytes{};
  int MAX_BUFFER_SIZE = 4096;  
  char buffer[MAX_BUFFER_SIZE];
  std::string request = "";
  
  // ---- Receive request from client ---- //
  numbytes = recv(socket_client, buffer, sizeof buffer, 0);
    
  if ( numbytes == -1 ) {
    perror("server: recv");
    return 1;
  }
    
  std::cout << "Bytes recieved from client: " << numbytes << '\n';
  
  // ---- Convert char array to string ---- //
  request += std::string(buffer, buffer+numbytes);
  request += '\0';

  std::cout << "DEBUG: searching bad words\n";
  // ---- If request contains bad words, send back redirection ---- //
  if( SearchBadWords( request ) ){
    SendAll(socket_client, BAD_URL);
    return 0;
  }
  std::cout << "DEBUG: formatting heades\n";
  // ---- Format headers, and get hostname ---- //
  std::string hostname{};
  if( FormatRequestHeaders( request, hostname ) == -1 ){
    std::cout << ">> Failed to format request, canceling request...\n";
    return 2;
  }
    std::cout << "DEBUG: server stuff..\n";
  // ---- Send request to host/server and deliver the responce to client ---- //
  if ( !GetDataFromServerAndDeliverToClient( hostname, socket_client, request ) ){
    std::cout << "Transmission from server to client went wrong \n";
    return 3;
  }
  
  return 0;

}

int Proxy::EstablishConnection( const std::string host ){
  
  struct addrinfo hints, *servinfo, *p;
  char s[INET6_ADDRSTRLEN];
  int socket_server;

  memset(&hints, 0, sizeof hints);
  
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  
  char* chost = (char*)host.c_str();
  
  int error = getaddrinfo(chost, HTTP_PORT, &hints, &servinfo);
  
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
    
    // ---- Connect to server/host ---- //
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

  // ---- Print out server IP ---- //
  inet_ntop(p->ai_family, Proxy::get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
  printf("client: connecting to %s\n", s);

  freeaddrinfo(servinfo); // all done with this structure
  
  return socket_server;
}

int Proxy::plainText( const std::string &server_response_message ){
  
  // ---- Find header Content-Type to see if responce is text or not ---- //
  int isText = 0;
  std::size_t start = server_response_message.find("Content-Type:");
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

  // ---- Find header Content-Encoding to see if responce is gzip ---- //
  if(isText == 1){
    std::size_t start = server_response_message.find("Content-Encoding:");
    if( start != std::string::npos ){
      std::size_t end = server_response_message.find("\r\n", start);
      if( end != std::string::npos ){
	std::string str = server_response_message.substr(start, end - start);
	std::size_t found = str.find("gzip");
	if( found != std::string::npos ){
	  isText = 0;
	}
      }
    }
  }
  return isText;
}

bool Proxy::GetDataFromServerAndDeliverToClient( const std::string &host, const int socket_client, const std::string &client_request_message ){
  
  std::string server_response_message = "";
  
  // ---- Establish connection with server ---- //
  int socket_server = EstablishConnection( host );
  if( socket_server == -1 ){
    close(socket_server);
    return false;
  }
  
  // ---- Send request to server ----- //
  SendAll( socket_server, client_request_message );
  
  bool doOnce = false;
  int isText = 0;
  int totnumbytes{};
  std::string textbuffer{};
  
  // ---- Loop till whole packet is received ---- //
  while( true ){
    
    int numbytes{};
    int MAX_BUFFER_SIZE = 4096;  
    char buffer[MAX_BUFFER_SIZE];
    
    // ---- Recieve one segment ---- //
    numbytes = recv(socket_server, buffer, sizeof buffer, 0);
    
    if ( numbytes == -1 ) {
      perror("client: recv");
      return -1;
    }
    
    // ---- Convert char array (packet) to string for easier text manipulation ---- //
    server_response_message = std::string(buffer, buffer+numbytes);
    totnumbytes += numbytes; // Count total recieved bytes for debugging
    
    // ---- Check if content type => text. This is done only once because Content-Type header is only in the first segment ---- //
    if(doOnce == false){
      doOnce = true;
      isText = plainText(server_response_message);
      if( isText == -1 ){ // If there where no information of type, exit
	return false;
      }
    }
    
    // ----- Check if text. If so, buffer it up so a full scan for bad words can be done.. -----//
    if( isText == 1 ){
      
      textbuffer += server_response_message;
      std::cout << "Buffering text... Bytes: " << textbuffer.size() << "\n";
      // ---- If not whole text packet in buffer, continue recieve... ---- //
      if( numbytes != 0 ){ 
	continue;
      }
      
      // ---- The whole text is now in buffer, start scan: ---- //
      std::cout << "Searching in content for bad words..\n";
      if( SearchBadWords( textbuffer ) ){
	std::cout << "OBS! OBS! BADWORDS IN CONTENT! " << "\n";
	SendAll(socket_client, BAD_CONTENT);
	return true;
      }
      // ---- Send whole text-buffer then break loop ---- //
      SendAll(socket_client, textbuffer);
      break;
    }
    
    if( numbytes == 0 ){
      break;
    }
    
    // ---- Send all non text segment directly ---- //
    SendAll(socket_client, server_response_message);
    
  } // End of while loop...
  
  std::cout << "Everything went fine with request! Bytes delivered: " << totnumbytes << "\n";
  
  close(socket_server);
  close(socket_client);
  return true;
  
} // End of function

