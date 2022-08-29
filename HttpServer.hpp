#pragma once 

#include <iostream>
#include <pthread.h>
#include"Protocol.hpp"
#include "TcpServer.hpp"
#include"Log.hpp"

#define PORT 8081

class HttpServer{
private:
    int port;
    TcpServer *tcp_server;
    bool stop;
public:
    HttpServer(int _port = PORT):port(_port),tcp_server(nullptr),stop(false)
    {}
    
    void InitServer()
    {
      tcp_server =TcpServer::getinstance(port);
    }
     
    void Loop()
    {
      LOG(INFO,"Loop,begin!");
      int listen_sock = tcp_server->Sock();
      while(!stop){
        struct sockaddr_in peer;
        socklen_t len =sizeof(peer);
        int sock =accept(listen_sock,(struct sockaddr*)&peer,&len);
        if(sock < 0 ){
          continue;
        }
        LOG(INFO,"Get a new link");
        int *_sock = new int(sock);
        pthread_t tid;
        pthread_create(&tid,nullptr,Entrance:: HandlerRequest,_sock);
        pthread_detach(tid);

        

      }
    }
    
    
    ~HttpServer()
    {}
};

