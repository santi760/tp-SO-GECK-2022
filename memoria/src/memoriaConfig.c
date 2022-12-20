/*
 * memoriaConfig.c
 *
 *  Created on: Sep 27, 2022
 *      Author: utnso
 */

#include "memoriaConfig.h"

t_config_memoria * configuracion;
t_config * fd_configuracion;
t_log * logger;

int configValida(t_config* fd_configuracion) {
	return (config_has_property(fd_configuracion, "PUERTO_ESCUCHA")
		&& config_has_property(fd_configuracion, "TAM_MEMORIA")
		&& config_has_property(fd_configuracion, "TAM_PAGINA")
		&& config_has_property(fd_configuracion, "ENTRADAS_POR_TABLA")
		&& config_has_property(fd_configuracion, "RETARDO_MEMORIA")
		&& config_has_property(fd_configuracion, "ALGORITMO_REEMPLAZO")
		&& config_has_property(fd_configuracion, "MARCOS_POR_PROCESO")
		&& config_has_property(fd_configuracion, "RETARDO_SWAP")
		&& config_has_property(fd_configuracion, "PATH_SWAP")
		&& config_has_property(fd_configuracion, "TAMANIO_SWAP"));
}

int cargarConfiguracion() {
	logger = log_create("LogMemoria.log", "Memoria", 1, LOG_LEVEL_INFO);
	configuracion = malloc(sizeof(t_config_memoria));

	fd_configuracion = config_create("cpu.conf");
	if (fd_configuracion == NULL) {
		fd_configuracion = config_create("memoria.conf");
	}

	if (fd_configuracion == NULL || !configValida(fd_configuracion)) {
		log_error(logger,"Archivo de configuración inválido.");
		return -1;
	}

	configuracion->PUERTO_ESCUCHA = config_get_int_value(fd_configuracion, "PUERTO_ESCUCHA");
	configuracion->TAM_MEMORIA = config_get_int_value(fd_configuracion, "TAM_MEMORIA");
	configuracion->TAM_PAGINA = config_get_int_value(fd_configuracion, "TAM_PAGINA");
	configuracion->ENTRADAS_POR_TABLA = config_get_int_value(fd_configuracion, "ENTRADAS_POR_TABLA");
	configuracion->RETARDO_MEMORIA = config_get_int_value(fd_configuracion, "RETARDO_MEMORIA");
	configuracion->ALGORITMO_REEMPLAZO = config_get_string_value(fd_configuracion, "ALGORITMO_REEMPLAZO");
	configuracion->MARCOS_POR_PROCESO = config_get_int_value(fd_configuracion, "MARCOS_POR_PROCESO");
	configuracion->RETARDO_SWAP = config_get_int_value(fd_configuracion, "RETARDO_SWAP");
	configuracion->PATH_SWAP = config_get_string_value(fd_configuracion, "PATH_SWAP");
	configuracion->TAMANIO_SWAP = config_get_int_value(fd_configuracion, "TAMANIO_SWAP");


	log_info(logger,
		"\PUERTO_ESCUCHA: %d\n"
		"TAM_MEMORIA: %d\n"
		"TAM_PAGINA: %d\n"
		"ENTRADAS_POR_TABLA: %d\n"
		"RETARDO_MEMORIA: %d\n"
		"ALGORITMO_REEMPLAZO: %s\n"
		"MARCOS_POR_PROCESO: %d\n"
	    "RETARDO_SWAP: %d\n"
		"PATH_SWAP: %s\n"
		"TAMANIO_SWAP: %d/n",
		configuracion->PUERTO_ESCUCHA,
		configuracion->TAM_MEMORIA,
		configuracion->TAM_PAGINA,
		configuracion->ENTRADAS_POR_TABLA,
		configuracion->RETARDO_MEMORIA,
		configuracion->ALGORITMO_REEMPLAZO,
		configuracion->MARCOS_POR_PROCESO,
		configuracion->RETARDO_SWAP,
		configuracion->PATH_SWAP,
		configuracion->TAMANIO_SWAP
		);
	return 0;
}

void limpiarConfiguracion() {
	free(configuracion);
	config_destroy(fd_configuracion);
	log_destroy(logger);

}

