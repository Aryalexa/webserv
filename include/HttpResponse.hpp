#pragma once

#include <string>
#include <iostream>

struct ResponseStatus {
    int code;
    std::string message;

    static const ResponseStatus HTTP_OK;
    static const ResponseStatus HTTP_NOT_FOUND;
    static const ResponseStatus HTTP_INTERNAL_SERVER_ERROR;
	static const ResponseStatus HTTP_BAD_REQUEST;
	static const ResponseStatus HTTP_FORBIDDEN;
	static const ResponseStatus HTTP_NOT_IMPLEMENTED;
	static const ResponseStatus HTTP_SERVICE_UNAVAILABLE;
};

struct ResponseHeaders {
	std::string content_type;
	std::string content_length;
	std::string connection;
};



class HttpResponse
{
private:
	ResponseStatus _status_line;
	ResponseHeaders _headers;
	std::string _body;

public:
	static const std::string CRLF;
	static const std::string version;
	HttpResponse();
	HttpResponse(const HttpResponse &other);
	HttpResponse &operator=(const HttpResponse &other);
	~HttpResponse();
	std::string getStatusLine() const ;
	std::string getHeaders() const ;
	std::string getBody() const ;
	std::string toString() const ;
};