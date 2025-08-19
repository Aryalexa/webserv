#!/usr/bin/env python3
import os
import cgi

print("Content-Type: text/html\n")

# Carpeta donde están las imágenes
IMG_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "../www/file"))

# Obtener el parámetro 'img' de la query string
form = cgi.FieldStorage()
img_name = form.getvalue("img", "")

# Validar el nombre del archivo (evitar path traversal)
safe_img = os.path.basename(img_name)
img_path = os.path.join(IMG_DIR, safe_img)

if not safe_img or not os.path.isfile(img_path):
    print("<h1>Imagen no encontrada</h1>")
else:
    # Generar el HTML dinámico
    print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Detalle de la imagen</title>
    <link rel="stylesheet" href="css/bootstrap.min.css">
    <link rel="stylesheet" href="fontawesome/css/all.min.css">
    <link rel="stylesheet" href="css/templatemo-style.css">
</head>
<body>
    <nav class="navbar navbar-expand-lg">
        <div class="container-fluid">
            <a class="navbar-brand" href="index.html">
                <i class="fas fa-image mr-2"></i>
                42 Madrid
            </a>
        </div>
    </nav>
    <div class="container-fluid tm-container-content tm-mt-60">
        <div class="row mb-4">
            <h2 class="col-12 tm-text-primary">Photo</h2>
        </div>
        <div class="row tm-mb-90">            
            <div class="col-xl-8 col-lg-7 col-md-6 col-sm-12">
                <img src="file/{safe_img}" alt="{safe_img}" class="img-fluid">
            </div>
            <div class="col-xl-4 col-lg-5 col-md-6 col-sm-12">
                <div class="tm-bg-gray tm-video-details">
                    <p class="mb-4">
                        Imagen: {safe_img}
                    </p>
                    <div class="text-center mb-5">
                        <a href="file/{safe_img}" class="btn btn-primary tm-btn-big" download>Download</a>
                    </div>
                </div>
            </div>
        </div>
    </div>
</body>
</html>
""")