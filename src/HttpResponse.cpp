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

HttpResponse::HttpResponse(const Request &request) : _request(request) {
	_status_line = ResponseStatus::HTTP_OK;
	_body = "";
	_headers.content_type = "text/plain";
	_headers.content_length = to_string(_body.size());
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
	return version + " " + to_string(_status_line.code) + " " + _status_line.message;
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

std::string validate_path(const std::string &path) {
	std::string valid_path;
	if (path == "/" || path.empty()) {
        valid_path = "www/index.html";
    } else {
        valid_path = "www" + path;
    }
    std::ifstream file(valid_path.c_str());
    if (!file.is_open()) {
        // TODO: Manejar error: archivo no encontrado
        //std::string r = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404 Not Found</h1>";
		logError("File not found: %s", valid_path.c_str());
		throw std::runtime_error("file not found");
		exit(2);
	}
	return valid_path;
}
std::string discover_content_type(const std::string &filename) {
	std::string content_type;

    if (in_str(".css", filename))
		content_type = "text/css";
    else if (in_str(".js", filename))
		content_type = "application/javascript";
    else if (in_str(".jpg", filename) || in_str(".jpeg", filename))
		content_type = "image/jpeg";
    else if (in_str(".png", filename))
		content_type = "image/png";
	else 
		content_type = "text/html";

	return content_type;
}
void HttpResponse::handle_GET() {

	std::string file_path = validate_path(_request.getPath());
    std::ifstream file(file_path.c_str(), std::ios::binary);
	if (!file) {
		logError("No se pudo abrir el archivo.");
		throw std::runtime_error("no se pudo abrir");
		exit(1);
	}
    std::stringstream buffer;
    buffer << file.rdbuf();
    _body = buffer.str();

	_headers.content_type = discover_content_type(file_path);
	_headers.content_length = to_string(_body.size());
	_headers.connection = "keep-alive";

	_status_line = ResponseStatus::HTTP_OK;
	
}

std::string HttpResponse::getResponse() const {
	return toString();
}
