#include "../include/WebServ.hpp"

const ResponseStatus ResponseStatus::HTTP_OK = {200, "OK"};
const ResponseStatus ResponseStatus::HTTP_NOT_FOUND = {404, "Not Found"};
const ResponseStatus ResponseStatus::HTTP_INTERNAL_SERVER_ERROR = {500, "Internal Server Error"};
const ResponseStatus ResponseStatus::HTTP_BAD_REQUEST = {400, "Bad Request"};
const ResponseStatus ResponseStatus::HTTP_FORBIDDEN = {403, "Forbidden"};
const ResponseStatus ResponseStatus::HTTP_NOT_IMPLEMENTED = {501, "Not Implemented"};
const ResponseStatus ResponseStatus::HTTP_SERVICE_UNAVAILABLE = {503, "Service Unavailable"};
const std::string HttpResponse::CRLF = "\r\n";
const std::string HttpResponse::version = "HTTP/1.1";

HttpResponse::HttpResponse() {
	_status_line = ResponseStatus::HTTP_OK;

	_body = "Hello, World!";

	_headers.content_type = "text/plain";
	_headers.content_length = std::to_string(_body.size());
	_headers.connection = "close";
}
HttpResponse::HttpResponse(const HttpResponse &other) {
	_status_line = other._status_line;
	_headers = other._headers;
	_body = other._body;
}
HttpResponse &HttpResponse::operator=(const HttpResponse &other)
{
	if (this != &other)
	{
		_status_line = other._status_line;
		_headers = other._headers;
		_body = other._body;
	}
	return *this;
}
HttpResponse::~HttpResponse() {}

std::string HttpResponse::getStatusLine() const {
	return version + " " + std::to_string(_status_line.code) + " " + _status_line.message;
}
std::string HttpResponse::getHeaders() const {
	return "Content-Type: " + _headers.content_type + "\r\n" +
		   "Content-Length: " + _headers.content_length + "\r\n" +
		   "Connection: " + _headers.connection + "\r\n";
}
std::string HttpResponse::getBody() const {
	return _body;
}
std::string HttpResponse::toString() const {
	return getStatusLine() + "\r\n" +
		   getHeaders() + "\r\n" +
		   getBody();
}