/*
 * protocolo.h
 *
 *  Created on: Sep 6, 2022
 *      Author: utnso
 */

#ifndef SRC_PROTOCOLO_H_
#define SRC_PROTOCOLO_H_

#include <inttypes.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include "estructuras.h"
#include "pcb.h"


typedef enum {
    DEBUG = 69,
	SET,
	ADD,
	MOV_IN,
	MOV_OUT,
	IO,
	EXIT,
	DISPATCH,
	INTERRUPT,
	PAGEFAULT,
	CONTINUE,
	BLOCKED,
	PANTALLA,
	TECLADO,
	SOLICITUD_NRO_MARCO,
	SIGSEGV,
	ESCRITURA_OK,
	ELIMINAR_ESTRUCTURAS,
	CREAR_TABLA,
	INICIALIZAR
	}op_code;

typedef struct{
	uint32_t elementosLista;
	t_list* listaInstrucciones;
    uint32_t cantSegmentos;
    t_list* listaTamSegmentos;
} t_instrucciones;

typedef struct {
    uint32_t size; // Tamaño del payload
    void* stream; // Payload
} t_buffer;

bool send_debug(int fd);

//INSTRUCCIONES CONSOLA-KERNEL
void* serializar_instrucciones_tam(uint32_t size, t_list* lista, char** segmento);
t_instrucciones* deserializar_instrucciones(t_buffer* buffer);
void enviar_instrucciones(int socket_fd, t_list* lista, char** segmentos);
t_instrucciones* recibir_instrucciones(int socket_fd);
uint32_t calcular_instrucciones_buffer_size(t_list* lista, char** segmentos);

//ENVIO PROCESO KERNEL CPU
bool send_proceso(int fd, PCB_t *proceso,op_code codigo);
static void* serializar_proceso(size_t* size, PCB_t *proceso,op_code codigo);
bool recv_proceso(int fd, PCB_t* proceso);
static void deserializar_proceso(void* stream, PCB_t* proceso);

#endif /* SRC_PROTOCOLO_H_ */
