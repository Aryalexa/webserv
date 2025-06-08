# Webserv_42: Servidor HTTP en C++

## Introducción al Proyecto

**Webserv_42** Mi parte está divida en dos etapas: 

- **1. Capa de configuración:** Cuando el servidor WebServ se inicia, abre un socket de red y comienza a escuchar en un puerto especificado (por ejemplo, el puerto 8001 como se muestra en la configuración). Esto es como una tienda abriendo su puerta y esperando a los clientes. El servidor está esencialmente diciendo "Estoy listo para aceptar conexiones en el puerto 8001." Si un cliente intenta conectarse, el servidor acepta la conexión; ahora se ha abierto una línea de comunicación entre el cliente y el servidor. (Bajo el capó, el componente Server Core de WebServ maneja la creación del socket y la aceptación de conexiones como parte de sus funciones de red, pero puedes pensar en ello simplemente como establecer una nueva "conversación" con el cliente).

- **2. Capa de ejecución:** Una vez conectado, el cliente (navegador) envía un mensaje de solicitud HTTP. Este llega como un flujo de bytes de texto. WebServ utiliza su clase de analizador HttpRequest para leer este flujo y entenderlo. La tarea del analizador es tomar los datos en bruto y descomponerlos en las partes significativas de una solicitud HTTP.

## Estructura del Proyecto - Parte Ana Paula

### 1. **Capa de Configuración**

Esta capa transforma el archivo de configuración (`config/default.config`) en estructuras listas para ser utilizadas.

#### **Clases principales:**

- **`ReadConfig`**
- **`ServerParser`**
- **`LocationParser`**
- **`ConfigFile`**

---

# ConfigFile

## Responsabilidad
Un a clase que valida las operaciones del sistema de archivos. Simplemente te indica:

- **`getTypePath(path)`**  
  ¿Es esto un archivo, un directorio o ninguno de los dos?

- **`checkFile(path, mode)`**  
  ¿Puedo `access()` este archivo con permisos de lectura/ejecución?

- **`isFileExistAndReadable(base, index)`**  
  ¿Existe y es legible bien solo `index` o bien `base + index`?

- **`readFile(path)`**  
  Lee por completo el archivo en una sola cadena.

## Por qué importa
Cada otro parser (de Server o de Location) necesita validar que un directorio o página realmente exista antes de continuar. 

# LocationParser

## Responsabilidad
Almacena todas las directivas por URL (“location”) definidas en un bloque `location { … }` en la configuración:

- **`path`** (`/images`, `/cgi-bin`, `/`)  
- **`root`** – anulación de la ruta raíz para ese path  
- **`methods`** (`GET`, `POST`, `DELETE`, etc.)  
- **`autoindex`** (`on`/`off`)  
- **`index`** – archivo a servir por defecto  
- **`alias`**, **`return`** (redirección), **`cgi_path`**/`cgi_ext` (listas)  
- **`client_max_body_size`** – anulación del tamaño máximo del cuerpo de la petición  

Valida cada directiva al establecerla (por ejemplo, sólo `on`/`off` para `autoindex`; extensiones correctas para CGI) y expone getters sencillos para que en tiempo de ejecución sepa exactamente cómo manejar peticiones a esa subruta.

## Por qué importa
Separa la lógica específica de cada URL de los ajustes globales del servidor. Cuando el servidor recibe una petición a `/foo/bar`, simplemente recorre su lista de objetos `LocationParser` para aplicar las reglas adecuadas. No hace falta volver a reparsear el archivo de configuración en tiempo de ejecución.

# ServerParser

## Responsabilidad
Representa un bloque `<server> { … }` completo y centraliza toda la configuración y puesta en marcha de un servidor virtual:

