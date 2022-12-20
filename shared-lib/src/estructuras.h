/*
 * estructuras.h
 *
 *  Created on: 12 sept 2022
 *      Author: utnso
 */

/*
  PREGUNTAR POR LONGITUD DE CHAR DE la estructura de instruccion????
 */
typedef struct instruccion{
	char comando[10];
	char parametro[10];
	char parametro2[5];
} INSTRUCCION;

typedef struct segmento{
	uint32_t id_segmento;
	uint32_t tamSegmento;
}SEGMENTO;


