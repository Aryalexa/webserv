#ifndef UTILS_HPP
#define UTILS_HPP

#include "../include/WebServ.hpp"


void		set_nonblocking(int sock);
int 		ft_stoi(std::string str);
HttpRequest parse_http_request(const std::string &request_str);

#endif // UTILS_HPP