- **Configuraciones globales:** `host`, `port`, `server_name`, `root`, `index`, `client_max_body_size`, `autoindex`  
- **Mapa de páginas de error:** asignación de códigos a rutas de error (por ejemplo, `404` → `/errors/404.html`)  
- **Ubicaciones:** un vector de objetos `LocationParser` para cada bloque `location { … }`  
- Al finalizar la validación de directivas, llama a **`setUpServer()`** para configurar el socket:
  - **`socket()`**  
  - **`setsockopt(SO_REUSEADDR)`**  
  - **`fill sockaddr_in`**  
  - **`bind()`**  
- Almacena internamente el descriptor de archivo de escucha resultante.

## Por qué importa
Todo lo necesario para manejar un servidor virtual. Incluyendo anulación de páginas de error y reglas de ruta se encapsula en esta clase. Realizar el bind del socket durante el parseo permite detectar conflictos de puertos o direcciones inválidas antes de iniciar el bucle de eventos.

# ReadConfig

## Responsabilidad
El cargador de configuración de más alto nivel. Dada una ruta en el sistema de archivos, se encarga de:

- **`readFile()`** (vía `ConfigFile`)  
- Eliminar comentarios (`#…`) y recortar espacios en blanco  
- Dividir el texto en N subcadenas distintas de `server { … }`  
- Por cada subcadena:
  - Crear una instancia nueva de **`ServerParser`**  
  - Tokenizarla y enviar cada directiva (`listen`, `host`, `root`, `error_page`, `location`, etc.) al parser  
  - Dejar que **`ServerParser`** y **`LocationParser`** validen cada valor  
- Si hay más de un servidor, comprobar combinaciones duplicadas de `(host, port, server_name)`  
- Finalmente, **`getServers()`** devuelve un `std::vector<ServerParser>` — cada uno ya enlazado a su socket de escucha o lanzando una excepción clara en caso de error.

## Por qué importa
Revosa la sintaxis de alto nivel de las validaciones por servidor y por ubicación. Una vez que llamas a **`createServerGroup()`**, sabes que obtendrás una lista limpia y validada de servidores, o bien se detendrá con un error informativo antes de arrancar el loop de eventos.

# Message

## Responsabilidad
Un envoltorio ligero para registro en estilo `printf`:

- **`logMessage()`**  
  Para mensajes de información normales.

- **`logError()`**  
  Para errores graves (se muestran en rojo).

## Por qué importa
Garantiza una salida de consola coherente en `main()`, `ServerManager` y todos los parsers, facilitando el seguimiento de la ejecución y la depuración de errores.

### 1. **Capa de Ejecución**

#### **Clases principales:**

- **`ServerManager`**

---

# ServerManager

## Responsabilidad
Convierte tu `std::vector<ServerParser>` en un conjunto de descriptores de archivo (FD-set) listo para usar con `select()`:

- **`setUpServers(...)`**  
  - Copia las configuraciones recibidas.  
  - Para cada `ServerParser`:
    - Si otro servidor ya ha enlazado el mismo `host`/`port`, reutiliza su FD.  
    - De lo contrario, llama a **`ServerParser::setUpServer()`**.  
    - Registra la relación FD → `ServerParser` en `_servers_map`.  
    - Registra en log: `"Server X bound to IP:port, fd=..."`.  

- **`initializeSockets()`**  
  - Llama a `listen()` y `fcntl(O_NONBLOCK)` en cada FD único.  
  - Ejecuta `FD_SET(fd, &_recv_fd_pool)` para cada descriptor.  
  - Mantiene actualizado `_biggest_fd` para la llamada a `select()`.

## Por qué importa
Actúa como puente entre la configuración y el tiempo de ejecución. Tras ejecutar estos métodos, tendrás sockets de escucha para cada `(host,port)` configurado. El siguiente paso es el bucle `select()` para `accept()`, lectura, escritura y manejo de timeouts.

# main()

## Responsabilidad
Ensambla todos los componentes y arranca el servidor:

- Selecciona la ruta del archivo de configuración (por defecto o `argv[1]`).  
- Llama a `ReadConfig::createServerGroup()`.  
- Obtiene el `std::vector<ServerParser>`.  
- Instancia `ServerManager`, ejecuta `setUpServers(...)` y `initializeSockets()`.  
- (Posteriormente) delega en `ServerManager::runServers()`.  
- Captura y reporta cualquier excepción surgida durante el parseo o el binding.

