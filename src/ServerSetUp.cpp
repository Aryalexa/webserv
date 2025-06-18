#include "../include/WebServ.hpp"

ServerSetUp::ServerSetUp() // Check
{
	this->_port = 0;
	this->_host = 0;
	this->_server_name = "";
	this->_root = "";
	this->_client_max_body_size = MAX_CONTENT_LENGTH;
	this->_index = "";
	this->_autoindex = false;
	this->initErrorPages();
	this->_listen_fd = 0;
}

ServerSetUp::~ServerSetUp() { } // Check

ServerSetUp::ServerSetUp(const ServerSetUp &other) // Check
{
	if (this != &other)
	{
		this->_server_name = other._server_name;
		this->_root = other._root;
		this->_host = other._host;
		this->_port = other._port;
		this->_client_max_body_size = other._client_max_body_size;
		this->_index = other._index;
		this->_error_list = other._error_list;
		this->_locations = other._locations;
		this->_listen_fd = other._listen_fd;
		this->_autoindex = other._autoindex;
		this->_server_address = other._server_address;
	}
	return ;
}

ServerSetUp &ServerSetUp::operator=(const ServerSetUp & rhs) // Check
{
	if (this != &rhs)
	{
		this->_server_name = rhs._server_name;
		this->_root = rhs._root;
		this->_port = rhs._port;
		this->_host = rhs._host;
		this->_client_max_body_size = rhs._client_max_body_size;
		this->_index = rhs._index;
		this->_error_list = rhs._error_list;
		this->_locations = rhs._locations;
		this->_listen_fd = rhs._listen_fd;
		this->_autoindex = rhs._autoindex;
		this->_server_address = rhs._server_address;
	}
	return (*this);
}

void ServerSetUp::initErrorPages(void) // Check
{
	_error_list[301] = "";
	_error_list[302] = "";
	_error_list[400] = "";
	_error_list[401] = "";
	_error_list[402] = "";
	_error_list[403] = "";
	_error_list[404] = "";
	_error_list[405] = "";
	_error_list[406] = "";
	_error_list[500] = "";
	_error_list[501] = "";
	_error_list[502] = "";
	_error_list[503] = "";
	_error_list[505] = "";
	_error_list[505] = "";
}

void ServerSetUp::setServerName(std::string server_name) // Check
{
	checkSemicolon(server_name);
	this->_server_name = server_name;
}

void ServerSetUp::setHost(std::string token) // Check
{
	checkSemicolon(token);
	if (token == "localhost")
		token = "127.0.0.1";
	if (!isValidHost(token))
		throw ErrorException(SYNTAX_ERR_HOST);
	this->_host = inet_addr(token.data());
}

void ServerSetUp::setRoot(std::string root) // Check 
{
	checkSemicolon(root);
	if (ConfigFile::getTypePath(root) == 2)
	{
		this->_root = root;
		return ;
	}
	char dir[1024];
	getcwd(dir, 1024);
	std::string full_root = dir + root;
	if (ConfigFile::getTypePath(full_root) != 2)
		throw ErrorException(SYNTAX_ERR_ROOT);
	this->_root = full_root;
}

void ServerSetUp::setPort(std::string token) // Check 
{
	unsigned int port;
	
	port = 0;
	checkSemicolon(token);
	for (size_t i = 0; i < token.length(); i++)
	{
		if (!std::isdigit(token[i]))
			throw ErrorException(SYNTAX_ERR_PORT);
	}
	port = ft_stoi((token));
	if (port < 1 || port > 65636)
		throw ErrorException(SYNTAX_ERR_PORT);
	this->_port = (uint16_t) port;
}

void ServerSetUp::setClientMaxBodySize(std::string token) // Check
{
	unsigned long body_size;
	
	body_size = 0;
	checkSemicolon(token);
	for (size_t i = 0; i < token.length(); i++)
	{
		if (token[i] < '0' || token[i] > '9')
			throw ErrorException(SYNTAX_ERR_CLIENT_MAX_SIZE);
	}
	if (!ft_stoi(token))
		throw ErrorException(SYNTAX_ERR_CLIENT_MAX_SIZE);
	body_size = ft_stoi(token);
	this->_client_max_body_size = body_size;
}

void ServerSetUp::setIndex(std::string index) //Check 
{
	checkSemicolon(index);
	this->_index = index;
}

void ServerSetUp::setAutoindex(std::string autoindex) //Check
{
	checkSemicolon(autoindex);
	if (autoindex != "on" && autoindex != "off")
		throw ErrorException(SYNTAX_ERR_AUTOINDEX);
	if (autoindex == "on")
		this->_autoindex = true;
}

