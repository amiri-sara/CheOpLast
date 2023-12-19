#ifndef LOG_H
#define LOG_H

#include <iostream>

#define SHOW_ERROR(X)             std::cout<<"\033[1;31m"<<X<<"\033[0m"<<std::endl
#define SHOW_LOG(X)               std::cout<<"\033[1;32m"<<X<<"\033[0m"<<std::endl
#define SHOW_WARNING(X)           std::cout<<"\033[1;33m"<<X<<"\033[0m"<<std::endl
#define SHOW_IMPORTANTLOG(X)      std::cout<<"\033[1;34m"<<X<<"\033[0m"<<std::endl
#define SHOW_IMPORTANTLOG2(X)     std::cout<<"\033[1;35m"<<X<<"\033[0m"<<std::endl
#define SHOW_IMPORTANTLOG3(X)     std::cout<<"\033[1;36m"<<X<<"\033[0m"<<std::endl

#endif // LOG_H
