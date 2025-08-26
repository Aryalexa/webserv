// paths.hpp
#ifndef PATHS_HPP
#define PATHS_HPP

#include <string>

// Carpeta raíz para archivos estáticos
const std::string WWW_ROOT = "www/";
// Archivos por defecto
const std::string DEFAULT_INDEX = "index.html";

// WIP
// Carpeta para scripts CGI
const std::string CGI_BIN = "cgi-bin/";
// Carpeta para uploads
const std::string UPLOADS_DIR = WWW_ROOT + "uploads/";
// Rutas especiales
const std::string UPLOADS_URI = "/www/uploads";

std::string		clean_path(const std::string& path);
std::string		path_normalization(const std::string& path);
#endif