void	ServerSetUp::setFd(int fd) //Check
{
	this->_listen_fd = fd;
}

void ServerSetUp::setErrorPages(std::vector<std::string> &token) // Check
{
	if (token.empty())
		return;
	if (token.size() % 2 != 0)
		throw ErrorException (PAGE_ERR_INIT);
	for (size_t i = 0; i < token.size() - 1; i++)
	{
		for (size_t j = 0; j < token[i].size(); j++) {
			if (!std::isdigit(token[i][j]))
				throw ErrorException(PAGE_ERR_CODE);
		}
		if (token[i].size() != 3)
			throw ErrorException(PAGE_ERR_CODE);
		short code_error = ft_stoi(token[i]);
		if (statusCodeString(code_error) == UNDEFINED|| code_error < 400)
			throw ErrorException (INCORRECT_ERR_CODE + token[i]);
		i++;
		std::string path = token[i];
		checkSemicolon(path);
		if (ConfigFile::getTypePath(path) != 2)
		{
			if (ConfigFile::getTypePath(this->_root + path) != 1)
				throw ErrorException (INCORRECT_ERR_PATH + this->_root + path);
			if (ConfigFile::checkFile(this->_root + path, 0) == -1 || ConfigFile::checkFile(this->_root + path, 4) == -1)
				throw ErrorException (INCORRECT_ERR_PATH + this->_root + path + UNACCESSIBLE);
		}
		std::map<short, std::string>::iterator it = this->_error_list.find(code_error);
		if (it != _error_list.end())
			this->_error_list[code_error] = path;
		else
			this->_error_list.insert(std::make_pair(code_error, path));
	}
}

void ServerSetUp::setLocation(std::string path, std::vector<std::string> token) // Check
{
	LocationParser new_location;

	std::vector<std::string> methods;
	bool flag_methods = false;
	bool flag_autoindex = false;
	bool flag_max_size = false;
	int valid;

	new_location.setPath(path);
	for (size_t i = 0; i < token.size(); i++)
	{
		if (token[i] == ROOT && (i + 1) < token.size())
		{
			if (!new_location.getRootLocation().empty())
				throw ErrorException(ROOT_DUP_ERR);
			checkSemicolon(token[++i]);
			if (ConfigFile::getTypePath(token[i]) == 2)
				new_location.setRootLocation(token[i]);
			else
				new_location.setRootLocation(this->_root + token[i]);
		}
		else if ((token[i] == ALLOW_METHODS || token[i] == METHODS) && (i + 1) < token.size())
		{
			if (flag_methods)
				throw ErrorException(METHODS_DUP_ERR);
			std::vector<std::string> methods;
			while (++i < token.size())
			{
				if (token[i].find(";") != std::string::npos)
				{
					checkSemicolon(token[i]);
					methods.push_back(token[i]);
					break ;
				}
				else
				{
					methods.push_back(token[i]);
					if (i + 1 >= token.size())
						throw ErrorException(TOKEN_ERR);
				}
			}
			new_location.setMethods(methods);
			flag_methods = true;
		}
		else if (token[i] == AUTOINDEX && (i + 1) < token.size())
		{
			if (path == CGI_BIN_PATH)
				throw ErrorException(AUTOINDEX_ERR);
			if (flag_autoindex)
				throw ErrorException(AUTOINDEX_DUP_ERR);
			checkSemicolon(token[++i]);
			new_location.setAutoindex(token[i]);
			flag_autoindex = true;
		}
		else if (token[i] == INDEX && (i + 1) < token.size())
		{
			if (!new_location.getIndexLocation().empty())
				throw ErrorException(INDEX_DUP_ERR);
			checkSemicolon(token[++i]);
			new_location.setIndexLocation(token[i]);
		}
		else if (token[i] == RETURN && (i + 1) < token.size())
		{
			if (path == CGI_BIN_PATH)
				throw ErrorException(RETURN_ERR_CGI);
			if (!new_location.getReturn().empty())
				throw ErrorException(RETURN_DUP_ERR);
			checkSemicolon(token[++i]);
			new_location.setReturn(token[i]);
		}
		else if (token[i] == ALIAS && (i + 1) < token.size())
		{
			if (path == CGI_BIN_PATH)
				throw ErrorException(ALIAS_ERR_CGI);
			if (!new_location.getAlias().empty())
				throw ErrorException(ALIAS_DUP_ERR);
			checkSemicolon(token[++i]);
			new_location.setAlias(token[i]);
		}
		else if (token[i] == CGI_EXIT && (i + 1) < token.size())
		{
			std::vector<std::string> extension;
			while (++i < token.size())
			{
				if (token[i].find(";") != std::string::npos)
				{
					checkSemicolon(token[i]);
					extension.push_back(token[i]);
					break ;
				}
				else
				{
					extension.push_back(token[i]);
					if (i + 1 >= token.size())
						throw ErrorException(TOKEN_ERR);
				}
			}
			new_location.setCgiExtension(extension);
		}
		else if (token[i] == CGI_PATH && (i + 1) < token.size())
		{
			std::vector<std::string> path;
			while (++i < token.size())
			{
				if (token[i].find(";") != std::string::npos)
				{
					checkSemicolon(token[i]);
					path.push_back(token[i]);
					break ;
				}
				else
				{
					path.push_back(token[i]);
					if (i + 1 >= token.size())
						throw ErrorException(TOKEN_ERR);
				}
				if (token[i].find("/python") == std::string::npos && token[i].find("/bash") == std::string::npos)
					throw ErrorException(INVLAID_CGI_ERR);
			}
			new_location.setCgiPath(path);
		}
		else if (token[i] == CMBS && (i + 1) < token.size())
		{
			if (flag_max_size)
				throw ErrorException(CMBS_DUP_ERR);
			checkSemicolon(token[++i]);
			new_location.setMaxBodySize(token[i]);
			flag_max_size = true;
		}
		else if (i < token.size())
			throw ErrorException(TOKEN_ERR);
	}
	if (new_location.getPath() != CGI_BIN_PATH && new_location.getIndexLocation().empty())
		new_location.setIndexLocation(this->_index);
	if (!flag_max_size)
		new_location.setMaxBodySize(this->_client_max_body_size);
	valid = isValidLocation(new_location);
	if (valid == 1)
		throw ErrorException(CGI_ERR_VALIDATION);
	else if (valid == 2)
		throw ErrorException(LOCATION_ERR_VALIDATION);
	else if (valid == 3)
		throw ErrorException(REDIRECTION_ERR_VALIDATION);
	else if (valid == 4)
		throw ErrorException(ALIAS_ERR_VALIDATION);
	this->_locations.push_back(new_location);
}

