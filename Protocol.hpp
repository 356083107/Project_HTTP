#pragma once 

#include<iostream>
#include<sstream>
#include<unistd.h>
#include<vector>
#include<unordered_map>
#include<string>
#include<fcntl.h>
#include<sys/types.h>
#include<algorithm>
#include<sys/stat.h>
#include<sys/socket.h>
#include <sys/sendfile.h>
#include"Util.hpp"
#include"Log.hpp"

#define OK 200
#define WEB_ROOT "wwwroot"
#define HOME_PAGE "index.html"
#define HTTP_VERSION "HTTP/1.0"
#define LINE_END "\r\n"

#define NOT_FOUND 404
#define SEP ": "

static std::string Code2Desc(int code)
{
   std::string desc;
   switch(code)
   {
     case 200:desc ="OK";
              break;
     case 404:desc ="Not Found";
              break;
    default:break;

   }
   return desc;
}

static std::string Suffix2Desc(const std::string &suffix)

{
  
  static std::unordered_map<std::string,std::string> suffix2desc ={
  {".html","text/html"},
  {".css","text/css"},
  {".js","application/javascript"},
  {".jpg","application/x-jpg"},
};

  auto iter =suffix2desc.find(suffix);
  if(iter !=suffix2desc.end()){
  return iter->second;
}
return "text/html";
}




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
      std::string path;
      std::string suffix;
      std::string query_string;

      bool cgi;


    public:
      HttpRequest():content_length(0),cgi(false){}
     ~HttpRequest(){}

};

class HttpResponse{
   public:
     std::string status_line;
     std::vector<std::string>response_header;
     std::string blank;
     std::string response_body;

     int status_code;
     int fd;
     int size;
  public:
     HttpResponse():status_code(OK),fd(-1){}

    ~ HttpResponse(){}

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
       auto &method =http_request.method;
       std::transform(method.begin(),method.end(),method.begin(),::toupper);
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

int ProcessNonCgi(int size)
    {
      http_response.fd =open(http_request.path.c_str(),O_RDONLY);
      if(http_response.fd >= 0){

      http_response.status_line =HTTP_VERSION;
      http_response.status_line +=" ";
      http_response.status_line +=std::to_string(http_response.status_code);
      http_response.status_line +=" ";
      http_response.status_line +=Code2Desc(http_response.status_code);
      http_response.status_line +=LINE_END;
      http_response.size = size;
      
      std::string header_line= "Content-Length: " ;
      header_line += std::to_string(size);
      header_line +=LINE_END;
      http_response.response_header.push_back(header_line);


      header_line ="Content-Type: ";
      header_line +=Suffix2Desc(http_request.suffix);
      header_line +=LINE_END; 
      http_response.response_header.push_back(header_line);

        return OK;
      }    
        return 404;
    }

    void BuildHttpResponse()
    {
      std::string _path;
      struct stat st;
      int size =0;
      std::size_t found =0;
      auto &code = http_response.status_code;

      if(http_request.method !="GET" && http_request.method !="POST"){
        //非法请求
        LOG(WARNING,"method is not right");
        code = NOT_FOUND;
        goto END;

      }
      if(http_request.method == "GET"){
         size_t pos = http_request.uri.find('?');
         if(pos !=std::string::npos){
           Util::CutString(http_request.uri,http_request.path,http_request.query_string,"?");
         }
           else {
             http_request.path = http_request.uri;
           }
         }
      else if(http_request.method == "POST"){
        //POST
        http_request.cgi = true;
      }
      else {
        //Do Nothing
      }
      _path = http_request.path;
      http_request.path = WEB_ROOT;
      http_request.path +=_path;
      if(http_request.path[http_request.path.size()-1] =='/'){
        http_request.path +=HOME_PAGE;
      }
    

      if(stat(http_request.path.c_str(),&st) == 0)
      {
        //说明资源是存在的
                        if(S_ISDIR(st.st_mode)){
                    //说明请求的资源是一个目录，不被允许的,需要做一下相关处理
                    //虽然是一个目录，但是绝对不会以/结尾！
                    http_request.path += "/";
                    http_request.path += HOME_PAGE;
                    stat(http_request.path.c_str(), &st);
                }
                if( (st.st_mode&S_IXUSR) || (st.st_mode&S_IXGRP) || (st.st_mode&S_IXOTH) ){
                    //特殊处理
                    http_request.cgi = true;
                }
                size =st.st_size;
              

      }
    else{
      //说明资源不存在的
      std::string info = http_request.path;
      info +="Not Found!";
      LOG(WARNING,info);
      code = NOT_FOUND;
      goto END;
    }
    
   found =http_request.path.rfind(".");
   if(found == std::string::npos){
      http_request.suffix =".html";
   }
   else{
     http_request.suffix = http_request.path.substr(found);
   }

    if(http_request.cgi){
      //ProcessCgi();
    }
    else{
      code = ProcessNonCgi(size);    //返回静态网页

      
    }


END:
    if(code != OK){

      
    }

    }

    void SendHttpResponse()
    {
      send(sock,http_response.status_line.c_str(),http_response.status_line.size(),0);
      for(auto iter:http_response.response_header){
        send(sock,iter.c_str(),iter.size(),0);
      }
     send(sock,http_response.blank.c_str(),http_response.blank.size(),0);
     
      
     sendfile(sock, http_response.fd ,nullptr, http_response.size);

    
     close(http_response.fd);
    }


    ~EndPoint()
    {
    close(sock);
    }

};


//#define DEBUG 1

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
