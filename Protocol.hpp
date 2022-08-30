#pragma once 

#include<iostream>
#include<sstream>
#include<unistd.h>
#include<vector>
#include<unordered_map>
#include<string>
#include<sys/types.h>
#include<sys/socket.h>
#include"Util.hpp"
#include"Log.hpp"

class HttpRequest{
    
    public:
      std::string request_line;
      std::vector<std::string> request_header;
      std::string blank;
      std::string request_body;

      //解析之后的方法
      std::string method;
      std::string uri;
      std::string version;

      std::unordered_map<std::string,std::string>header_kv;

};

class HttpResponse{
   public:
     std::string status_line;
     std::vector<std::string>response_header;
     std::string blank;
     std::string response_body;
};

//读取请求，分析请求，构建相应
//IO通信
class EndPoint{
  private:
    int sock;
    HttpRequest http_request;
    HttpResponse http_response;
  private:
    void RecvHttpRequestLine()
    {
       auto &line =http_request.request_line;
       Util::ReadLine(sock,line);
       line.resize(line.size()-1);
       LOG(INFO,http_request.request_line);
    }
    
    void RecvHttpRequestHeader()
    {
      std::string line;
      while(true)
      {
        line.clear();
        Util::ReadLine(sock,line);
        if(line == "\n")
        {
          http_request.blank = line;  
          break;
        }
        line.resize(line.size()-1);
        http_request.request_header.push_back(line);
        LOG(INFO,line);
      }
    
    }
   
    void ParseHttpRequestLine()
    {  auto &line = http_request.request_line;
       std::stringstream ss(line);
       ss >> http_request.method >> http_request.uri >>http_request.version ;
       LOG(INFO,http_request.method);
       LOG(INFO,http_request.uri);
       LOG(INFO,http_request.version);

    }

    void ParseHttpRequestHeader()
    {
      
    }



 public:
    EndPoint(int _sock):sock(_sock)
    {}
    void RecvHttpPoint()
    {
      RecvHttpRequestLine();
      RecvHttpRequestHeader();
    }
    void ParseHttpRequest()
    {
      ParseHttpRequestLine();
      RarseHttpRequestHeader();
    }
    void BuildHttpResponse()
    {

    }
    void SendHttpResponse()
    {

    }


    ~EndPoint()
    {
    close(sock);
    }

};



class Entrance{
    public:
      static void *HandlerRequest(void *_sock)
      {
        LOG(INFO,"Hander Requeset Begin!");
        int sock =*(int*)_sock;
        delete (int*)_sock;  
       
 
#ifdef DEBUG 
        //For Test
          char buffer[4096];
          recv(sock,buffer,sizeof(buffer),0);
          std::cout<<"-------------------begin--------------------"<<std::endl;
          std::cout<<buffer<<std::endl;
          std::cout<<"-------------------end--------------------"<<std::endl;
      
#else 
          EndPoint *ep = new EndPoint(sock);
          ep->RecvHttpPoint();
          ep->ParseHttpRequest();
          ep->BuildHttpResponse();
          ep->SendHttpResponse();
          delete ep;
#endif 
         LOG(INFO,"Hander Request End!");
          return nullptr;

      }

};