int ServerSetUp::isValidLocation(LocationParser &location) const // Check
{
	if (location.getPath() == CGI_BIN_PATH)
	{
		if (location.getCgiPath().empty() || location.getCgiExtension().empty() || location.getIndexLocation().empty())
			return (1);


		if (ConfigFile::checkFile(location.getIndexLocation(), 4) < 0)
		{
			std::string path = location.getRootLocation() + location.getPath() + "/" + location.getIndexLocation();
			if (ConfigFile::getTypePath(path) != 1)
			{				
				std::string root = getcwd(NULL, 0);
				location.setRootLocation(root);
				path = root + location.getPath() + "/" + location.getIndexLocation();
			}
			if (path.empty() || ConfigFile::getTypePath(path) != 1 || ConfigFile::checkFile(path, 4) < 0)
				return (1);
		}
		if (location.getCgiPath().size() != location.getCgiExtension().size())
			return (1);
		std::vector<std::string>::const_iterator it;
		for (it = location.getCgiPath().begin(); it != location.getCgiPath().end(); ++it)
		{
			if (ConfigFile::getTypePath(*it) < 0)
				return (1);
		}
		std::vector<std::string>::const_iterator it_path;
		for (it = location.getCgiExtension().begin(); it != location.getCgiExtension().end(); ++it)
		{
			std::string tmp = *it;
			if (tmp != ".py" && tmp != ".sh" && tmp != "*.py" && tmp != "*.sh")
				return (1);
			for (it_path = location.getCgiPath().begin(); it_path != location.getCgiPath().end(); ++it_path)
			{
				std::string tmp_path = *it_path;
				if (tmp == ".py" || tmp == "*.py")
				{
					if (tmp_path.find("python") != std::string::npos)
						location._ext_path.insert( std::make_pair(std::string(".py"), tmp_path) );
				}
				else if (tmp == ".sh" || tmp == "*.sh")
				{
					if (tmp_path.find("bash") != std::string::npos)
						location._ext_path[".sh"] = tmp_path;
				}
			}
		}
		if (location.getCgiPath().size() != location.getExtensionPath().size())
			return (1);
	}
	else
	{
		if (location.getPath()[0] != '/')
			return (2);
		if (location.getRootLocation().empty()) {
			location.setRootLocation(this->_root);
		}
		if (ConfigFile::isFileExistAndReadable(location.getRootLocation() + location.getPath() + "/", location.getIndexLocation()))
			return (5);
		if (!location.getReturn().empty())
		{
			if (ConfigFile::isFileExistAndReadable(location.getRootLocation(), location.getReturn()))
				return (3);
		}
		if (!location.getAlias().empty())
		{
			if (ConfigFile::isFileExistAndReadable(location.getRootLocation(), location.getAlias()))
			 	return (4);
		}
	}
	return (0);
}

