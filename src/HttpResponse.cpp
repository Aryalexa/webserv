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
  _headers.location = "";

  /*
    - Find the correct resource or action
    - Check HTTP method rules
    - Apply redirections or aliases
    - Locate the file or resource
    - Consider CGI (dynamic content)
    - Determine the response status
    */

  if (request.getMethod() == "POST" && request.getBody().empty()) {
      // Body vacío: error 400
      _status_line = ResponseStatus(HttpStatusCode::NoContent, "No content");
      _body = "<h1>204 No Content</h1>";
      _headers.content_type = "text/html";
      _headers.content_length = to_string(_body.size());
      _headers.connection = "close";
      return;
  }
  else if (request.getMethod() == "GET") 
    handle_GET();
  else if (request.getMethod() == "POST" && request.getPath() == "www/upload") {
    logDebug("[DEBUG] Body size: %i", request.getBody().size());
    Cgi cgi("cgi-bin/saveFile.py");
    std::string cgi_output = cgi.run(request);
    createOk();
  }
  else if (request.getMethod() == "DELETE" ) {
    if(request.getQuery().empty()) {
      isNotFound();
    } else {
      std::cout << "[DEBUG] path: " << request.getPath() << std::endl;
      Cgi cgi("cgi-bin/deleteFile.py");
      std::string cgi_output = cgi.run(request);
      isOk();
    }
  }
  
  if (_status_line.code == 0) {
    _status_line = ResponseStatus(HttpStatusCode::BadRequest, "Bad Request");
    _body = "<h1>400 Bad Request</h1>";
    _headers.content_type = "text/html";
    _headers.content_length = to_string(_body.size());
    _headers.connection = "keep-alive";
}

  //añadido _headers.location.empty() porque si es una redireccion , el body esta vacio 
  //(_body.empty() && _headers.location.empty()) quito ambos porque delete no tiene ni location ni body
  // if (_headers.content_type.empty() || _headers.connection.empty()
  // || _status_line.code == 0) {
  //   logError("bad");
  //   exit(2);
  // }
}

std::string get_default_error_page(int errorCode) {
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
  _body = get_default_error_page(errorCode);

  _headers.content_type = "text/html";
  _headers.content_length = to_string(_body.size());
  // a ser pagina de error, cerrar conexion
  _headers.connection = "close";

}

HttpResponse::HttpResponse(const Request &request, int errorCode, const std::string errorPagePath) : _request(request) {
  _status_line = ResponseStatus(errorCode, statusCodeString(errorCode));
  std::string valid_path = errorPagePath;
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
    std::string headers =
        "Content-Type: " + _headers.content_type + "\r\n" +
        "Content-Length: " + _headers.content_length + "\r\n" +
        "Connection: " + _headers.connection + "\r\n";
    if (!_headers.location.empty())
        headers += "Location: " + _headers.location + "\r\n";
    return headers;
}
std::string HttpResponse::getBody() const {
  return _body;
}
std::string HttpResponse::toString() const {
  return getStatusLine() + "\r\n" +
       getHeaders() + "\r\n" +
       getBody();
}

/**
 * checks if the file exists, otherwise throw 404
 */
std::string validate_path(const std::string &path) {
  std::ifstream file(path.c_str());
  if (!file.is_open()) {
    logError("File not found: %s", path.c_str());
    throw HttpException(HttpStatusCode::NotFound);
  }
  return path;
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
  if (file_path == "www/photo-detail.html") {
    std::string html = read_file_text("www/photo-detail.html");

    Cgi cgi("cgi-bin/getFile.py");
    std::string photo_block = cgi.run(_request);
    html = replace_all(html, "<!--PHOTO_DETAIL-->", photo_block);

    _body = html;
    _headers.content_type = "text/html";
    _headers.content_length = to_string(_body.size());
    _headers.connection = "keep-alive";

    int code = HttpStatusCode::OK;
    _status_line = ResponseStatus(code,statusCodeString(code) );
    return;
  }
  if(file_path == "www/index.html") {
    generate_index(_request);
    return;
  }
  // else
  _body = read_file_binary(file_path);

  _headers.content_type = discover_content_type(file_path);
  _headers.content_length = to_string(_body.size());
  _headers.connection = "keep-alive";

  int code = HttpStatusCode::OK;
  _status_line = ResponseStatus(code,statusCodeString(code) );
  
}

std::string HttpResponse::getResponse() const {
  return toString();
}

void HttpResponse::generate_index(const Request& request) {
    std::ifstream file("www/index.html");
    if (!file.is_open()) {
      throw HttpException(HttpStatusCode::NotFound);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string html = buffer.str();

    Cgi cgi("cgi-bin/getIndex.py");
    std::string galeria = cgi.run(request); // Puedes pasar un Request vacío o uno simulado

    size_t pos = html.find("<!--GALERIA-->");
    if (pos != std::string::npos)
        html.replace(pos, std::string("<!--GALERIA-->").length(), galeria);

    _body = html;

    _headers.content_type = "text/html";
    _headers.content_length = to_string(html.size());
    _headers.connection = "keep-alive";

    int code = HttpStatusCode::OK;
    _status_line = ResponseStatus(code,statusCodeString(code) );
}

void HttpResponse::redirect() {
    int code = HttpStatusCode::SeeOther;
    _status_line = ResponseStatus(code, statusCodeString(code));
    _headers.content_type = "text/html";
    _headers.content_length = "0";
    _headers.connection = "keep-alive";
    _headers.location = "/"; // Debes agregar este campo en tu struct de headers

    _body = ""; // Sin cuerpo
}

void HttpResponse::createOk() {
    int code = HttpStatusCode::Created;
    _status_line = ResponseStatus(code, statusCodeString(code));
    _headers.content_type = "text/html";
    _headers.content_length = "0";
    _headers.connection = "keep-alive";
    _body = ""; // Sin cuerpo
}

void HttpResponse::isOk() {
    int code = HttpStatusCode::OK;
    _status_line = ResponseStatus(code, statusCodeString(code));
    _headers.content_type = "text/html";
    _headers.content_length = "0";
    _headers.connection = "keep-alive";

    _body = ""; // Sin cuerpo
}

void HttpResponse::isNotFound() {
    int code = HttpStatusCode::NotFound;
    _status_line = ResponseStatus(code, statusCodeString(code));
    _headers.content_type = "text/html";
    _headers.content_length = "0";
    _headers.connection = "close";

    _body = ""; // Sin cuerpo
}
