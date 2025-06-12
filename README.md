
# WebServ: Servidor HTTP

## Introducción al Proyecto

- **1. Capa de configuración**  
  Cuando el servidor se arranca, abre un *socket* y comienza a escuchar en un puerto especificado (ej. `8001`). Es como si una tienda levantara la persiana y anunciara: «estoy lista para atender clientes». Si un navegador se conecta, el servidor acepta la conexión y se establece un canal de comunicación.

- **2. Capa de ejecución**  
  Una vez conectados, los clientes envían peticiones HTTP que llegan como un flujo de bytes. 

---

## Estructura del Proyecto – Parte **Ana Paula**

### 1. Capa de Configuración

Esta capa convierte el archivo de texto `config/default.config` en estructuras C++ listas para usar en tiempo de ejecución.

#### Clases principales

----------------------------------------------------------------------------------------------
`ConfigFile`: Utilidades de sistema de archivos (existencia, permisos, lectura completa)
`LocationParser`: Almacena y valida directivas de cada bloque `location { … }`
`ServerSetUp`: Representa y valida un bloque `server { … }`; crea el *socket* de escucha
`ReadConfig`: Orquestador: lee el fichero, elimina comentarios, tokeniza, invoca a los *parsers* anteriores

# ConfigFile

## Responsabilidad
Valida y opera sobre rutas y ficheros:

* **`getTypePath(path)`** – ¿Es archivo (`1`), directorio (`2`) o inexistente (`-1`)?
* **`checkFile(path, mode)`** – Comprueba permisos con `access()`.
* **`isFileExistAndReadable(base, index)`** – Test exclusivo para páginas índice.
* **`readFile(path)`** – Devuelve el fichero completo como `std::string`.

### Por qué importa
Todo parser (server o location) necesita saber si un recurso realmente existe antes de aceptarlo.

---

# LocationParser

## Responsabilidad
Mantiene las directivas de una ruta (`location`) concreta:

* `path`, `root`, `alias`, `return` (redirección)
* `methods` (`GET`, `POST`, `DELETE`, …) – vector de hasta 3 flags
* `autoindex` (`on`/`off`), `index`, `cgi_path`, `cgi_ext`
* `client_max_body_size` – override por ubicación

Valida cada directiva (p. ej. solo `on`/`off` para *autoindex*) y ofrece *getters* ligeros al runtime.

### Por qué importa
Separa reglas por URL de la configuración global.  
Cuando el servidor recibe `/imagenes/foto.jpg`, sólo busca el `LocationParser` adecuado; **no** re‑parsea el archivo.

---

# ServerSetUp

## Responsabilidad
Representa un bloque completo `server { … }` y encapsula la creación del *socket* de escucha.

### Propiedades clave

* `host`, `port`, `server_name`
* `root`, `index`, `autoindex`
* `client_max_body_size`
* `std::map<int,std::string> error_pages`
* `std::vector<LocationParser> locations`
* `int _listen_fd` – descriptor de escucha

### Método principal: `setUpIndividualServer()`

1. `socket(AF_INET, SOCK_STREAM, 0)`
2. `setsockopt(SO_REUSEADDR)` – hot reload.
3. Rellena `sockaddr_in` (`sin_family`, `sin_addr`, `sin_port`).
4. `bind()`  
   *Error → lanza excepción legible (puerto ocupado, IP inválida).*
5. Devuelve y almacena el *fd*.

### Por qué importa
Centraliza **toda** la configuración de un servidor virtual; el resto del programa sólo ve «un objeto con getters y un fd listo».

---

# ReadConfig

## Responsabilidad
Parser maestro del archivo de configuración.

1. Usa `ConfigFile` para leer el texto completo.
2. Elimina comentarios (`# …`) y espacios redundantes.
3. Secciona el texto en N bloques `server { … }`.
4. Para cada bloque:
   * Instancia `ServerSetUp` y manda tokens/directivas.
   * Cada `location` se delega a su propio `LocationParser`.
5. Verifica duplicados `(host, port, server_name)`.
6. Expone `createServerGroup()` → `std::vector<ServerSetUp>` **validado**.

### Por qué importa
Aísla la sintaxis de las estructuras runtime.  
Después de `createServerGroup()` el código de red no vuelve a tocar cadenas crudas.

---

# Message

## Responsabilidad
Pequeña utilidad de logging con colores ANSI:

* `logMessage()` – info normal.
* `logError()`   – errores graves (rojo).

### Por qué importa
Unifica la salida en consola; hace el *debug* menos doloroso.

---

### 2. Capa de Ejecución

#### Clase principal – `ServerManager`

---

# ServerManager

## Visión general
Toma el vector de `ServerSetUp` generado por **ReadConfig** y lo convierte en:

1. Un conjunto de *sockets* realmente **`listen()`‑ando**.  
2. Dos `fd_set` inicializados para `select()`.  
3. El entero `_biggest_fd` que `select()` exige como primer parámetro.

## Atributos internos

