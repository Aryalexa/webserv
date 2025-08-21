#pragma once

#include "../include/WebServ.hpp"


struct ResponseStatus {
    int code;
    std::string message;

	ResponseStatus();
	ResponseStatus(int code, const std::string& message);
};

struct ResponseHeaders {
	std::string content_type;
	std::string content_length;
	std::string connection;
	std::string location;
};



class HttpResponse
{
private:
	const Request &_request;
	ResponseStatus _status_line;
	ResponseHeaders _headers;
	std::string _body;
	
	HttpResponse();
	HttpResponse(const HttpResponse &other);
	HttpResponse &operator=(const HttpResponse &other);
	
public:
	static const std::string CRLF;
	static const std::string version;

	HttpResponse(const Request &request);
	HttpResponse(const Request &request, int errorCode);
	HttpResponse(const Request &request, int errorCode, const std::string errorPagePath);
	~HttpResponse();

	std::string getStatusLine() const ;
	std::string getHeaders() const ;
	std::string getBody() const ;
	std::string toString() const ;
	std::string getResponse() const;
	void generate_index(const Request& request);
	void redirect();
	void isOk();
	void createOk();

	void handle_GET();

};