#include<iostream>
#include<string>
#include<sstream>

int main()
{
  std::string msg ="GET /a/b/c.html http/1.0";
  std::string method;
  std::string uri;
  std::string version;
  std::stringstream ss(msg);

  ss >> method >> uri >> version;
  std::cout<< method << std::endl;
  std::cout<< uri << std::endl;
  std::cout<< version << std::endl;

 return 0;
}

