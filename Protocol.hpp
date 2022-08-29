#include<iostream>
#include<unistd.h>
#include<vector>
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

};

class HttpResponse{
   public:
     std::string status_line;
     std::vector<std::string>response_header;
     std::string blank;
     std::string response_body;
};

//读取请求，分析请求，构建相应

class EndPoint{
  private:
    int sock;
    HttpRequest http_request;
    HttpResponse http_response;
  private:
    void RecvHttpRequestLine()
    {
       Util::ReadLine(sock,http_request.request_line);
    }
    


 public:
    EndPoint(int _sock):sock(_sock)
    {}
    void RecvHttpPoint()
    {

    }
    void ParseHttpRequest()
    {

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
