#include "../include/WebServ.hpp"

ConfigFile::ConfigFile() : _size(0) { }

ConfigFile::ConfigFile(std::string const path) : _path(path), _size(0) { }

ConfigFile::~ConfigFile() { }

/**
 * ¿Es archivo (`1`), directorio (`2`) o inexistente (`-1`)?
 */
int ConfigFile::getTypePath(std::string const path)
{
	struct stat	buffer;
	int			result;
	
	result = stat(path.c_str(), &buffer);
	if (result == 0)
	{
		if (buffer.st_mode & S_IFREG)
			return (1);
		else if (buffer.st_mode & S_IFDIR)
			return (2);
		else
			return (3);
	}
	else
		return (-1);
}

/**
 * Comprueba permisos con `access()`
 */
int	ConfigFile::checkFile(std::string const path, int mode)
{
	return (access(path.c_str(), mode));
}

/**
 * Test exclusivo para páginas índice.
 * Comprueba si el archivo existe y es legible.
 * Si `index` es un archivo, comprueba si existe y es legible.
 * Si `index` es un directorio, comprueba si existe y es legible.
 * Si `index` es un archivo dentro de `path`, comprueba si existe y es legible.
 * Devuelve `0` si es legible, `-1` si no
 */
int ConfigFile::isFileExistAndReadable(std::string const path, std::string const index)
{
	if (getTypePath(index) == 1 && checkFile(index, 4) == 0)
		return (0);
	if (getTypePath(path + index) == 1 && checkFile(path + index, 4) == 0)
		return (0);
	return (-1);
}

/**
 * Lee el archivo de configuración.
 * Abre el archivo, lo lee y devuelve su contenido como `std::string`.
 */
std::string	ConfigFile::readFile(std::string path)
{
	if (path.empty() || path.length() == 0)
		return (NULL);
	std::ifstream config_file(path.c_str());
	if (!config_file || !config_file.is_open())
		return (NULL);

	std::stringstream stream_binding;
	stream_binding << config_file.rdbuf();
	return (stream_binding.str());
}

std::string ConfigFile::getPath()
{
	return (this->_path);
}

int ConfigFile::getSize()
{
	return (this->_size);
}
