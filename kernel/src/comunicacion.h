/*
 * comunicacion.h
 *
 *  Created on: Sep 12, 2022
 *      Author: utnso
 */

#ifndef SRC_COMUNICACION_H_
#define SRC_COMUNICACION_H_

#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include "socket.h"
#include "planificacion.h"

extern int cliente_socket;

extern uint16_t pid_nuevo;
//SERVIDOR
int server_escuchar(char* server_name, int server_socket);
//CLIENTE
int generar_conexiones(int* interrupt_fd, int* dispatch_fd, t_config_kernel* configuracion);
int generar_conexion_memoria(int* memoria_fd, t_config_kernel* configuracion);

#endif /* SRC_COMUNICACION_H_ */
