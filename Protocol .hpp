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

#define SEP ": "
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
      int content_length;

    public:
      HttpRequest():content_length(0){}
     ~HttpRequest(){}

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
       std::string key;
            std::string value;
            for(auto &iter : http_request.request_header)
            {
                if(Util::CutString(iter, key, value, SEP)){
                    http_request.header_kv.insert({key, value});
                }
            }
    }

     bool IsNeedRecvHttpRequestBody()
        {
            auto &method = http_request.method;
            if(method == "POST"){
                auto &header_kv = http_request.header_kv;
                auto iter = header_kv.find("Content-Length");
                if(iter != header_kv.end()){
                    LOG(INFO, "Post Method, Content-Length: "+iter->second);
                    http_request.content_length = atoi(iter->second.c_str());
                    return true;
                }
            }
            return false;
        }

     void RecvHttpRequestBody()

        {
            if(IsNeedRecvHttpRequestBody()){
                int content_length = http_request.content_length;
                auto &body = http_request.request_body;

                char ch = 0;
                while(content_length){
                    ssize_t s = recv(sock, &ch, 1, 0);
                    if(s > 0){
                        body.push_back(ch);
                        content_length--;
                    }
                    else{
                        break;
                    }
                }
            }

        }


 public:
    EndPoint(int _sock):sock(_sock)
    {}
    void RecvHttpPoint()
    {
      RecvHttpRequestLine();
      RecvHttpRequestHeader();
      RecvHttpRequestBody();
      ParseHttpRequestLine();
      ParseHttpRequestHeader();
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
          ep->BuildHttpResponse();
          ep->SendHttpResponse();
          delete ep;
#endif 
         LOG(INFO,"Hander Request End!");
          return nullptr;

      }

};
