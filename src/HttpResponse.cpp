#include "../include/WebServ.hpp"

const std::string HttpResponse::CRLF = "\r\n";
const std::string HttpResponse::version = "HTTP/1.1";

ResponseStatus::ResponseStatus()
        : code(0), message("Empty") {}
ResponseStatus::ResponseStatus(int code, const std::string& message)
        : code(code), message(message) {}

HttpResponse::HttpResponse(const Request &request) : _request(request) {
  _status_line = ResponseStatus(0, "");
  _body = "";
  _headers.content_type = "";
  _headers.content_length = "0";
  _headers.connection = "";

  /*
    - Find the correct resource or action
    - Check HTTP method rules
    - Apply redirections or aliases
    - Locate the file or resource
    - Consider CGI (dynamic content)
    - Determine the response status
    */

  if (request.getMethod() == "GET") 
    handle_GET();

  if ( _body.empty() || _headers.content_type.empty() || _headers.connection.empty()
  || _status_line.code == 0) {
    logError("bad");
    exit(2);
  }
 
}

std::string get_deafult_error_page(int errorCode) {
  std::string errorMessage = statusCodeString(errorCode);
  
  std::stringstream htmlResponse;
  htmlResponse << "<!DOCTYPE html>"
                << "<html lang=\"en\">"
                << "<head>"
                << "<meta charset=\"UTF-8\">"
                << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
                << "<title>Error - " << errorCode << "</title>"
                << "<style>"
                << "body { font-family: Arial, sans-serif; text-align: center; padding: 20px; color: #333; background-color: #f8f8f8; }"
                << ".error { font-size: 48px; font-weight: bold; color: #e74c3c; }"
                << ".message { font-size: 24px; color: #555; }"
                << "</style>"
                << "</head>"
                << "<body>"
                << "<div class=\"error\">" << errorCode << " - " << errorMessage << "</div>"
                << "<div class=\"message\">"

                // Puedes agregar un mensaje personalizado para cada error
                << "We are sorry."
                << "</div>"
                << "</body>"
                << "</html>";
  return htmlResponse.str();
}

HttpResponse::HttpResponse(const Request &request, int errorCode) : _request(request) {
  _status_line = ResponseStatus(errorCode, statusCodeString(errorCode));
  _body = get_deafult_error_page(errorCode);

  _headers.content_type = "text/html";
  _headers.content_length = to_string(_body.size());
  _headers.connection = "keep-alive";

}
HttpResponse::HttpResponse(const Request &request, int errorCode, const std::string errorPagePath) : _request(request) {
  _status_line = ResponseStatus(errorCode, statusCodeString(errorCode));
  std::string valid_path = errorPagePath.substr(1, errorPagePath.size() - 1);
  _body = read_file_binary(valid_path);

  _headers.content_type = "text/html";
  _headers.content_length = to_string(_body.size());
  _headers.connection = "keep-alive";
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
    logError("File not found: %s", valid_path.c_str());
    throw HttpException(HttpStatusCode::NotFound);
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
  _body = read_file_binary(file_path);

  _headers.content_type = discover_content_type(file_path);
  _headers.content_length = to_string(_body.size());
  _headers.connection = "keep-alive";

  int code = HttpStatusCode::Accepted;
  _status_line = ResponseStatus(code,statusCodeString(code) );
  
}

std::string HttpResponse::getResponse() const {
  return toString();
}
