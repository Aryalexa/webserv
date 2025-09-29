#include "../include/WebServ.hpp"

Server::Server() // Check
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

Server::~Server() { } // Check

Server::Server(const Server &other) // Check
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

Server &Server::operator=(const Server & rhs) // Check
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

void Server::initErrorPages(void) // Check
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

void Server::setServerName(std::string server_name) // Check
{
    checkSemicolon(server_name);
    this->_server_name = server_name;
}

void Server::setHost(std::string token) // Check
{
    checkSemicolon(token);
    if (token == "localhost")
        token = "127.0.0.1";
    if (!isValidHost(token))
        throw ErrorException(SYNTAX_ERR_HOST);
    this->_host = inet_addr(token.data());
}

void Server::setRoot(std::string root) // Check 
{
    checkSemicolon(root);
    if (ConfigFile::getTypePath(root) == F_DIRECTORY)
    {
        this->_root = root;
        return ;
    }
    // if it is not a directory:
    char dir[1024];
    getcwd(dir, 1024);
    std::string full_root = dir + root;
    if (ConfigFile::getTypePath(full_root) != F_DIRECTORY)
        throw ErrorException(SYNTAX_ERR_ROOT);
    this->_root = full_root;
    logError("root is not a directory - averigua que pasa en este caso");
    exit(2);
}

void Server::setPort(std::string token) // Check 
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

void Server::setClientMaxBodySize(std::string token) // Check
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

void Server::setIndex(std::string index) //Check 
{
    checkSemicolon(index);
    // check it does not start with /
    if (index[0] == '/')
        throw ErrorException(SYNTAX_ERR_INDEX ": cannot start with /");
    this->_index = index;
}

void Server::setAutoindex(std::string autoindex) //Check
{
    checkSemicolon(autoindex);
    if (autoindex != "on" && autoindex != "off")
        throw ErrorException(SYNTAX_ERR_AUTOINDEX);
    if (autoindex == "on")
        this->_autoindex = true;
}

void	Server::setFd(int fd) //Check
{
    this->_listen_fd = fd;
}

void Server::setErrorPages(std::vector<std::string> &token) // Check
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
        if (ConfigFile::getTypePath(path) != F_DIRECTORY)
        {
            if (ConfigFile::getTypePath(this->_root + path) != F_REGULAR_FILE)
                throw ErrorException (INCORRECT_ERR_PATH + this->_root + path);
            if (!ConfigFile::checkFile(this->_root + path, F_OK) || !ConfigFile::checkFile(this->_root + path, R_OK))
                throw ErrorException (INCORRECT_ERR_PATH + this->_root + path + UNACCESSIBLE);
        }
        std::map<short, std::string>::iterator it = this->_error_list.find(code_error);
        if (it != _error_list.end())
            this->_error_list[code_error] = path;
        else
            this->_error_list.insert(std::make_pair(code_error, path));
    }
}

void Server::setLocation(std::string path, std::vector<std::string> token) // Check
{
    Location new_location;

    std::vector<std::string> methods;
    bool flag_methods = false;
    bool flag_autoindex = false;
    bool flag_max_size = false;
    int valid;

    new_location.setPathLocation(path);
    for (size_t i = 0; i < token.size(); i++)
    {
        if (token[i] == ROOT && (i + 1) < token.size())
        {
            if (!new_location.getRootLocation().empty())
                throw ErrorException(ROOT_DUP_ERR);
            checkSemicolon(token[++i]);
            if (ConfigFile::getTypePath(token[i]) == F_DIRECTORY)
                new_location.setRootLocation(token[i]);
            else
            {
                logDebug("setting Root location to: %s", token[i].c_str());
                exit(1) ; // TODO: quiero entender esto          
                new_location.setRootLocation(this->_root + token[i]);
            }
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
                        throw ErrorException(TOKEN_ERR ": " + token[i] + "(missing semicolon)");    
                }
            }
            new_location.setMethods(methods);
            flag_methods = true;
        }
        else if (token[i] == AUTOINDEX && (i + 1) < token.size())
        {
            if (path == CGI_BIN_PATH)
                throw ErrorException(AUTOINDEX_ERR ": cannot be set for cgi-bin");
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
            // check it does not start with /
            if (token[i][0] == '/')
                throw ErrorException(SYNTAX_ERR_INDEX ": cannot start with /");
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
                        throw ErrorException(TOKEN_ERR ": " + token[i] + "(missing semicolon)");
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
                        throw ErrorException( ": " + token[i]);
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
            throw ErrorException(TOKEN_ERR ": " + token[i] + "(unknown token)");
    }
    // if (new_location.getPathLocation() != CGI_BIN_PATH && new_location.getIndexLocation().empty())
    //     new_location.setIndexLocation(this->_index);
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

