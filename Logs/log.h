#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <iomanip>
#include <ctime>
#include <sys/time.h>

#include <bsoncxx/v_noabi/bsoncxx/json.hpp>
#include <mongocxx/v_noabi/mongocxx/client.hpp>
#include <mongocxx/v_noabi/mongocxx/stdx.hpp>
#include <mongocxx/v_noabi/mongocxx/uri.hpp>
#include <mongocxx/v_noabi/mongocxx/instance.hpp>
#include <bsoncxx/v_noabi/bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/v_noabi/bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/v_noabi/bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/v_noabi/bsoncxx/builder/stream/array.hpp>
#include <mongocxx/v_noabi/mongocxx/pool.hpp>
#include <mongocxx/v_noabi/mongocxx/exception/exception.hpp>
#include <mongocxx/v_noabi/mongocxx/exception/bulk_write_exception.hpp>

#include <boost/thread.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include <curl/curl.h>

#define SHOW_ERROR(X)             std::cout<<"\033[1;31m"<<X<<"\033[0m"<<std::endl
#define SHOW_LOG(X)               std::cout<<"\033[1;32m"<<X<<"\033[0m"<<std::endl
#define SHOW_WARNING(X)           std::cout<<"\033[1;33m"<<X<<"\033[0m"<<std::endl
#define SHOW_IMPORTANTLOG(X)      std::cout<<"\033[1;34m"<<X<<"\033[0m"<<std::endl
#define SHOW_IMPORTANTLOG2(X)     std::cout<<"\033[1;35m"<<X<<"\033[0m"<<std::endl
#define SHOW_IMPORTANTLOG3(X)     std::cout<<"\033[1;36m"<<X<<"\033[0m"<<std::endl

#endif // LOG_H
