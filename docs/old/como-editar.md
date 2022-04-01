# Cómo editar bitácora

## General
Esta documentación está almacenada en el directorio `/docs` del repositorio [github.com/niclabs/water-monitoring](https://github.com/niclabs/water-monitoring) y utiliza [docsify](https://docsify.js.org/#/?id=docsify) para generar la versión web. Cualquier cambio de los archivos del repositorio actualizarán automáticamente este sitio.

### Resumen de docsify
En el directorio raíz de la documentación (`/docs`) hay un archivo `README.md` y otros directorios. El archivo `README.md` de cada directorio es el archivo principal que será cargado por defecto al acceder a una nueva ubicación; por ejemplo, este texto está en la ruta `/docs/README.md`. Si desea modificar la página principal de [Grupo Sensores](1-sensores/README.md), basta editar el archivo `/docs/1-sensores/README.md`. Si desea modificar otra página de dicho directorio, como [Homologación](1-sensores/homologacion.md), basta editar `/docs/1-sensores/homologacion.md`.

### Sidebar
En el directorio raíz también hay un archivo llamado `_sidebar.md`. Si desea agregar una nueva página a la barra lateral, sólo debe editar `/docs/_sidebar.md` y agregar el hipervínculo a la lista Markdown. Si agrega páginas de un subdirectorio, procure mantener la jerarquía de la lista indentando con espacios.
