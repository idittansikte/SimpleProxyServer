#include "Proxy.h"

//#define HTTP_PORT "80"

Proxy::Proxy()
{}

// get sockaddr, IPv4 or IPv6:
void *Proxy::get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}



int Proxy::EditGETLine( std::string &message, std::string &address ){
  
    std::string str, get, host, usr_agent, accept, accept_lang, accept_enc, dnt, referer, connection = "";
  
    std::istringstream ss(message);
    
    message.clear();
    
    char *buffer;
    
    int i{};
    while(ss >> str){
      
      //std::cout << i++ << str << '\n';
      std::string tmp{""};
      std::string pooop;
      
      std::getline(ss, tmp, '\r');
      
      if(str == "GET"){
	std::string http;
	std::size_t pos1 = tmp.find("http://");
	if(pos1 != std::string::npos){
	  std::size_t pos2 = tmp.find_first_of("/",pos1+7);
	  if(pos2 != std::string::npos){
	    tmp.erase(pos1, pos2-pos1);
	  }
	}
	pooop = str + tmp;
	
      }
      else if(str == "HOST:" || str == "Host:" || str == "host:"){
	pooop = str + tmp;
	tmp.erase(remove_if(tmp.begin(), tmp.end(), isspace),tmp.end());
	address = tmp;
	
      }
//      else if(str == "User-Agent:" || str == "user-agent:" || str == "USER-AGENT:"){
//	pooop = str + tmp;
//      }
//      else if(str == "Accept:" || str == "accept:" || str == "ACCEPT:"){
//	pooop = str + tmp;
//      }
//      else if(str == "Accept-Language:" || str == "accept-language:" || str == "ACCEPT-LANGUAGE:"){
//	pooop = str + tmp;
//      }
//      else if(str == "Accept-Encoding:" || str == "accept-encoding:" || str == "ACCEPT-ENCODING:"){
//	pooop = str + tmp;
//      }
//      else if(str == "DNT:" || str == "dnt:" || str == "Dnt:"){
//	
//	pooop = str + tmp;
//      }
//      else if(str == "Referer:" || str == "referer:" || str == "REFERER:"){
//	pooop = str + tmp;
//      }
      else if(str == "Connection:" || str == "connection:" || str == "CONNECTION:"){
	pooop = str + " close";
      }
      
      if(!pooop.empty())
	message.append(pooop+"\r\n");
	
      pooop = "";
      getline(ss, tmp);
      str.clear();
    }
    message.append("\r\n");
    //std::cout << buffer << std::endl;

  return 0;
}

bool Proxy::SearchBadWords( const std::string & message ){
  
  std::string str(message);
  transform(str.begin(), str.end(), str.begin(), tolower);
  str.erase(remove_if(str.begin(), str.end(), isspace),str.end());
  
  std::vector<std::string> badwords{"spongebob", "britneyspears", "parishilton", "norrkoping"};
  
  for( auto it : badwords){
    std::size_t found = str.find(it);
    if( found != std::string::npos ){
      return true;
    }
  }
  return false;
}

bool Proxy::CheckBadWords( std::string & message ){
  if( SearchBadWords(message) ){
    message.clear();
    message = "";
    message = "GET /~TDTS04/labs/2011/ass2/error2.html HTTP/1.1\r\nHost: www.ida.liu.se\r\nConnection: close\r\n";
    return true;
  }
  return false;
}