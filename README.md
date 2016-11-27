# IluminarFotoBN
Iluminador y fusionador de fotos en escala de grises. Las fotos tiene que estar en formato BMP de 8 bits (256 tonos de gris).
El programa usará dos algoritmos diferentes para iluminar la misma imagen, uno de alto nivel y otro de bajo nivel.
También usará dos algoritmos de bajo nivel para fundir la imagen, uno con isntrucciones MMX y otro mezclando MMX con SSE.
Una vez esté hecha la operación con los dos métodos, el programa mostrará una comparativa de los tiempos que ha tardado en realizarse 100 iteraciones de cada operación. 

Para ejecutar el programa ir desde la consola a el directorio Release y ejecutar el iluminador pasandole como argumento el nombre del fichero de prueba de la siguiente manera:

```imageManagement.exe cara.bmp```

ó

```imageManagement.exe barbara.bmp lena.bmp 128```

Siendo la primera sintaxis para iluminar la foto cara.bmp y la segunda para fundir al 50% (128) las dos imagenes (barbara_gray.bmp y lena_gray.bmp). Podemos variar el nombre de las imágenes y el nivel de fundido de las mismas.

Este proyecto fue una práctica de la asignatura Dispositivos e Infraestructuras de Sistemas Multimedia, del tercer año de Ingeniería Multimedia.

¡Compilar en x86!
