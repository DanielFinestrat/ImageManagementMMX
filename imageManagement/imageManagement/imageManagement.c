#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <Windows.h>
#include "imagenBMP.h"

 
// Retorna (a - b) en segundos
double performancecounter_diff(LARGE_INTEGER *a, LARGE_INTEGER *b){
  LARGE_INTEGER freq;
  QueryPerformanceFrequency(&freq);
  return (double)(a->QuadPart - b->QuadPart) / (double)freq.QuadPart;
}

void fundirSSE(ImagenBMP *imgA, ImagenBMP *imgB, ImagenBMP *imgResultado, short fade) {
	unsigned char puntoA[8];
	unsigned char puntoB[8];
	unsigned char resultado[8];

	short factorFade[8] = { fade, fade, fade, fade, fade, fade, fade, fade };
	short maxFade[8] = { 255, 255, 255, 255, 255, 255, 255, 255 };
	short vectorDivision[8] = { 255, 255, 255, 255, 255, 255, 255, 255 };

	double puntoResultado;
	int i;

	for (i = 0; i < (int)imgA->tamanyo; i += 8) {
		puntoA[0] = imgA->datos[i];
		puntoA[1] = imgA->datos[i + 1];
		puntoA[2] = imgA->datos[i + 2];
		puntoA[3] = imgA->datos[i + 3];
		puntoA[4] = imgA->datos[i + 4];
		puntoA[5] = imgA->datos[i + 5];
		puntoA[6] = imgA->datos[i + 6];
		puntoA[7] = imgA->datos[i + 7];

		puntoB[0] = imgB->datos[i];
		puntoB[1] = imgB->datos[i + 1];
		puntoB[2] = imgB->datos[i + 2];
		puntoB[3] = imgB->datos[i + 3];
		puntoB[4] = imgB->datos[i + 4];
		puntoB[5] = imgB->datos[i + 5];
		puntoB[6] = imgB->datos[i + 6];
		puntoB[7] = imgB->datos[i + 7];

		_asm {
			movups xmm0, puntoA 
			movups xmm1, puntoB 
			movups xmm2, factorFade 
			movups xmm3, maxFade
			movups xmm4, vectorDivision
			xorps xmm5, xmm5 // Registro vacio para empaquetar/desempaquetar

			punpcklbw xmm0, xmm5
			punpcklbw xmm1, xmm5

			pmullw xmm0, xmm2 // xmm0 = puntoA * fade
			subps xmm3, xmm2 // xmm3 = 255 - fade
			pmullw xmm1, xmm3 // xmm1 = puntoB * (255 - fade)
			paddsw xmm0, xmm1 // xmm0 = (puntoA*fade) + (puntoB*(255-fade))
			psrlw xmm0, 8 // xmm0 = ((puntoA*fade) + (puntoB*(255-fade))) / 255

			packuswb xmm0, xmm5

			movlps puntoResultado, xmm0
		}

		memcpy(resultado, &puntoResultado, 8);
		imgResultado->datos[i] = resultado[0];
		imgResultado->datos[i + 1] = resultado[1];
		imgResultado->datos[i + 2] = resultado[2];
		imgResultado->datos[i + 3] = resultado[3];
		imgResultado->datos[i + 4] = resultado[4];
		imgResultado->datos[i + 5] = resultado[5];
		imgResultado->datos[i + 6] = resultado[6];
		imgResultado->datos[i + 7] = resultado[7];
	}
	_asm { emms }
}

