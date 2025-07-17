#pragma once

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <fcntl.h>
#include <cstring>
#include <string> 
#include <unistd.h>
#include <dirent.h>
#include <sstream>
#include <cstdarg>
#include <cstdio>
#include <utility>

#include <cstdlib> 
#include <fstream>
#include <sstream>
#include <cctype>
#include <ctime>
#include <cstdarg>

#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <iterator>
#include <list>

# include <sys/types.h>
# include <sys/wait.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <unistd.h>
# include <signal.h>

# include <sys/socket.h>
# include <netinet/in.h>
# include <sys/select.h>
# include <arpa/inet.h>

#include "Message.hpp"
#include "ServerSetUp.hpp"
#include "LocationParser.hpp"
#include "ConfigFile.hpp"
#include "ReadConfig.hpp"
#include "ServerManager.hpp"
#include "utils.hpp"
#include "Request.hpp"

# define SUCCESS    0
# define ERROR      1

# define MAX_CONTENT_LENGTH 30000000
# define USAGE "Usage: ./WebServ [config_file]"
# define BUFFER_SIZE  1024
# define BACKLOG_SIZE 10


std::string statusCodeString(short);
int 		ft_stoi(std::string str);

# endif