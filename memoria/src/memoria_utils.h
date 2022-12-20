/*
 * memoria_utils.h
 *
 *  Created on: Nov 21, 2022
 *      Author: utnso
 */

#ifndef SRC_MEMORIA_UTILS_H_
#define SRC_MEMORIA_UTILS_H_

#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<stdbool.h>
#include<pthread.h>
#include "swap.h"
#include <stdint.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/string.h>
#include<readline/readline.h>
#include<semaphore.h>
#include<commons/collections/queue.h>
#include<commons/collections/list.h>
#include<commons/collections/dictionary.h>
//#include <sockets.h>
//#include <protocol.h>
//#include <structures.h>
//#include <commons/bitarray.h>

extern t_list* tabla_de_paginas;

extern t_list* lista_tablas_de_procesos;

extern uint16_t pid_global;

typedef struct {
	int32_t nro_marco;
	unsigned char modificado;
	unsigned char presencia;
	unsigned char uso;
	uint16_t pid;
} fila_de_pagina;

typedef struct {
	uint16_t pid;
	t_list* marcos_en_memoria;
	uint16_t puntero;
} estructura_clock;

typedef struct {
	int32_t nro_marco_en_memoria;
	fila_de_pagina* pagina;
	uint32_t nro_marco_en_swap;
	uint16_t nro_segmento;
} fila_estructura_clock;


void inicializar_memoria();
void apagar_memoria();

/***************************** HILO KERNEL *****************************/

t_list* crear_tabla(uint16_t pid);
void inicializar_tabla_de_paginas(fila_de_pagina* pagina);

void suspender_proceso(uint16_t pid, uint32_t tabla_paginas);

void eliminar_estructuras(uint32_t tabla_paginas, uint16_t pid);

/***************************** HILO CPU *****************************/

uint32_t obtener_nro_marco_memoria(uint32_t num_segmento, uint32_t num_pagina, uint16_t pid_actual);
uint32_t tratar_page_fault(uint32_t num_segmento, uint32_t num_pagina, uint16_t pid_actual);

// FUNCIONES ALGORITMOS CLOCK Y CLOCK MODIFICADO
uint32_t usar_algoritmo(int pid);
uint32_t clock_simple(int pid);
uint32_t clock_modificado(int pid);
void reemplazo_por_clock(uint32_t nro_marco_en_swap, fila_de_pagina* entrada_2do_nivel, int pid);

// READ Y WRITE
uint32_t read_en_memoria(uint32_t nro_marco, uint32_t desplazamiento, uint16_t pid_actual);
void write_en_memoria(uint32_t nro_marco, uint32_t desplazamiento, uint32_t dato, uint16_t pid_actual);

// FUNCIONES GENERALES
void* obtener_marco(uint32_t nro_marco);
uint32_t marcos_en_memoria(uint16_t pid_actual);
void escribir_marco_en_memoria(uint32_t nro_marco, void* marco);
int buscar_marco_libre();
uint32_t calcular_cant_marcos(uint16_t tamanio);
int marcos_actuales(int entrada_1er_nivel, int entrada_2do_nivel);

/***************************** FUNCIONES ESTRUCTURA CLOCK *****************************/
void crear_estructura_clock(uint16_t pid);
uint16_t avanzar_puntero(uint16_t puntero_clock);
void agregar_pagina_a_estructura_clock(int32_t nro_marco, fila_de_pagina* pagina, uint32_t nro_marco_en_swap, uint16_t pid_actual);
fila_de_pagina* obtener_pagina(uint16_t pid_actual, int32_t nro_marco);

#endif /* SRC_MEMORIA_UTILS_H_ */