bool ServerSetUp::isValidHost(std::string host) const // Check
{
	struct sockaddr_in sockaddr;
	if (inet_pton(AF_INET, host.c_str(), &(sockaddr.sin_addr)))
		return true;
	else
		return false;
}

bool ServerSetUp::isValidErrorPages() // Check
{
	std::map<short, std::string>::const_iterator it;
	for (it = this->_error_list.begin(); it != this->_error_list.end(); it++)
	{
		if (it->first < 100 || it->first > 599)
			return (false);
		if (ConfigFile::checkFile(getRoot() + it->second, 0) < 0 || ConfigFile::checkFile(getRoot() + it->second, 4) < 0)
			return (false);
	}
	return (true);
}

const std::string &ServerSetUp::getServerName() //Check
{
	return (this->_server_name);
}

const std::string &ServerSetUp::getRoot() //Check
{
	return (this->_root);
}

const bool &ServerSetUp::getAutoindex() //Check
{
	return (this->_autoindex);
}

const in_addr_t &ServerSetUp::getHost() const { return this->_host; }

const uint16_t &ServerSetUp::getPort() const  { return this->_port; }

const size_t &ServerSetUp::getClientMaxBodySize() //Check
{
	return (this->_client_max_body_size);
}

const std::vector<LocationParser> &ServerSetUp::getLocations() //Check
{
	return (this->_locations);
}

const std::map<short, std::string> &ServerSetUp::getErrorPages() //Check
{
	return (this->_error_list);
}

const std::string &ServerSetUp::getIndex() //Check
{
	return (this->_index);
}

int ServerSetUp::getFd() const { return this->_listen_fd; }

const std::string &ServerSetUp::getPathErrorPage(short key) // Check
{
	std::map<short, std::string>::iterator it = this->_error_list.find(key);
	if (it == this->_error_list.end())
		throw ErrorException(ERR_PAGE_ERR);
	return (it->second);
}

const std::vector<LocationParser>::iterator ServerSetUp::getLocationKey(std::string key) // Check 
{
	std::vector<LocationParser>::iterator it;
	for (it = this->_locations.begin(); it != this->_locations.end(); it++)
	{
		if (it->getPath() == key)
			return (it);
	}
	throw ErrorException(LOCATION_ERR);
}

void ServerSetUp::checkSemicolon(std::string &token) // Check 
{
	size_t pos = token.rfind(';');
	if (pos != token.size() - 1)
		throw ErrorException(TOKEN_ERR);
	token.erase(pos);
}

bool ServerSetUp::checkLocations() const // Check
{
	if (this->_locations.size() < 2)
		return (false);
	std::vector<LocationParser>::const_iterator it1;
	std::vector<LocationParser>::const_iterator it2;
	for (it1 = this->_locations.begin(); it1 != this->_locations.end() - 1; it1++) {
		for (it2 = it1 + 1; it2 != this->_locations.end(); it2++) {
			if (it1->getPath() == it2->getPath())
				return (true);
		}
	}
	return (false);
}

void ServerSetUp::setUpIndividualServer()
{
    _listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listen_fd == -1)
        throw ErrorException(std::string(SOCKET_ERR) + strerror(errno));

    int opt = 1;
    if (setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        throw ErrorException(std::string(SET_SOCKET_ERR) + strerror(errno));

	set_nonblocking(_listen_fd);

    memset(&_server_address, 0, sizeof(_server_address));
    _server_address.sin_family = AF_INET;
    _server_address.sin_addr.s_addr = _host;
	// _server_address.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any IP
    _server_address.sin_port = htons(_port); 

    if (bind(_listen_fd, reinterpret_cast<sockaddr*>(&_server_address),
             sizeof(_server_address)) == -1)
        throw ErrorException(std::string(BIND_ERR) + strerror(errno));
}

