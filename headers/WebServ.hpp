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

#include "../headers/Message.hpp"
#include "../headers/ServerParser.hpp"
#include "../headers/LocationParser.hpp"
#include "../headers/ConfigFile.hpp"
#include "../headers/ReadConfig.hpp"
#include "../headers/ServerManager.hpp"

# define SUCCESS    0
# define ERROR      1

# define MAX_CONTENT_LENGTH 30000000
# define USAGE "Usage: ./WebServ [config_file]"

std::string statusCodeString(short);
int 		ft_stoi(std::string str);

# endif