/*
 * cpuConfig.h
 *
 *  Created on: Sep 27, 2022
 *      Author: utnso
 */

#ifndef SRC_CPUCONFIG_H_
#define SRC_CPUCONFIG_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "commons/log.h"
#include "commons/config.h"

typedef struct {
	uint32_t ENTRADAS_TLB;
	char* REEMPLAZO_TLB;
	uint32_t RETARDO_INSTRUCCION;
	char* IP_MEMORIA;
	uint32_t PUERTO_MEMORIA;
	uint32_t PUERTO_ESCUCHA_DISPATCH;
	uint32_t PUERTO_ESCUCHA_INTERRUPT;
} t_config_cpu;

extern t_config_cpu * configuracion;
extern t_config * fd_configuracion;
extern t_log * logger;

int cargarConfiguracion();

#endif /* SRC_CPUCONFIG_H_ */
