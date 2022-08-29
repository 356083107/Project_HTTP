#include<iostream>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>

class Entrance{
    public:
      static void *HandlerRequest(void *_sock)
      {
        int sock =*(int*)_sock;
        delete (int*)_sock;  
       
        std::cout<<"get a new link ..."<<sock<<std::endl;
 
        //For Test
        char buffer[4096];
        recv(sock,buffer,sizeof(buffer),0);
        std::cout<<"-------------------begin--------------------"<<std::endl;
        std::cout<<buffer<<std::endl;
        std::cout<<"-------------------end--------------------"<<std::endl;

        close(sock);
        return nullptr;
      }

};