## Por qué importa
Actúa como punto de entrada y coordina la lectura de la configuración, la inicialización de sockets y el bucle de eventos. Además, garantiza que los errores en la fase de arranque se informen claramente antes de entrar al bucle principal.

## Interacciones entre Componentes

### `main()`
- Invoca **ReadConfig** para procesar el archivo de configuración.  
- Crea e inicializa **ServerManager** para preparar los sockets.  
- Utiliza **Message** para loguear eventos.

### `ReadConfig`
- Llama a **ConfigFile** para leer y validar el fichero.  
- Genera objetos **ServerParser** (uno por cada bloque `server { … }`).

### `ServerParser`
- Usa **LocationParser** para cada bloque `location { … }` en su configuración.  
- Vuelve a usar **ConfigFile** para validar rutas y permisos.  
- Loguea errores y mensajes con **Message**.

### `LocationParser`
- Guarda directivas específicas de ruta (`path`, `methods`, `root`, etc.).  
- Se comunica sólo con **Message** para logging de errores.

### `ConfigFile`
- Proporciona utilidades de I/O:
  - `readFile()`  
  - `getTypePath()`, `checkFile()`, `isFileExistAndReadable()`  
- Emplea **Message** para reportar fallos.

### `ServerManager`
- Recibe el vector de **ServerParser**, y para cada servidor:
  - Reusa o crea sockets con `socket()`/`bind()` (vía `ServerParser::setUpServer()`).  
  - Almacena la relación FD → `ServerParser` en `_servers_map`.  
  - Prepara los `fd_set` con `initializeSockets()`.  
- Usa **Message** para indicar progreso y errores.

```markdown

┌───────────┐
│   main    │
│ (WebServ) │
└─────┬─────┘
      │
      ▼
┌────────────┐               Invoca
│ ReadConfig │─────────────► createServerGroup(config)
└─────┬───────┘
      │
      │ 1) usa ConfigFile para leer y validar el archivo  
      │ 2) divide el texto en bloques “server {…}”  
      │ 3) para cada bloque crea un ServerParser  
      ▼
┌───────────────────┐
│  ServerParser     │◄───────────┐  
│  (config por srv) │            │  
└─────┬─────────────┘            │  
      │                          │  
      │ – valida host/puerto     │  
      │ – guarda server_name,    │  
      │   root, index, error_pages│  
      │ – delega cada bloque      │  
      │   `location {…}` a        │  
      ▼                          │  
┌───────────────────┐            │  
│ LocationParser    │            │  
│ (config ruta)     │            │  
└───────────────────┘            │  
                                 │  
      ▲                          │  
      │  utiliza                 │  
      │  ConfigFile para rutas   │  
      ▼                          │  
┌───────────────────┐            │  
│  ConfigFile       │            │  
│ (I/O y FS-checks) │            │  
└───────────────────┘            │  
                                 │  
      ▲                          │  
      │  All log/error messages  │  
      ▼                          │  
┌───────────────────┐            │  
│  Message          │────────────┘  
│ (logMessage /     │  
│  logError)        │  
└───────────────────┘  
                                  
      │  
      │ al terminar, devuelve  
      ▼  
┌───────────────────┐  
│ main recibe      │  
│ vector<ServerParser>  
└─────┬─────────────┘  
      │  
      ▼  
┌───────────────────┐  
│ ServerManager     │  
└─────┬─────────────┘  
      │  
      │ 1) setUpServers(...)  
      │    – recorre cada ServerParser  
      │    – reutiliza o llama a setUpServer()  
      │      (socket() + bind())  
      │    – guarda en map<fd,ServerParser>  
      │ 2) initializeSockets()  
      │    – listen(), non-blocking  
      │    – construye fd_sets para select()  
      ▼  
┌───────────────────┐  
│  (Next)           │  
│  accept/read/write│  
└───────────────────┘  

```text