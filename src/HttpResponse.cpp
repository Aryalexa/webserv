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

HttpResponse::HttpResponse(const HttpRequest &request) : _request(request) {
	_status_line = ResponseStatus::HTTP_OK;
	_body = "";
	_headers.content_type = "text/plain";
	_headers.content_length = std::to_string(_body.size());
	_headers.connection = "close";


	if (request.getMethod() == "GET") 
		handle_GET();
    
    // else {
    //     // Manejar métodos no soportados
    //     return "HTTP/1.1 501 Not Implemented\r\nContent-Type: text/plain\r\n\r\nMethod Not Implemented";
    // }
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

void HttpResponse::handle_GET() {
	std::string file_path;
	HttpRequest req = _request;

    if (req.getPath() == "/" || req.getPath().empty()) {
        file_path = "www/index.html";
    } else {
        file_path = "www" + req.getPath();
    }
    std::ifstream file(file_path.c_str(), std::ios::binary);
    if (!file.is_open()) {
        // TODO: Manejar error: archivo no encontrado
        //std::string r = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404 Not Found</h1>";
		logError("File not found: " + file_path);
		return;
	}

    std::stringstream buffer;
    buffer << file.rdbuf();
    _body = buffer.str();

    std::string content_type = "text/html";
    if (file_path.find(".css") != std::string::npos) content_type = "text/css";
    else if (file_path.find(".js") != std::string::npos) content_type = "application/javascript";
    else if (file_path.find(".jpg") != std::string::npos || file_path.find(".jpeg") != std::string::npos) content_type = "image/jpeg";
    else if (file_path.find(".png") != std::string::npos) content_type = "image/png";

	_headers.content_type = content_type;
	_headers.content_length = std::to_string(_body.size());
	_headers.connection = "close";
	_status_line = ResponseStatus::HTTP_OK;
	
	// Construir la respuesta HTTP

    
}

std::string HttpResponse::getResponse() const {
	return toString();
}

std::string HttpResponse::toString() const {
	// std::string response = "HTTP/1.1 200 OK\r\n";
    // response += "Content-Type: " + content_type + "\r\n";
    // response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    // response += "\r\n";
    // response += body;
	return getStatusLine() + "\r\n" +
		   getHeaders() + "\r\n" +
		   getBody();
}