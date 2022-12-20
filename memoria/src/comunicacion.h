/*
 * comunicacion.h
 *
 *  Created on: Sep 27, 2022
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
#include "protocolo.h"
#include "socket.h"
#include "memoriaConfig.h"
#include "memoria_utils.h"
#include <semaphore.h>


//KERNEL
int kernel_escuchar(char* server_name, int server_socket);
//CPU
int cpu_escuchar(char* server_name, int server_socket);

#endif /* SRC_COMUNICACION_H_ */
