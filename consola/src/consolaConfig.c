/*
 * consolaConfig.c
 *
 *  Created on: Sep 6, 2022
 *      Author: utnso
 */

#include "consolaConfig.h"

t_config_consola * configuracion;
t_config * fd_configuracion;
t_log * logger;

int configValida(t_config* fd_configuracion) {
	return (config_has_property(fd_configuracion, "IP_KERNEL")
		&& config_has_property(fd_configuracion, "PUERTO_KERNEL")
		&& config_has_property(fd_configuracion, "SEGMENTOS")
		&& config_has_property(fd_configuracion, "TIEMPO_PANTALLA"));
}

int cargarConfiguracion(char* path) {
	int i=0;
	logger = log_create("LogConsola.log", "Consola", 1, LOG_LEVEL_INFO);
	configuracion = malloc(sizeof(t_config_consola));

	fd_configuracion = config_create(path);
	if (fd_configuracion == NULL) {
		fd_configuracion = config_create(path);
	}

	if (fd_configuracion == NULL || !configValida(fd_configuracion)) {
		log_error(logger,"Archivo de configuración inválido.");
		return -1;
	}

	configuracion->IP_KERNEL = config_get_string_value(fd_configuracion, "IP_KERNEL");
	configuracion->PUERTO_KERNEL = config_get_int_value(fd_configuracion, "PUERTO_KERNEL");
	configuracion->SEGMENTOS = config_get_array_value(fd_configuracion, "SEGMENTOS");
	configuracion->TIEMPO_PANTALLA = config_get_int_value(fd_configuracion, "TIEMPO_PANTALLA");


	log_info(logger,
		"\nIP_KERNEL: %s\n"
		"PUERTO_KERNEL: %d",
		configuracion->IP_KERNEL,
		configuracion->PUERTO_KERNEL); /*DESPUES HAY QUE ADAPTARLO PARA LOGGEAR LOS SEGMENTOS*/

	while(configuracion->SEGMENTOS[i]!=NULL){
		log_info(logger,"SEGMENTO: %s",configuracion->SEGMENTOS[i]);
		i++;
	}
	return 0;
}

void limpiarConfiguracion() {
	free(configuracion);
	config_destroy(fd_configuracion);
	log_destroy(logger);

}