int isValidCgiLocation(Location &cgi_location) // Check
{
    if (cgi_location.getCgiPath().empty() || cgi_location.getCgiExtension().empty() || cgi_location.getIndexLocation().empty())
        return (1);

    if (!ConfigFile::checkFile(cgi_location.getIndexLocation(), R_OK))
    {
        std::string path = cgi_location.getRootLocation() + cgi_location.getPathLocation() + "/" + cgi_location.getIndexLocation();
        if (ConfigFile::getTypePath(path) != F_REGULAR_FILE)
        {				
            std::string root = getcwd(NULL, 0);
            logDebug("2 setting Root location to: %s", cgi_location.getRootLocation().c_str());
            cgi_location.setRootLocation(root);
            path = root + cgi_location.getPathLocation() + "/" + cgi_location.getIndexLocation();
        }
        if (path.empty() || ConfigFile::getTypePath(path) != F_REGULAR_FILE || !ConfigFile::checkFile(path, R_OK))
            return (1);
    }
    if (cgi_location.getCgiPath().size() != cgi_location.getCgiExtension().size())
        return (1);
    std::vector<std::string>::const_iterator it;
    for (it = cgi_location.getCgiPath().begin(); it != cgi_location.getCgiPath().end(); ++it)
    {
        if (ConfigFile::getTypePath(*it) < 0)
            return (1);
    }
    std::vector<std::string>::const_iterator it_path;
    for (it = cgi_location.getCgiExtension().begin(); it != cgi_location.getCgiExtension().end(); ++it)
    {
        std::string tmp = *it;
        if (tmp != ".py" && tmp != ".sh" && tmp != "*.py" && tmp != "*.sh")
            return (1);
        for (it_path = cgi_location.getCgiPath().begin(); it_path != cgi_location.getCgiPath().end(); ++it_path)
        {
            std::string tmp_path = *it_path;
            if (tmp == ".py" || tmp == "*.py")
            {
                if (tmp_path.find("python") != std::string::npos)
                    cgi_location._ext_path.insert( std::make_pair(std::string(".py"), tmp_path) );
            }
            else if (tmp == ".sh" || tmp == "*.sh")
            {
                if (tmp_path.find("bash") != std::string::npos)
                    cgi_location._ext_path[".sh"] = tmp_path;
            }
        }
    }
    if (cgi_location.getCgiPath().size() != cgi_location.getExtensionPath().size())
        return (1);   
    return (0);
}

int Server::isValidLocation(Location &location) const // Check
{
    if (location.getPathLocation() == CGI_BIN_PATH)
    {
        return isValidCgiLocation(location);
    }
    else
    {
        if (location.getPathLocation()[0] != '/')
            return (2);
        if (!ConfigFile::isFileExistAndReadable(location.getRootLocation() + location.getPathLocation() + "/", location.getIndexLocation()))
            return (5); //check
        if (!location.getReturn().empty())
        {
            if (!ConfigFile::isFileExistAndReadable(location.getRootLocation(), location.getReturn()))
                return (3);
        }
        if (!location.getAlias().empty())
        {
            if (!ConfigFile::isFileExistAndReadable(location.getRootLocation(), location.getAlias()))
                 return (4);
        }
    }
    return (0);
}

bool Server::isValidHost(std::string host) const // Check
{
    struct sockaddr_in sockaddr;
    if (inet_pton(AF_INET, host.c_str(), &(sockaddr.sin_addr)))
        return true;
    else
        return false;
}

bool Server::isValidErrorPages() // Check
{
    std::map<short, std::string>::const_iterator it;
    for (it = this->_error_list.begin(); it != this->_error_list.end(); it++)
    {
        if (it->first < 100 || it->first > 599)
            return (false);
        if (!ConfigFile::checkFile(getRoot() + it->second, F_OK) || !ConfigFile::checkFile(getRoot() + it->second, R_OK))
            return (false);
    }
    return (true);
}

const std::string &Server::getServerName() //Check
{
    return (this->_server_name);
}

const std::string &Server::getRoot() //Check
{
    return (this->_root);
}

const bool &Server::getAutoindex() //Check
{
    return (this->_autoindex);
}

const in_addr_t &Server::getHost() const { return this->_host; }

const uint16_t &Server::getPort() const  { return this->_port; }

const size_t &Server::getClientMaxBodySize() //Check
{
    return (this->_client_max_body_size);
}

const std::vector<Location> &Server::getLocations() //Check
{
    return (this->_locations);
}

const std::map<short, std::string> &Server::getErrorPages() //Check
{
    return (this->_error_list);
}

const std::string &Server::getIndex() //Check
{
    return (this->_index);
}

int Server::getFd() const { return this->_listen_fd; }

/**
 * Returns the path of the error page for a given HTTP status code.
 * If no custom error page is set for the given code, an empty string is returned.
 */
const std::string &Server::getPathErrorPage(short key) // Check
{
    std::map<short, std::string>::iterator it = this->_error_list.find(key);
    if (it == this->_error_list.end())
    {
        static const std::string empty = ""; // no se destruirá cuando acabe la función
        return empty;
    }
    return (it->second);
}

const std::vector<Location>::iterator Server::getLocationKey(std::string key) // Check 
{
    std::vector<Location>::iterator it;
    for (it = this->_locations.begin(); it != this->_locations.end(); it++)
    {
        if (it->getPathLocation() == key)
            return (it);
    }
    throw ErrorException(LOCATION_ERR);
}

/**
 * 
 */
void Server::checkSemicolon(std::string &token) // Check 
{
    size_t pos = token.rfind(';');
    if (pos != token.size() - 1)
        throw ErrorException(TOKEN_ERR ": " + token + " (missing semicolon)");
    token.erase(pos);
}

bool Server::checkLocations() const // Check
{
    if (this->_locations.size() < 2)
        return (false);
    std::vector<Location>::const_iterator it1;
    std::vector<Location>::const_iterator it2;
    for (it1 = this->_locations.begin(); it1 != this->_locations.end() - 1; it1++) {
        for (it2 = it1 + 1; it2 != this->_locations.end(); it2++) {
            if (it1->getPathLocation() == it2->getPathLocation())
                return (true);
        }
    }
    return (false);
}

void Server::setUpIndividualServer()
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

