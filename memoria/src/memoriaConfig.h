/*
 * memoriaConfig.h
 *
 *  Created on: Sep 27, 2022
 *      Author: utnso
 */

#ifndef SRC_MEMORIACONFIG_H_
#define SRC_MEMORIACONFIG_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "commons/log.h"
#include "commons/config.h"

typedef struct {
	uint32_t PUERTO_ESCUCHA;
	uint32_t TAM_MEMORIA;
	uint32_t TAM_PAGINA;
	uint32_t ENTRADAS_POR_TABLA;
	uint32_t RETARDO_MEMORIA;
	char* ALGORITMO_REEMPLAZO;
	uint32_t MARCOS_POR_PROCESO;
	uint32_t RETARDO_SWAP;
	char* PATH_SWAP;
	uint32_t TAMANIO_SWAP;
} t_config_memoria;

extern t_config_memoria * configuracion;
extern t_config * fd_configuracion;
extern t_log * logger;

int cargarConfiguracion();

#endif /* SRC_MEMORIACONFIG_H_ */
