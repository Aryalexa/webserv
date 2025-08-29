#include "../include/WebServ.hpp"


Location::Location()
{
	this->_path = "";
	this->_root = "";
	this->_autoindex = false;
	this->_index = "";
	this->_return = "";
	this->_alias = "";
	this->_client_max_body_size = MAX_CONTENT_LENGTH;
	this->_methods.reserve(5);
	this->_methods.push_back(1); // GET enabled by default
	this->_methods.push_back(0);
	this->_methods.push_back(0);
	this->_methods.push_back(0);
	this->_methods.push_back(0);
}

Location::Location(const Location &other)
{
	this->_path = other._path;
	this->_root = other._root;
	this->_autoindex = other._autoindex;
	this->_index = other._index;
	this->_cgi_path = other._cgi_path;
	this->_cgi_ext = other._cgi_ext;
	this->_return = other._return;
	this->_alias = other._alias;
    this->_methods = other._methods;
	this->_ext_path = other._ext_path;
	this->_client_max_body_size = other._client_max_body_size;
}

Location &Location::operator=(const Location &rhs)
{
	if (this != &rhs)
	{
		this->_path = rhs._path;
		this->_root = rhs._root;
		this->_autoindex = rhs._autoindex;
		this->_index = rhs._index;
		this->_cgi_path = rhs._cgi_path;
		this->_cgi_ext = rhs._cgi_ext;
		this->_return = rhs._return;
		this->_alias = rhs._alias;
		this->_methods = rhs._methods;
		this->_ext_path = rhs._ext_path;
		this->_client_max_body_size = rhs._client_max_body_size;
    }
	return (*this);
}

Location::~Location() { }

void Location::setPath(std::string token)
{
	this->_path = token;
}

void Location::setRootLocation(std::string token)
{
	if (ConfigFile::getTypePath(token) != F_DIRECTORY)
		throw ServerSetUp::ErrorException(ERR_ROOT_LOCATION);
	if (Location::getAlias() != "")
		throw ServerSetUp::ErrorException(ERR_ROOT_ALIAS);
	this->_root = token;
}

void Location::setMethods(std::vector<std::string> methods)
{
	this->_methods[0] = 0;
	this->_methods[1] = 0;
	this->_methods[2] = 0;
	this->_methods[3] = 0;
	this->_methods[4] = 0;

	for (size_t i = 0; i < methods.size(); i++)
	{
		if (methods[i] == "GET")
			this->_methods[M_GET] = 1;
		else if (methods[i] == "POST")
			this->_methods[M_POST] = 1;
		else if (methods[i] == "DELETE")
			this->_methods[M_DELETE] = 1;
		else if (methods[i] == "PUT")
			this->_methods[M_PUT] = 1;
		else if (methods[i] == "HEAD")
			this->_methods[M_HEAD] = 1;
		else
			throw ServerSetUp::ErrorException(ERR_SUPPORT_METHOD + methods[i]);
	}
}

void Location::setAutoindex(std::string token)
{
	if (token == "on" || token == "off")
		this->_autoindex = (token == "on");
	else
		throw ServerSetUp::ErrorException(AUTOINDEX_ERR);
}

void Location::setIndexLocation(std::string token)
{
	this->_index = token;
}

void Location::setReturn(std::string token)
{
	this->_return = token;
}

void Location::setAlias(std::string token)
{
	if (Location::getRootLocation() != "")
		throw ServerSetUp::ErrorException(ERR_ROOT_ALIAS);
	this->_alias = token;
}

void Location::setCgiPath(std::vector<std::string> path)
{
	this->_cgi_path = path;
}

void Location::setCgiExtension(std::vector<std::string> extension)
{
	this->_cgi_ext = extension;
}

void Location::setMaxBodySize(std::string token)
{
	unsigned long body_size = 0;

	for (size_t i = 0; i < token.length(); i++)
	{
		if (token[i] < '0' || token[i] > '9')
			throw ServerSetUp::ErrorException(SYNTAX_ERR_CLIENT_MAX_SIZE);
	}
	if (!ft_stoi(token))
		throw ServerSetUp::ErrorException(SYNTAX_ERR_CLIENT_MAX_SIZE);
	body_size = ft_stoi(token);
	this->_client_max_body_size = body_size;
}

void Location::setMaxBodySize(unsigned long token)
{
	this->_client_max_body_size = token;
}

const std::string &Location::getPath() const
{
	return (this->_path);
}

const std::string &Location::getRootLocation() const
{
	return (this->_root);
}

const std::string &Location::getIndexLocation() const
{
	return (this->_index);
}

const std::vector<short> &Location::getMethods() const
{
	return (this->_methods);
}

const std::vector<std::string> &Location::getCgiPath() const
{
	return (this->_cgi_path);
}

const std::vector<std::string> &Location::getCgiExtension() const
{
	return (this->_cgi_ext);
}

const bool &Location::getAutoindex() const
{
	return (this->_autoindex);
}

const std::string &Location::getReturn() const
{
	return (this->_return);
}

const std::string &Location::getAlias() const
{
	return (this->_alias);
}

const std::map<std::string, std::string> &Location::getExtensionPath() const
{
	return (this->_ext_path);
}

const unsigned long &Location::getMaxBodySize() const
{
	return (this->_client_max_body_size);
}

std::string Location::getPrintMethods() const
{
	std::string res;
	if (_methods[M_HEAD])
		res.insert(0, "HEAD");
	if (_methods[M_PUT])
	{
		if (!res.empty())
			res.insert(0, ", ");
		res.insert(0, "PUT");
	}
	if (_methods[M_DELETE])
	{
		if (!res.empty())
			res.insert(0, "\n");
		res.insert(0, "DELETE");
	}
	if (_methods[M_POST])
	{
		if (!res.empty())
			res.insert(0, "\n");
		res.insert(0, "POST");
	}
	if (_methods[M_GET])
	{
		if (!res.empty())
			res.insert(0, "\n");
		res.insert(0, "GET");
	}
	return (res);
}