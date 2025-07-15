#pragma once

#include "../include/WebServ.hpp"

class HttpRequest
{
private:
	std::string _method;
	std::string _uri;
	std::string _version;
	std::map<std::string, std::string> _headers;
	std::string _body;
	void _parse_request_line(const std::string &request_line);
	void _parse_headers(const std::list<std::string> &headers_strs);
	void _parse_body(const std::string &body);

public:
	HttpRequest(); //TODO: hace falta? see rule of 4
	HttpRequest(std::string request_str);
	HttpRequest(const HttpRequest &other);
	HttpRequest &operator=(const HttpRequest &other);
	~HttpRequest();

};