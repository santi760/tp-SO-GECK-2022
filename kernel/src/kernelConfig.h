/*
 * kernelConfig.h
 *
 *  Created on: Sep 6, 2022
 *      Author: utnso
 */

#ifndef SRC_KERNELCONFIG_H_
#define SRC_KERNELCONFIG_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "commons/log.h"
#include "commons/config.h"
#include "commons/collections/list.h"

typedef struct {
	char* IP_MEMORIA;
	uint32_t PUERTO_MEMORIA;
	char* IP_CPU;
	uint32_t PUERTO_CPU_DISPATCH;
    uint32_t PUERTO_CPU_INTERRUPT;
	uint32_t PUERTO_ESCUCHA;
	char* ALGORITMO_PLANIFICACION;
	uint32_t GRADO_MAX_MULTIPROGRAMACION;
	char** DISPOSITIVOS_IO;
	char** TIEMPOS_IO;
	uint32_t QUANTUM_RR;
} t_config_kernel;

extern t_config_kernel * configuracion;
extern t_config * fd_configuracion;
extern t_log * logger;

int cargarConfiguracion();

#endif /* SRC_KERNELCONFIG_H_ */