void fundirMMX(ImagenBMP *imgA, ImagenBMP *imgB, ImagenBMP *imgResultado, short fade) {
	unsigned char puntoA[4];
	unsigned char puntoB[4];
	unsigned char resultado[4];

	short factorFade[4] = { fade, fade, fade, fade };
	short maxFade[4] = { 255, 255, 255, 255 };

	double puntoResultado;
	int i;

	for (i = 0; i < (int)imgA->tamanyo; i += 4) {
		puntoA[0] = imgA->datos[i];
		puntoA[1] = imgA->datos[i + 1];
		puntoA[2] = imgA->datos[i + 2];
		puntoA[3] = imgA->datos[i + 3];

		puntoB[0] = imgB->datos[i];
		puntoB[1] = imgB->datos[i + 1];
		puntoB[2] = imgB->datos[i + 2];
		puntoB[3] = imgB->datos[i + 3];

		_asm {
			pxor mm0, mm0 // Registro vacio para empaquetar/desempaquetar
			movq mm1, puntoA
			movq mm2, puntoB
			movq mm3, factorFade
			movq mm4, maxFade

			punpcklbw mm1, mm0 // Desempaquetar
			punpcklbw mm2, mm0 // Desempaquetar

			pmullw mm1, mm3 // mm1 = puntoA * fade
			psubusw mm4, mm3 // mm4 = 255 - fade
			pmullw mm2, mm4 // mm2 = puntoB * (fade - 255)
			paddsw mm1, mm2 // mm1 = (puntoA * fade) + (puntoB * (fade - 255))
			psrlw mm1, 8 // mm1 = ((puntoA * fade) + (puntoB * (fade - 255))) / 255

			packuswb mm1, mm0 // Empaquetar

			movq puntoResultado, mm1
		}

		memcpy(resultado, &puntoResultado, 4);
		imgResultado->datos[i] = resultado[0];
		imgResultado->datos[i + 1] = resultado[1];
		imgResultado->datos[i + 2] = resultado[2];
		imgResultado->datos[i + 3] = resultado[3];
	}
	_asm { emms }
}

void equalizar(ImagenBMP *img) {

    int p, vp, m = 1;
    int max = 0, min = 255;
        
    for(p = 0; p < img->tamanyo; m++ ) {                
        vp = img->datos[p];   
        if ( vp < min ) min = vp;	//Hallar minimo valor de gris
        else if ( vp > max ) max = vp;	//Hallar maximo valor de gris
        
        //Si hemos llegado al final de la fila, saltamos al siguiente byte de datos
        if ( m % img->ancho == 0 ) p += img->padding + 1;	//Los bits de padding son "de relleno"
        else p++;        
    }
    
    //Hacemos la ecualizacion
    //Aqui no tenemos en cuenta los bytes de padding ya que no importa el valor que contengan, asi que los modificaremos tambien
    for(p = 0; p < img->tamanyo; p++) img->datos[p] = 255 * (img->datos[p] - min) / (max - min);

}

void equalizarMMX(ImagenBMP *img) {

    int p, vp, m = 1, j;
    int max = 0, min = 255;
	int ctecorreccion;
	short vctecorreccion[4];	// 16 x [4] bits
	unsigned char vmin[8];		//  8 x [8] bits
	double *puntero;
        
    for(p = 0; p < img->tamanyo; m++ ) {                
        vp = img->datos[p];   
        if ( vp < min ) min = vp;
        else if ( vp > max ) max = vp;
        
        //Si hemos llegado al final de la fila saltamos al siguiente byte de datos
        if ( m % img->ancho == 0 ) p += img->padding + 1;
        else p++;        
    }
    
    //Hacemos la ecualizacion
    //Aqui no tenemos en cuenta los bytes de padding ya que no importa el valor que contengan, asi que los modificaremos tambien 

	ctecorreccion = 255 / (max - min);
	for (j = 0; j < 8; j++) vmin[j] = min;
	for (j = 0; j < 4; j++) vctecorreccion[j] = ctecorreccion;
	for (p = 0; p < img->tamanyo; p += 8) {
		puntero = (double*)&img->datos[p];
		
		_asm {
			mov esi, puntero[0]
			mov edi, puntero[0]
			movq mm1, [esi]
			movq mm0, vmin
			movq mm3, vctecorreccion
			psubw mm1, mm0
			movq mm2, mm1
			pxor mm0, mm0
			punpcklbw mm1, mm0
			punpckhbw mm2, mm0
			pmullw mm1, mm3
			pmullw mm2, mm3
			packuswb mm1, mm2
			movq[edi], mm1
		}

	}
	_asm { emms }
}

