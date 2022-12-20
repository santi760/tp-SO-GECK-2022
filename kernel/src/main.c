/*
 * main.c
 *
 *  Created on: Sep 6, 2022
 *      Author: utnso
 */

#include "main.h"

int interrupt_fd, dispatch_fd, memoria_fd;

int main(){

	cargarConfiguracion();
	t_config* config_ips = config_create("../ips.conf");
	char* ip = config_get_string_value(config_ips,"IP_KERNEL");
	inicializarPlanificacion();
	char* puerto = string_itoa(configuracion->PUERTO_ESCUCHA);



	//CLIENTE
	//CPU
	generar_conexiones(&interrupt_fd, &dispatch_fd, configuracion);
	//MEMORIA
	generar_conexion_memoria(&memoria_fd, configuracion);

	//INICIO SERVIDOR
		int kernelServer= iniciar_servidor(logger,"kernel server",ip,puerto);//ACA IP PROPIA
		free(puerto);
		while (server_escuchar("KERNEL_SV", kernelServer));

	limpiarConfiguracion();
	return 0;
}
