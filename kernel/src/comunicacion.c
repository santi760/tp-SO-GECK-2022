/*
 * comunicacion.c
 *
 *  Created on: Sep 12, 2022
 *      Author: utnso
 */

#include "comunicacion.h"
uint16_t pid_nuevo=0;

typedef struct {
    int fd;
    char* server_name;
} t_procesar_conexion_args;
pthread_mutex_t pid_xd = PTHREAD_MUTEX_INITIALIZER;
int cliente_socket;

static void procesar_conexion(void* void_args) {
	t_procesar_conexion_args* args = (t_procesar_conexion_args*) void_args;
	int cliente_socket = args->fd;
	char* server_name = args->server_name;
	free(args);

	t_instrucciones* mensaje=malloc(sizeof(t_instrucciones));
	mensaje=recibir_instrucciones(cliente_socket);
	//printf("Segmento % \n", &mensaje->segmentos[0]);

	/*t_list* segmentos=malloc(sizeof(TABLA_SEGMENTO));
	int c=0;
	while(c<list_size(mensaje->listaTamSegmentos)){
		TABLA_SEGMENTO* aux=malloc(sizeof(TABLA_SEGMENTO));
		aux->tamSegmento=list_get(mensaje->listaTamSegmentos,c);//habria que probar esto
		list_add(segmentos,aux);
		c++;
	}*/
    PCB_t* proceso = malloc(sizeof(PCB_t));
	proceso = pcb_create();



	//TODO pasarle los valores de inicializacion al PCB
	uint32_t registros[4];
	registros[0]=0;
	registros[1]=0;
	registros[2]=0;
	registros[3]=0;

	pthread_mutex_lock(&pid_xd);
	pcb_set(proceso, pid_nuevo, mensaje->listaInstrucciones,      0,  registros,  mensaje->listaTamSegmentos,cliente_socket);
	       //( pcb,       pid,  instrucciones,  pc,  registro_cpu,  tabla_segmentos);
	pid_nuevo++;
	pthread_mutex_unlock(&pid_xd);



	list_destroy(mensaje->listaInstrucciones);
	list_destroy(mensaje->listaTamSegmentos);
	free(mensaje);

	pthread_mutex_lock(&mx_cola_new);
    queue_push(cola_new,proceso);
    pthread_mutex_unlock(&mx_cola_new);
    //pthread_mutex_lock(&mx_log);
    log_info(logger,"Se crea el proceso %d en NEW", proceso->pid);
    //pthread_mutex_unlock(&mx_log);
    sem_wait(&s_multiprogramacion_actual);
    pthread_mutex_lock(&mx_cola_new);
	proceso=queue_pop(cola_new);
    pthread_mutex_unlock(&mx_cola_new);
    solicitar_tabla_de_segmentos(proceso);

    pthread_mutex_lock(&mx_cola_ready);
    queue_push(cola_ready,proceso);
    pthread_mutex_unlock(&mx_cola_ready);
    //pthread_mutex_lock(&mx_log);
    log_info(logger,"“PID: %d - Estado Anterior: NEW - Estado Actual: READY", proceso->pid);
    //pthread_mutex_unlock(&mx_log);
    sem_post(&s_cont_ready);
    sem_post(&s_ready_execute);

  //  log_info(logger, "se envio proceso a cpu");
    //y todo el log de que algo entro a ready y lo tira como lista

	//liberar_conexion(cliente_socket);

	return;
}

void solicitar_tabla_de_segmentos(PCB_t* pcb){
	op_code op=CREAR_TABLA;
	uint32_t cantElementos=list_size(pcb->segmentos);
	pthread_mutex_lock(&mx_memoria);
    send(memoria_fd,&op,sizeof(op_code),0);
	send(memoria_fd,&(pcb->pid),sizeof(uint16_t),0);
	send(memoria_fd,&cantElementos,sizeof(uint32_t),0);
	//pthread_mutex_unlock(&mx_memoria);
	int i=0;
	while(i<cantElementos){
	    uint32_t sid = list_get(pcb->nros_segmentos,i);
	    uint32_t tamanio = list_get(pcb->segmentos,i);
	    //pthread_mutex_lock(&mx_memoria);
	    send(memoria_fd,&sid ,sizeof(uint32_t),0);
	    send(memoria_fd,&tamanio, sizeof(uint32_t),0);
	    //pthread_mutex_unlock(&mx_memoria);
	    i++;
	}



	//pthread_mutex_lock(&mx_memoria);
    recv(memoria_fd,&op,sizeof(op_code),0);
    pthread_mutex_unlock(&mx_memoria);
}

int server_escuchar(char* server_name, int server_socket) {
    cliente_socket = esperar_cliente(logger, server_name, server_socket);


    if (cliente_socket != -1) {
        pthread_t hilo;
        t_procesar_conexion_args* args = malloc(sizeof(t_procesar_conexion_args));
        args->fd = cliente_socket;
        args->server_name = server_name;
        pthread_create(&hilo, NULL, (void*) procesar_conexion, (void*) args);
        pthread_detach(hilo);
        return 1;
    }
    return 0;
}

//CLIENTE
//CPU
int generar_conexiones(int* interrupt_fd, int* dispatch_fd, t_config_kernel* configuracion) {
    char* port_dispatch = string_itoa(configuracion->PUERTO_CPU_DISPATCH);
    char* port_interrupt = string_itoa(configuracion->PUERTO_CPU_INTERRUPT);

    *dispatch_fd = crear_conexion(
            logger,
            "CPU DISPATCH",
            configuracion->IP_CPU,
            port_dispatch
    );

    free(port_dispatch);

    *interrupt_fd = crear_conexion(
            logger,
            "CPU INTERRUPT",
            configuracion->IP_CPU,
            port_interrupt
    );

    free(port_interrupt);

    return *interrupt_fd != 0 && *dispatch_fd != 0;
}

//MEMORIA
int generar_conexion_memoria(int* memoria_fd, t_config_kernel* configuracion) {
    char* port_memoria = string_itoa(configuracion->PUERTO_MEMORIA);

    *memoria_fd = crear_conexion(
            logger,
            "MEMORIA",
            configuracion->IP_MEMORIA,
            port_memoria
    );

    free(port_memoria);

    return *memoria_fd != 0;
}