//nombrePrograma.exe nombreFoto1.bmp nombreFoto2.bmp fade(0 < x < 255)
int hacerFundido(int argc, char **argv) {

	if (atoi(argv[3]) < 0 || atoi(argv[3]) > 255) {
		fprintf(stderr, "Uso incorrecto de los parametros.\n");
		exit(1);
	}
	else {

		struct stat buf;
		char *dSalida;
		ImagenBMP aMMX, bMMX, salidaMMX, aSSE, bSSE, salidaSSE, aAUX, bAUX, salidaAUX;
		short fade;
		LARGE_INTEGER t_ini, t_fin;
		double secs;
		int i = 0;

		fade = atoi(argv[3]);
		dSalida = (char*)calloc(8 + strlen(argv[1]), 1);
		sprintf(dSalida, "salida_%s", argv[1]);

		if (stat(argv[1], &buf) == 0 && stat(argv[2], &buf) == 0) {
			fprintf(stderr, "Procesando imagenes: %s %s...\n", argv[1], argv[2]);

			leerBMP(&aMMX, argv[1]);
			leerBMP(&bMMX, argv[2]);
			salidaMMX = aMMX;

			leerBMP(&aSSE, argv[1]);
			leerBMP(&bSSE, argv[2]);
			salidaSSE = aSSE;

			leerBMP(&aAUX, argv[1]);
			leerBMP(&bAUX, argv[2]);
			salidaAUX = aAUX;


			QueryPerformanceCounter(&t_ini);
			for (i = 0; i < 100; i++) fundirMMX(&aAUX, &bAUX, &salidaAUX, fade);
			QueryPerformanceCounter(&t_fin);
			fundirMMX(&aMMX, &bMMX, &salidaMMX, fade);
			secs = performancecounter_diff(&t_fin, &t_ini);
			fprintf(stderr, "MMX terminado en %f\n", secs);

			QueryPerformanceCounter(&t_ini);
			for (i = 0; i < 100; i++) fundirSSE(&aAUX, &bAUX, &salidaAUX, fade);
			QueryPerformanceCounter(&t_fin);
			fundirSSE(&aSSE, &bSSE, &salidaSSE, fade);
			secs = performancecounter_diff(&t_fin, &t_ini);
			fprintf(stderr, "SSE terminado en %f\n", secs);

			fprintf(stderr, "Proceso terminado, imagenes guardadas con un fade de %s\n", argv[3]);
			sprintf(dSalida, "salida_fadeMMX.bmp");
			escribirBMP(&salidaMMX, dSalida);
			sprintf(dSalida, "salida_fadeSSE.bmp");
			escribirBMP(&salidaSSE, dSalida);

			exit(0);
		}
		else {
			fprintf(stderr, "No existen los ficheros o directorios indicados\n");
			exit(1);
		}
	}

	return 0;
}

int hacerEqualizado(int argc, char **argv) {

    int i=0;
    struct stat buf;  
	char *dSalida;
	ImagenBMP img;
    
	LARGE_INTEGER t_ini, t_fin;	//Para contar tiempo de proceso
    double secs;

    dSalida = (unsigned char*)calloc( 8 + strlen(argv[1]), 1);
    sprintf(dSalida, "salida_%s", argv[1]);    
    
    if (stat(argv[1], &buf) == 0) {
        
            fprintf(stderr, "Procesando imagen: %s ... ", argv[1]);
            leerBMP(&img, argv[1]);            
            
			QueryPerformanceCounter(&t_ini);
			for (i=0; i<100; i++) equalizar(&img); //100 pruebas para poder obtener tiempos medibles
			QueryPerformanceCounter(&t_fin);
			secs = performancecounter_diff(&t_fin, &t_ini);
			                    
            fprintf(stderr, "FIN. CORRECTO. TIEMPO = %f\n",secs);
            escribirBMP(&img, dSalida);
            
			QueryPerformanceCounter(&t_ini);
			for (i=0; i<100; i++) equalizarMMX(&img); //100 pruebas para poder obtener tiempos medibles
			QueryPerformanceCounter(&t_fin);
			secs = performancecounter_diff(&t_fin, &t_ini);
			                    
            fprintf(stderr, "FIN MMX. CORRECTO. TIEMPO = %f\n",secs);
            sprintf(dSalida, "salidammx_%s", argv[1]);    
			escribirBMP(&img, dSalida);
            exit(0);
    }
	
	else {
        fprintf(stderr, "No existe el fichero o directorio indicado\n");
        exit(1);        
    }
    
        
    return 0;
}

int main(int argc, char **argv) {

	if (argc == 2) { hacerEqualizado(argc, argv); }
	else if (argc == 4) { hacerFundido(argc, argv); }
	else { fprintf(stderr, "Uso incorrecto de los parametros.\n"); exit(1); }
	
	return 0;
}