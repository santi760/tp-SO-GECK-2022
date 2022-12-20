/*
 * comunicacion.h
 *
 *  Created on: Sep 6, 2022
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
#include "protocolo.h"
#include "socket.h"
#include "consolaConfig.h"

int generar_conexion(int* kernel_fd, t_config_consola* configuracion);

#endif /* SRC_COMUNICACION_H_ */
