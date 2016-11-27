#ifndef __imagenBMP__
#define __imagenBMP__

#define TAM_CABECERA 1078

typedef struct {
   char cabecera[TAM_CABECERA];
   unsigned int offsetDatos;
   unsigned int ancho;
   unsigned int alto;
   unsigned int tamanyo;
   unsigned int padding;
   unsigned char *datos;
} ImagenBMP;


void leerBMP(ImagenBMP *img, char *archivo);
void escribirBMP(ImagenBMP *img, char *archivo);


#endif
