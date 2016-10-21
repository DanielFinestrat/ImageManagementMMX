#include <stdio.h>
#include <stdlib.h>
#include "imagenBMP.h"


void leerBMP(ImagenBMP *img, char *archivo) {

	int n;
    FILE *f = fopen(archivo, "rb");
    if (!f) perror(archivo), exit(1);
  
    // Leo la cebecera del archivo
    n = fread(img->cabecera, 1, TAM_CABECERA, f);    
    if (n != TAM_CABECERA) fprintf(stderr, "Entrada muy pequenya (%d bytes)\n", n), exit(1);
    if (img->cabecera[0]!='B' || img->cabecera[1]!='M') fprintf(stderr, "El archivo procesado no tiene formato BMP\n"), exit(1);
    
    //Obtengo el campo bfSize quitandole los bytes de la ImagenBMP para obtener los bytes de datos de la imagen
    img->tamanyo = *(int*)(img->cabecera + 2) - TAM_CABECERA;    
    if (img->tamanyo <= 0 || img->tamanyo > 0x9000000) fprintf(stderr, "Imagen demasiado grande: %d bytes\n", img->tamanyo), exit(1);
    
    //Obtengo bfOffBits: offset desde el principio del archivo hasta el inicio del bloque de datos
    img->offsetDatos = *(int*)(img->cabecera + 10);
    
    if (img->offsetDatos!=TAM_CABECERA || *(short*)(img->cabecera + 28)!=8)
        fprintf(stderr, "FORMATO: %d\n", *(short*)(img->cabecera + 28) ),
        fprintf(stderr, "El formato de la imagen BMP no es correcto, se esperaba BMP-grayscale\n"), exit(1);
        
    if (*(int*)(img->cabecera + 30)!=0) fprintf(stderr, "Compresion no soportada\n"), exit(1);
        
    img->ancho = *(int *)(img->cabecera + 18);
    img->alto = *(int *)(img->cabecera + 22);

    //Calculo el relleno que queda al final de cada fila
    if (img->ancho > 8 ) img->padding = img->ancho % 8;
    else img->padding = 8 - img->ancho;

    //Leer Imagen
    img->datos = (unsigned char*)calloc( img->tamanyo, 1);
  
    if ( (n = fread(img->datos, 1, img->tamanyo + 1, f)) != img->tamanyo ) fprintf(stderr,"Tamanyo archivo incorrecto: leidos %d bytes el lugar de %d\n", n, img->tamanyo), exit(1);
        
    fclose(f);
}


void escribirBMP(ImagenBMP *img, char *archivo) {

    int n;

    // Escribir salida
    FILE *f = fopen(archivo, "wb");
    if (!f) fprintf(stderr, "El archivo %s no se ha podido crear\n", archivo), exit(1);
    n = fwrite(img->cabecera, 1, TAM_CABECERA, f);
    n += fwrite(img->datos, 1, img->tamanyo, f);
    if (n != TAM_CABECERA + img->tamanyo) fprintf(stderr, "Escritos %d de %d bytes\n", n, img->tamanyo + TAM_CABECERA);

    fclose(f);
}
