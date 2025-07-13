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

#include "../include/logging.hpp"
#include "../include/ServerSetUp.hpp"
#include "../include/LocationParser.hpp"
#include "../include/ConfigFile.hpp"
#include "../include/ReadConfig.hpp"
#include "../include/ServerManager.hpp"
#include "../include/utils.hpp"

# define SUCCESS    0
# define ERROR      1

# define MAX_CONTENT_LENGTH 30000000
# define USAGE "Usage: ./webserv [config_file]"
# define BUFFER_SIZE  1024
# define BACKLOG_SIZE 10

# define DEFAULT_CONFIG_FILE "config/default.config"

std::string statusCodeString(short);
int 		ft_stoi(std::string str);

# endif