| Atributo         | Tipo                         | Propósito                                                    |
| ---------------- | ---------------------------- | ------------------------------------------------------------ |
| `_servers`       | `std::vector<ServerSetUp>`   | Copia local de la configuración validada.                    |
| `_servers_map`   | `std::map<int, ServerSetUp>` | Lookup rápido **fd → servidor** (para aceptar y *routing*).  |
| `_recv_fd_pool`  | `fd_set`                     | Descriptores monitoreados para lectura (incl. listen fds).   |
| `_write_fd_pool` | `fd_set`                     | Descriptores para escritura (respuestas pendientes).         |
| `_biggest_fd`    | `int`                        | Mayor fd => arg `maxfd+1` de `select()`.                     |
| `_timeout`       | `struct timeval`             | Timeout global (p. ej. 5 segundos) para `select()`.          |

## Fases de vida

| Fase                     | Método / Acción                                               | Resultado clave                                                   |
| ------------------------ | ------------------------------------------------------------- | ----------------------------------------------------------------- |
| **Construcción**        | `ServerManager(cfgs)`                                         | Copia el vector; inicializa fd\_sets a cero.                      |
| **Setup**               | `setUpMultipleServers()`                                      | Cada server crea/reutiliza socket; se llena `_servers_map`.       |
| **Init**                | `initializeSockets()`                                         | `listen()`, `fcntl(O_NONBLOCK)`, `FD_SET()` y cálculo `_biggest_fd`. |
| **Loop principal**      | `runServers()` *(futuro)*                                     | Bucle `select()` infinito hasta señal SIGINT.                     |
| **Cierre ordenado**     | `shutdownServers()` *(futuro)*                                | `close()` de todos los fds y limpieza de memoria.                 |

## Detalle de métodos clave

### 1. `setUpMultipleServers()`

```cpp
void ServerManager::setUpMultipleServers(const std::vector<ServerSetUp>& cfgs) {
    _servers = cfgs;
    for (size_t i = 0; i < _servers.size(); ++i) {
        ServerSetUp& srv = _servers[i];
        // — 1) ¿Existe ya un fd con mismo (host,port)?
        int fd_reuse = findExistingSocket(srv.getHost(), srv.getPort());
        if (fd_reuse != -1) {
            srv.overwriteFd(fd_reuse);           // <<— reuse
        } else {
            srv.setUpIndividualServer();         // <<— new socket
        }
        _servers_map[srv.getFd()] = srv;         // (fd -> srv) map
        Message::logMessage("[Setup] %s:%d -> fd=%d (%s)",
                            srv.getHost().c_str(),
                            srv.getPort(),
                            srv.getFd(),
                            fd_reuse != -1 ? "reused" : "new");
    }
}
```

*La deduplicación ahorra *file descriptors* y permite *virtual hosts* en el mismo puerto.*

### 2. `initializeSockets()`

```cpp
void ServerManager::initializeSockets() {
    FD_ZERO(&_recv_fd_pool);
    FD_ZERO(&_write_fd_pool);
    for (size_t i = 0; i < _servers.size(); ++i) {
        int fd = _servers[i].getFd();
        listen(fd, 512);                      // backlog de 512
        fcntl(fd, F_SETFL, O_NONBLOCK);       // no-blocking
        FD_SET(fd, &_recv_fd_pool);           // track for accept()
        if (fd > _biggest_fd) _biggest_fd = fd;
    }
    _timeout.tv_sec  = 5;
    _timeout.tv_usec = 0;
    Message::logMessage("[Init] biggest_fd=%d, timeout=5s", _biggest_fd);
}
```


## Errores comunes y cómo se manejan

| Caso                                            | Componente   | Acción                                   | Código HTTP resultante |
| ----------------------------------------------- | ------------ | ---------------------------------------- | ---------------------- |
| Puerto ocupado al *bind()*                      | ServerSetUp  | Lanza excepción → captura en `main()`    | – (cierra programa)    |
| `select()` devuelve `EBADF` o `EINTR`           | ServerManager| Re‑construye `_recv_fd_pool` / re‑intenta| 500 si se detecta roto |
| Ruta estática no encontrada                     | HttpResponse | Sirve `error_pages[404]`  o mensaje gen. | 404 Not Found          |
| Límite de cuerpo superado (`client_max_body_size`)| HttpRequest  | Detiene lectura, marca error            | 413 Payload Too Large  |

---

# main()

## Responsabilidad
Punto de entrada que orquesta todo:

```cpp
int main(int ac, char** av) {
    std::string config = (ac == 2) ? av[1] : "config/default.config";
    try {
        std::vector<ServerSetUp> servers = ReadConfig::createServerGroup(config);
        ServerManager sm(servers);
        sm.setUpMultipleServers();
        sm.initializeSockets();
        sm.runServers();          // bucle infinito
    } catch (const std::exception& e) {
        Message::logError("Fatal: %s", e.what());
    }
    return 0;
}
```

---

> Con esta arquitectura, las clases de *config* convierten texto en objetos C++,  
> **ServerManager** traduce esos objetos en *sockets* listos,  
> y el futuro bucle de eventos descansa sobre esa base para ofrecer un servidor HTTP estable y extensible.
