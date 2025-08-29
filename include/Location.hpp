#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "../include/WebServ.hpp"

#define ERR_ROOT_ALIAS "Error: Alias and Root cannot coexist in the same location"
#define ERR_ROOT_LOCATION "Error: Root must be a valid directory"
#define ERR_SUPPORT_METHOD "Error: Allow Method not Supported "

class Location
{
	private:
		std::string					_path;
		std::string					_root;
		bool						_autoindex;
		std::string					_index;
		std::vector<short>			_methods;
		std::string					_return;
		std::string					_alias;
		std::vector<std::string>	_cgi_path;
		std::vector<std::string>	_cgi_ext;
		unsigned long				_client_max_body_size;

	public:
		Location();
		Location(const Location &other);
		Location &operator=(const Location &rhs);
		~Location();

		std::map<std::string, std::string> _ext_path;

		void                                        setPath(std::string token);
		void                                        setRootLocation(std::string token);
		void                                        setMethods(std::vector<std::string> methods);
		void                                        setAutoindex(std::string token);
		void                                        setIndexLocation(std::string token);
		void                                        setReturn(std::string token);
		void                                        setAlias(std::string token);
		void                                        setCgiPath(std::vector<std::string> path);
		void                                        setCgiExtension(std::vector<std::string> extension);
		void                                        setMaxBodySize(std::string token);
		void                                        setMaxBodySize(unsigned long token);

		const std::string                           &getPath() const;
		const std::string                           &getRootLocation() const;
		const std::vector<short>                    &getMethods() const;
		const bool                                  &getAutoindex() const;
		const std::string                           &getIndexLocation() const;
		const std::string                           &getReturn() const;
		const std::string                           &getAlias() const;
		const std::vector<std::string>              &getCgiPath() const;
		const std::vector<std::string>              &getCgiExtension() const;
		const std::map<std::string, std::string>    &getExtensionPath() const;
		const unsigned long                         &getMaxBodySize() const;
		std::string                                 getPrintMethods() const;

};

#endif