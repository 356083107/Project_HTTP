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
        close(sock);
        return nullptr;
      }

};
