#include "../include/WebServ.hpp"

HttpRequest::HttpRequest() : _method(""), _uri(""), _version("") {
	// Default constructor initializes an empty request
	// This is useful for creating an HttpRequest object without any data
}
HttpRequest::HttpRequest(std::string request_str) {
	// Parse the request string to initialize the HttpRequest object
    std::istringstream request_stream(request_str);
	std::string line;

	// Parse the request line (first line of the request)
	if (std::getline(request_stream, line)) {
		_parse_request_line(line);
	}

	// Parse headers until the blank line
	std::list<std::string> headers_lines;
	while (std::getline(request_stream, line) && !line.empty()) {
		headers_lines.push_back(line);
	}
	_parse_headers(headers_lines);

	// Parse the body (remaining part of the request after headers)
	std::string body;
	while (std::getline(request_stream, line)) {
		body += line + "\n";
	}
	_parse_body(body);
	

}
HttpRequest::HttpRequest(const HttpRequest &other) {
	// Copy constructor
	if (this != &other) {
		_method = other._method;
		_uri = other._uri;
		_version = other._version;
		_headers = other._headers;
		_body = other._body;
	}
}
HttpRequest &HttpRequest::operator=(const HttpRequest &other)
{
	if (this != &other)
	{
		_method = other._method;
		_uri = other._uri;
		_version = other._version;
		_headers = other._headers;
		_body = other._body;
	}
	return *this;
}
HttpRequest::~HttpRequest() {}

void HttpRequest::_parse_request_line(const std::string &request_line) {
	// Parse the request line to extract method, URI, and version
	// This is a placeholder; actual parsing logic should be implemented
	std::istringstream request_stream(request_line);
	std::string method, path, version;
	request_stream >> method >> path >> version;
	_method = method;
	_uri = path;
	_version = version;
	// validate they are not empty
	if (_method.empty() || _uri.empty() || _version.empty()) {
		throw std::invalid_argument("Invalid request line: " + request_line);
	}
	// check the values of method, uri and version
	// supported methods: GET, POST, PUT, DELETE
	if (_method != "GET" && _method != "POST" && _method != "PUT" && _method != "DELETE") {
		throw std::invalid_argument("Unsupported method: " + _method);
	}
	// check the uri starts with a slash and the path is valid
	if (_uri.empty() || _uri[0] != '/') {
		throw std::invalid_argument("Invalid URI: " + _uri);
	}
	if (_version != "HTTP/1.0" && _version != "HTTP/1.1") {
		throw std::invalid_argument("Unsupported HTTP version: " + _version);
	}


}

void HttpRequest::_parse_headers(const std::list<std::string> &headers_strs) {
	// Parse the headers from the list of header strings (c++98 compatible)
	std::list<std::string>::const_iterator it;
	for ( it = headers_strs.begin(); it != headers_strs.end(); ++it) {
		const std::string &header_str = *it;
		size_t pos = header_str.find(':');
		if (pos == std::string::npos) {
			throw std::invalid_argument("Invalid header format: " + header_str);
		}
		std::string key = header_str.substr(0, pos);
		std::string value = header_str.substr(pos + 1);
		// Trim whitespace from key and value
		key.erase(0, key.find_first_not_of(' '));
		key.erase(key.find_last_not_of(' ') + 1);
		value.erase(0, value.find_first_not_of(' '));
		value.erase(value.find_last_not_of(' ') + 1);
		_headers[key] = value;
	}
}

void HttpRequest::_parse_body(const std::string &body) {
    // Store the body for further processing
    _body = body;
	// if method is POST or PUT, we can parse the body further
	if (_method == "POST" || _method == "PUT") {
		// check if the body is empty
		if (_body.empty())
			throw std::invalid_argument("Empty body for POST/PUT request");
		// // TODO: LOGIC - this does not goes here !!
		// // This is a placeholder; actual parsing logic should be implemented
		// if (_headers.find("Content-Type") != _headers.end()) {
		// 	std::string content_type = _headers["Content-Type"];
		// 	if (content_type == "application/x-www-form-urlencoded") {
		// 		// Parse form data
		// 		std::istringstream body_stream(_body);
		// 		std::string key_value_pair;
		// 		while (std::getline(body_stream, key_value_pair, '&')) {
		// 			size_t pos = key_value_pair.find('=');
		// 			if (pos != std::string::npos) {
		// 				std::string key = key_value_pair.substr(0, pos);
		// 				std::string value = key_value_pair.substr(pos + 1);
		// 				// Trim whitespace from key and value
		// 				key.erase(0, key.find_first_not_of(' '));
		// 				key.erase(key.find_last_not_of(' ') + 1);
		// 				value.erase(0, value.find_first_not_of(' '));
		// 				value.erase(value.find_last_not_of(' ') + 1);
		// 				// Store the key-value pair in the body map or process it as needed
		// 				logDebug("Parsed form data: %s = %s", key.c_str(), value.c_str());
		// 			} else {
		// 				logDebug("Invalid key-value pair in form data: %s", key_value_pair.c_str());
		// 			}
		// 		}
		// 	} else if (content_type == "application/json") {}
		// }
	}

    logDebug("Parsed body: %s", _body.c_str());
}

const std::string &HttpRequest::getMethod() const {
	return _method;
}
const std::string &HttpRequest::getUri() const {
	return _uri;
}
const std::string &HttpRequest::getVersion() const {
	return _version;
}
// const std::string &HttpRequest::getHeader(const std::string &key) const {
	
// }
const std::map<std::string, std::string> &HttpRequest::getHeaders() const {
	return _headers;
}
const std::string &HttpRequest::getBody() const {
	return _body;
}
const std::string &HttpRequest::getPath() const {
	return _uri; // Assuming the path is the same as the URI
}
