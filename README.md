# Webserv_42: Servidor HTTP en C++

## Introducción al Proyecto

**Webserv_42** Mi parte está divida en dos etapas: 

- **1. Capa de configuración:** Cuando el servidor WebServ se inicia, abre un socket de red y comienza a escuchar en un puerto especificado (por ejemplo, el puerto 8001 como se muestra en la configuración). Esto es como una tienda abriendo su puerta y esperando a los clientes. El servidor está esencialmente diciendo "Estoy listo para aceptar conexiones en el puerto 8001." Si un cliente intenta conectarse, el servidor acepta la conexión; ahora se ha abierto una línea de comunicación entre el cliente y el servidor. (Bajo el capó, el componente Server Core de WebServ maneja la creación del socket y la aceptación de conexiones como parte de sus funciones de red, pero puedes pensar en ello simplemente como establecer una nueva "conversación" con el cliente).

- **2. Capa de ejecución:** Una vez conectado, el cliente (navegador) envía un mensaje de solicitud HTTP. Este llega como un flujo de bytes de texto. WebServ utiliza su clase de analizador HttpRequest para leer este flujo y entenderlo. La tarea del analizador es tomar los datos en bruto y descomponerlos en las partes significativas de una solicitud HTTP.

## Estructura del Proyecto - Parte Ana Paula

### 1. **Capa de Configuración**

Esta capa transforma tu archivo de configuración (`config/default.config`) en estructuras validadas listas para ser utilizadas.

#### **Clases principales:**

- **`ReadConfig`**
- **`ServerParser`**
- **`LocationParser`**
- **`ConfigFile`**

---

### **ReadConfig**

**Objetivo:**  
Leer, preprocesar y dividir la configuración general en servidores individuales.

**Funciones clave:**

- Lee archivo y remueve comentarios/espacios.
- Divide contenido en bloques individuales (`server {}`).
- Valida estructura general y delega la validación específica a `ServerParser`.

---

### **ServerParser**

**Objetivo:**  
Analizar, validar y almacenar configuraciones específicas de cada servidor.

**Atributos esenciales:**

| Atributo | Descripción | Ejemplo |
|----------|-------------|---------|
| `host` / `port` | Dirección IP y puerto | localhost:8002 |
| `server_name` | Nombre identificador del servidor | localhost |
| `root` | Directorio base de archivos estáticos | `/Users/.../WebServAP` |
| `index` | Archivo índice por defecto | `index.html` |
| `client_max_body_size` | Límite máximo del cuerpo de solicitud | `3MB` |
| `error_list` | Páginas personalizadas por código de error | `404 → /error_pages/404.html` |
| `locations` | Reglas específicas según rutas URL | `/` |
| `listen_fd` | Descriptor del socket para escuchar conexiones | fd generado por `socket()` |

---

### **LocationParser**

**Objetivo:**  
Almacenar reglas específicas para gestionar solicitudes según rutas URL.

**Atributos esenciales:**

- `path`: Ruta URL específica (ej.: `/`)
- `root`: Directorio específico para servir archivos
- `methods`: Métodos HTTP permitidos (`GET, POST, DELETE`)
- `index`: Archivo índice específico para la ruta

---

### **ConfigFile**

**Objetivo:**  
Validar y gestionar rutas y archivos en el sistema operativo.

**Funciones clave:**

- Verificar tipo de ruta (archivo/directorio).
- Comprobar existencia y permisos.
- Leer archivos completos en memoria.

---

## Integración de las Clases (Visualmente)

    Archivo de configuración
               │
               ▼
       ┌───────────────┐
       │  ReadConfig   │
       └───────┬───────┘
               │
               ▼
       ┌───────────────┐
       │ ServerParser  │
       └───┬───────────┘
           │
     ┌─────┴───────┐
     ▼             ▼
    LocationParser     ConfigFile

## Resumen

| Clase          | Tarea principal                           | Rol en runtime                                 |
| -------------- | ----------------------------------------- | ---------------------------------------------- |
| `ReadConfig`   | Leer, preprocesar y validar configuración | Asegurar inicio correcto del servidor          |
| `ServerParser` | Validar ajustes y abrir sockets           | Proveer configuración y socket para ejecución  |
| `Location`     | Almacenar reglas específicas por URL      | Definir comportamiento de rutas específicas    |
| `ConfigFile`   | Validar rutas y archivos                  | Simplificar validaciones del sistema operativo |
