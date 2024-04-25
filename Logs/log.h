#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <iomanip>
#include <ctime>
#include <sys/time.h>
#include <vector>
#include <shared_mutex>
#include <chrono>

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
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/filesystem.hpp>

#include <opencv2/opencv.hpp>

#include <curl/curl.h>

#define SHOW_ERROR(X)             std::cout<<"\033[1;31m"<<X<<"\033[0m"<<std::endl
#define SHOW_LOG(X)               std::cout<<"\033[1;32m"<<X<<"\033[0m"<<std::endl
#define SHOW_WARNING(X)           std::cout<<"\033[1;33m"<<X<<"\033[0m"<<std::endl
#define SHOW_IMPORTANTLOG(X)      std::cout<<"\033[1;34m"<<X<<"\033[0m"<<std::endl
#define SHOW_IMPORTANTLOG2(X)     std::cout<<"\033[1;35m"<<X<<"\033[0m"<<std::endl
#define SHOW_IMPORTANTLOG3(X)     std::cout<<"\033[1;36m"<<X<<"\033[0m"<<std::endl

// Error Code
#define SUCCESSFUL                                          0
#define INVALIDJSON                                         600
#define INVALIDUSERPASS                                     601
#define INVALIDPLATEVALUE                                   604
#define INVALIDPASSEDTIME                                   605
#define OLDDATA                                             606
#define INVALIDPLATEIMAGE                                   607
#define INVALIDCOLORIMAGE                                   609
#define INVALIDCOLORIMAGESIZE                               610
#define DUPLICATERECORD                                     613
#define INVALIDPLATERECT                                    614
#define INVALIDCARRECT                                      615
#define INVALIDDEVICEID                                     616
#define INVALIDUSERID                                       617
#define INVALIDSTREETID                                     618
#define INVALIDVIOLATIONID                                  619
#define INVALIDDIRECTION                                    620
#define INVALIDPLATETYPE                                    621
#define INVALIDSUSPICIOUS                                   622
#define INVALIDSPEED                                        623
#define INVALIDVEHICLETYPE                                  624
#define INVALIDVEHICLEMODEL                                 625
#define INVALIDVEHICLECOLOR                                 626
#define INVALIDLANE                                         627
#define INVALIDLATITUDE                                     628
#define INVALIDLONGITUDE                                    629
#define INVALIDACCURACY                                     630

#define INVALIDGRAYSCALEIMAGE                               632
#define INVALIDCODETYPE                                     633
#define INVALIDMASTERPLATE                                  634
#define INVALIDPROBABILITY                                  635
#define INVALIDRECORDID                                     636
#define INVALIDRECEIVEDTIME                                 637

#define DATABASEERROR                                       651
#define CANNOTFINDVIOLATIONID                               652
#define CANNOTCREATETHUMBNAILIMAGE                          653

#define CANNOTSAVECOLORIMAGE                                654
#define CANNOTSAVEPLATEIMAGE                                655
#define CANNOTSAVETHUMBNAILIMAGE                            656
#define CANNOTCREATEBANNER                                  657
#define CANNOTADDPLATECROP                                  658

#define INVALIDCOMPANYNAME                                  659

#endif // LOG_H
