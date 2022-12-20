/*
 * kernelConfig.c
 *
 *  Created on: Sep 6, 2022
 *      Author: utnso
 */

#include "kernelConfig.h"

t_config_kernel * configuracion;
t_config * fd_configuracion;
t_log * logger;

int configValida(t_config* fd_configuracion) {
	return (config_has_property(fd_configuracion, "IP_MEMORIA")
		&& config_has_property(fd_configuracion, "PUERTO_MEMORIA")
		&& config_has_property(fd_configuracion, "IP_CPU")
		&& config_has_property(fd_configuracion, "PUERTO_CPU_DISPATCH")
		&& config_has_property(fd_configuracion, "PUERTO_CPU_INTERRUPT")
		&& config_has_property(fd_configuracion, "PUERTO_ESCUCHA")
		&& config_has_property(fd_configuracion, "ALGORITMO_PLANIFICACION")
		&& config_has_property(fd_configuracion, "GRADO_MAX_MULTIPROGRAMACION")
		&& config_has_property(fd_configuracion, "DISPOSITIVOS_IO")
		&& config_has_property(fd_configuracion, "TIEMPOS_IO")
		&& config_has_property(fd_configuracion, "QUANTUM_RR"));
}

int cargarConfiguracion() {
	logger = log_create("LogKernel.log", "Kernel", 1, LOG_LEVEL_INFO);
	configuracion = malloc(sizeof(t_config_kernel));

	fd_configuracion = config_create("kernel.conf");
	if (fd_configuracion == NULL) {
		fd_configuracion = config_create("kernel.conf");
	}

	if (fd_configuracion == NULL || !configValida(fd_configuracion)) {
		log_error(logger,"Archivo de configuración inválido.");
		return -1;
	}

	configuracion->IP_MEMORIA = config_get_string_value(fd_configuracion, "IP_MEMORIA");
	configuracion->PUERTO_MEMORIA = config_get_int_value(fd_configuracion, "PUERTO_MEMORIA");
	configuracion->IP_CPU = config_get_string_value(fd_configuracion, "IP_CPU");
	configuracion->PUERTO_CPU_DISPATCH = config_get_int_value(fd_configuracion, "PUERTO_CPU_DISPATCH");
	configuracion->PUERTO_CPU_INTERRUPT = config_get_int_value(fd_configuracion, "PUERTO_CPU_INTERRUPT");
	configuracion->PUERTO_ESCUCHA = config_get_int_value(fd_configuracion, "PUERTO_ESCUCHA");
	configuracion->ALGORITMO_PLANIFICACION = config_get_string_value(fd_configuracion, "ALGORITMO_PLANIFICACION");
	configuracion->GRADO_MAX_MULTIPROGRAMACION = config_get_int_value(fd_configuracion, "GRADO_MAX_MULTIPROGRAMACION");
	configuracion->DISPOSITIVOS_IO = config_get_array_value(fd_configuracion, "DISPOSITIVOS_IO");
	configuracion->TIEMPOS_IO = config_get_array_value(fd_configuracion, "TIEMPOS_IO");
	configuracion->QUANTUM_RR = config_get_int_value(fd_configuracion, "QUANTUM_RR");




	log_info(logger,
		"\nIP_MEMORIA: %s\n"
		"PUERTO_MEMORIA: %d\n"
		"IP_CPU: %s\n"
		"PUERTO_CPU_DISPATCH: %d\n"
		"PUERTO_CPU_INTERRUPT: %d\n"
	    "PUERTO_ESCUCHA: %d\n"
		"ALGORITMO_PLANIFICACION: %s\n"
		"GRADO_MAX_MULTIPROGRAMACION: %d\n"
		//"DISPOSITIVOS_IO: %d\n"
		//"TIEMPOS_IO: %d\n"
		"QUANTUM_RR: %d\n",
		configuracion->IP_MEMORIA,
		configuracion->PUERTO_MEMORIA,
		configuracion->IP_CPU,
		configuracion->PUERTO_CPU_DISPATCH,
		configuracion->PUERTO_CPU_INTERRUPT,
		configuracion->PUERTO_ESCUCHA,
		configuracion->ALGORITMO_PLANIFICACION,
		configuracion->GRADO_MAX_MULTIPROGRAMACION,
		//configuracion->DISPOSITIVOS_IO,
		//configuracion->TIEMPOS_IO), HAY QUE VER COMO MOSTRARLOS PORQUE SON LISTAS
	    configuracion->QUANTUM_RR);
	return 0;
}

void limpiarConfiguracion() {
	free(configuracion);
	config_destroy(fd_configuracion);
	log_destroy(logger);

}
