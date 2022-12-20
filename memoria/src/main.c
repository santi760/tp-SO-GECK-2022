/*
 * main.c
 *
 *  Created on: Sep 27, 2022
 *      Author: utnso
 */

#include "main.h"

int main() {
	cargarConfiguracion();
	inicializar_memoria();
	t_config* config_ips = config_create("../ips.conf");
	char* ip = config_get_string_value(config_ips,"IP_MEMORIA");

	char* puerto= string_itoa(configuracion->PUERTO_ESCUCHA);
	//INICIO SERVIDORES
    memoriaServer = iniciar_servidor(logger,"memoria_sv",ip,puerto);// ACA IP PROPIA


	pthread_t hilo_kernel;
	pthread_t hilo_cpu;

	pthread_create(&hilo_cpu, NULL, (void*) escuchar_cpu, NULL);
	usleep(5000);
	pthread_create(&hilo_kernel, NULL, (void*) escuchar_kernel, NULL);


	pthread_join(hilo_kernel,NULL);
	pthread_join(hilo_cpu, NULL);

	return 0;

}

void escuchar_kernel() {
	while(kernel_escuchar("memoria_sv",memoriaServer));
}
void escuchar_cpu() {
	while(cpu_escuchar("memoria_sv",memoriaServer));
}
