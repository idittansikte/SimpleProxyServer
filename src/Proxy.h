#ifndef PROXY_H
#define PROXY_H

#include <string>
#include <stdlib.h>
#include <iostream>
#include <cstring> //Memset
#include <string>
#include <sstream>
#include <algorithm>

#include <unistd.h> // Close
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> // Already has addrinfo???
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define HTTP_PORT "80"

class Proxy {
public:
  Proxy(std::vector<std::string> v);

  void *get_in_addr(struct sockaddr *sa);
  
  /*
  *	Bind socket with the chosen port number and starts listening
  *	for connections at that socket.
  *	Returns the listening socket.
  */
  int StartListen(const char* port);
  
  /*
  *	Wait for any connection with client and try accept if there is.
  *	Returns the connected clients socket.
  */
  int AcceptConnections(int socket_listner);
  
  /**
  *	Handles all sendings.
  *	Makes sure everything is sent.
  **/
  void SendAll(const int socket, const std::string &http_message );
  
  /*
   *	Format/edits request header to please server/host.
   *	Edits GET and connection header.
   *	Sets hostname parameter to be the same as Host header,
   *	we will need it later when connection to server.
   *	Returns -1 when something went wrong and 0 if right.
   */
  int FormatRequestHeaders( std::string &message, std::string &hostname );
  
  /*
   *	Searches for blocked words.
   *	Uses badwords vector with words to compare with.
   *	Bad words is set as arguments at program start.
   *	Returns true if bad word is found.
   */
  bool SearchBadWords( const std::string & message );
  
  /*
   *	Handle client request.
   *	Receives request message, handle bad words, format headers
   *	and send/receive to/from server and send it back to client.
   *	Return 0 if everything went fine else > 0.
   */
  int HandleRequest( const int client_socket );
  
  /*
   *	Creates connection to server/host.
   *	Hostname/IP needed as parameter.
   *	Returns server socket.
   */
  int EstablishConnection( const std::string host );
  
  /*
   *	Check if server response is text. (Not image or other format)
   *	Simply checks Content-Type: header.
   *	Needed cuz we dont want to search for bad words in picutres.
   *	Returns 1 if text, 0 if not, -1 if content type header not found.
   */
  int plainText( const std::string &server_response_message );
  
  /*
   *	Send client request to server, receive the response.
   *	If response content is text, buffer it all upp and check for bad words
   *	and then send it to client.
   *	If response is not text, send every packet to client without buffer
   *	it up.
   *	Returns true if everything went good, false if bad.
   */
  bool GetDataFromServerAndDeliverToClient( const std::string &host, const int socket_client, const std::string &client_request_message );
  
private:
    
    std::vector<std::string> badwords;
    
    std::string BAD_URL;
    std::string BAD_CONTENT;
};

#endif
