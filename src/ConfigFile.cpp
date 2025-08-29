#include "../include/WebServ.hpp"

ConfigFile::ConfigFile() : _size(0) { }

ConfigFile::ConfigFile(std::string const path) : _path(path), _size(0) { }

ConfigFile::~ConfigFile() { }

/**
 * ¿Es archivo (`1:F_REGULAR_FILE`), directorio (`2:F_DIRECTORY`) o inexistente (`-1:F_NOT_EXIST`)?
 */
int ConfigFile::getTypePath(std::string const path)
{
	struct stat	buffer;
	int			result;
	
	result = stat(path.c_str(), &buffer);
	if (result == 0)
	{
		if (buffer.st_mode & S_IFREG)
			return (F_REGULAR_FILE);
		else if (buffer.st_mode & S_IFDIR)
			return (F_DIRECTORY);
		else
			return (F_OTHER);
	}
	else
		return (F_NOT_EXIST);
}

/**
 * Comprueba permisos con `access()`
 * (F_OK): Comprueba si el archivo existe.
 * (R_OK): Comprueba si el archivo es legible (permiso de lectura).
 */
bool	ConfigFile::checkFile(std::string const path, int mode)
{
	return (access(path.c_str(), mode) == 0);
}

/**
 * Test exclusivo para páginas índice.
 * Comprueba si el archivo existe y es legible.
 * Si `index` es un archivo, comprueba si existe y es legible.
 * Si `index` es un directorio, comprueba si existe y es legible.
 * Si `index` es un archivo dentro de `path`, comprueba si existe y es legible.
 * Devuelve `0` si es legible, `-1` si no
 */
bool ConfigFile::isFileExistAndReadable(std::string const path, std::string const index)
{
	if (getTypePath(index) == F_REGULAR_FILE
		&& checkFile(index, R_OK))
		return (true);
	if (getTypePath(path + index) == F_REGULAR_FILE
		&& checkFile(path + index, R_OK))
		return (true);
	return (false);
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
