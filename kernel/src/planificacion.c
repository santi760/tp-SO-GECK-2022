/*
 * planificacion.c
 *
 *  Created on: Sep 29, 2022
 *      Author: utnso
 */

#include "planificacion.h"


pthread_mutex_t mx_cola_new = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mx_hay_interrupcion = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mx_cola_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mx_cola_ready_sec= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mx_lista_block = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mx_lista_new = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mx_log = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mx_cola_blocked = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mx_cpu_desocupado = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mx_memoria = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mx_cpu = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mx_pageFault = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mx_interrupt = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mx_hilo_pageFault = PTHREAD_MUTEX_INITIALIZER;

sem_t s_pasaje_a_ready, s_ready_execute,s_cpu_desocupado,s_cont_ready,s_multiprogramacion_actual,s_esperar_cpu,s_pcb_desalojado,s_blocked,s_io;
sem_t s_ios[10];
t_queue* cola_new;
t_queue* cola_ready;
t_queue* cola_ready_sec;
t_list* list_blocked;

bool cpu_desocupado=true;
bool hay_interrupcion=false;

void fifo_ready_execute(){
	while(1){
		sem_wait(&s_ready_execute);
		sem_wait(&s_cpu_desocupado); // Para que no ejecute cada vez que un proceso llega a ready
		sem_wait(&s_cont_ready); // Para que no intente ejecutar si la lista de ready esta vacia
		pthread_mutex_lock(&mx_cola_ready);
		PCB_t* proceso = queue_pop(cola_ready);
		pthread_mutex_unlock(&mx_cola_ready);
		//pthread_mutex_lock(&mx_log);
		log_info(logger,"PID: %d - Estado Anterior: READY - Estado Actual: EXECUTE", proceso->pid);
		//pthread_mutex_unlock(&mx_log);
		pthread_mutex_lock(&mx_cpu);
		send_proceso(dispatch_fd, proceso,DISPATCH);
		//pthread_mutex_unlock(&mx_cpu);
		pcb_destroy(proceso);
		sem_post(&s_esperar_cpu);
	}
}

void rr_ready_execute(){
	while(1){
		sem_wait(&s_ready_execute);
		sem_wait(&s_cpu_desocupado); // Para que no ejecute cada vez que un proceso llega a ready
		sem_wait(&s_cont_ready); // Para que no intente ejecutar si la lista de ready esta vacia
		pthread_mutex_lock(&mx_cola_ready);
		PCB_t* proceso = queue_pop(cola_ready);
		pthread_mutex_unlock(&mx_cola_ready);
		//pthread_mutex_lock(&mx_log);
		log_info(logger,"PID: %d - Estado Anterior: READY - Estado Actual: EXECUTE", proceso->pid);
		//pthread_mutex_unlock(&mx_log);
		pthread_mutex_lock(&mx_cpu);
		send_proceso(dispatch_fd, proceso,DISPATCH);
		//pthread_mutex_unlock(&mx_cpu);
		pcb_destroy(proceso);
		pthread_mutex_lock(&mx_cpu_desocupado);
		cpu_desocupado=false;
		pthread_mutex_unlock(&mx_cpu_desocupado);
		sem_post(&s_esperar_cpu);
		usleep(configuracion->QUANTUM_RR*1000);
		if(!cpu_desocupado){
			//pthread_mutex_lock(&mx_log);
			log_info(logger,"mando interrupt");
			//pthread_mutex_unlock(&mx_log);
			pthread_mutex_lock(&mx_interrupt);
			send(interrupt_fd,INTERRUPT,sizeof(op_code),0);
			pthread_mutex_unlock(&mx_interrupt);
			hay_interrupcion=true;
		}
	}
}

void feedback_ready_execute(){
	while(1){
		sem_wait(&s_ready_execute);
		sem_wait(&s_cpu_desocupado); // Para que no ejecute cada vez que un proceso llega a ready
		sem_wait(&s_cont_ready); // Para que no intente ejecutar si la lista de ready esta vacia
		if(!queue_is_empty(cola_ready)){
			pthread_mutex_lock(&mx_cola_ready);
			PCB_t* proceso = queue_pop(cola_ready);
			pthread_mutex_unlock(&mx_cola_ready);
			//pthread_mutex_lock(&mx_log);
			log_info(logger,"PID: %d - Estado Anterior: READY - Estado Actual: EXECUTE", proceso->pid);
			//pthread_mutex_unlock(&mx_log);
			pthread_mutex_lock(&mx_cpu);
			send_proceso(dispatch_fd, proceso,DISPATCH);
			//pthread_mutex_unlock(&mx_cpu);
			pcb_destroy(proceso);
			cpu_desocupado=false;
			sem_post(&s_esperar_cpu);
			usleep(configuracion->QUANTUM_RR*1000);
			if(!cpu_desocupado){
				pthread_mutex_lock(&mx_interrupt);
				send(interrupt_fd,INTERRUPT,sizeof(op_code),0);
				pthread_mutex_unlock(&mx_interrupt);
			}
		}
		else{
			pthread_mutex_lock(&mx_cola_ready_sec);
			PCB_t* proceso = queue_pop(cola_ready_sec);
			pthread_mutex_unlock(&mx_cola_ready_sec);
			//pthread_mutex_lock(&mx_log);
			log_info(logger,"PID: %d - Estado Anterior: READY - Estado Actual: EXECUTE", proceso->pid);
			//pthread_mutex_unlock(&mx_log);
			pthread_mutex_lock(&mx_cpu);
			send_proceso(dispatch_fd, proceso,DISPATCH);
			//pthread_mutex_unlock(&mx_cpu);
			pcb_destroy(proceso);
			sem_post(&s_esperar_cpu);
		}
	}
}

void execute_a_exit(PCB_t* pcb){
	//pthread_mutex_lock(&mx_log);
    log_info(logger,"PID: %d - Estado Anterior: EXECUTE - Estado Actual: EXIT", pcb->pid);
    //pthread_mutex_unlock(&mx_log);
    sem_post(&s_multiprogramacion_actual);//cuando se finaliza
    //liberar_espacio_de_memoria(PCB); Liberamos las estructructuras de memoria
    pcb_destroy(pcb);
    //avisar_consola_finalizacion(); Funcion que le avisa a la consola que se finalizo correctamente
}



void inicializarPlanificacion(){
	cola_new=queue_create();
	cola_ready=queue_create();
	cola_ready_sec=queue_create();
	sem_init(&s_ready_execute,0,0);
	sem_init(&s_cpu_desocupado, 0, 1);
	sem_init(&s_esperar_cpu, 0, 0);
	sem_init(&s_cont_ready,0,0);
	sem_init(&s_io, 0, 1);
	for(int i=0;i<10;i++){
		sem_init(&s_ios[i], 0, 1);
	}
	sem_init(&s_multiprogramacion_actual, 0, configuracion->GRADO_MAX_MULTIPROGRAMACION);
	pthread_t corto_plazo;
	if(!strcmp(configuracion->ALGORITMO_PLANIFICACION,"FIFO")){
		pthread_create(&corto_plazo, NULL, (void*) fifo_ready_execute, NULL);
		//pthread_mutex_lock(&mx_log);
		log_info(logger,"ALGORITMO_PLANIFICACION FIFOOO!!!!");
		//pthread_mutex_unlock(&mx_log);
	}
	else if(!strcmp(configuracion->ALGORITMO_PLANIFICACION,"RR")){
		pthread_create(&corto_plazo, NULL, (void*) rr_ready_execute, NULL);
		//pthread_mutex_lock(&mx_log);
		log_info(logger,"ALGORITMO_PLANIFICACION RR!!!!");
		//pthread_mutex_unlock(&mx_log);
	}
	else if(!strcmp(configuracion->ALGORITMO_PLANIFICACION,"FEEDBACK")){
		pthread_create(&corto_plazo, NULL, (void*) feedback_ready_execute, NULL);
	}
	else{
		//pthread_mutex_lock(&mx_log);
		log_info(logger,"ALGORITMO_PLANIFICACION INVALIDO!!!!");
		//pthread_mutex_unlock(&mx_log);
	}
	pthread_t espera_CPU;
	pthread_create(&espera_CPU, NULL, (void*) esperar_cpu, NULL);
}




//int  string_array_size(char** array);
/****Hilo NEW -> READY */

void pageFault(PCB_t* pcb){
	uint32_t segmento=0;
	uint32_t pagina=0;
	op_code op = PAGEFAULT;
	sem_wait(&s_blocked);
	recv(dispatch_fd,&segmento,sizeof(uint32_t),0);
	recv(dispatch_fd,&pagina,sizeof(uint32_t),0);
	pthread_mutex_unlock(&mx_pageFault);
	pthread_mutex_unlock(&mx_cpu);
	//pthread_mutex_lock(&mx_log);
	log_info(logger, "Page Fault PID: %d - Segmento: %d - Pagina: %d", pcb->pid,segmento,pagina);
	//pthread_mutex_unlock(&mx_log);

	pthread_mutex_lock(&mx_memoria);
	send(memoria_fd,&op,sizeof(op_code),0);
	send(memoria_fd,&(pcb->pid),sizeof(uint16_t),0);
	send(memoria_fd,&segmento,sizeof(uint32_t),0);
	send(memoria_fd,&pagina,sizeof(uint32_t),0);
	recv(memoria_fd,&op,sizeof(op_code),0);
	pthread_mutex_unlock(&mx_memoria);
	//log_info(logger, "pase");

	pthread_mutex_lock(&mx_cola_ready);
	queue_push(cola_ready, pcb);
	pthread_mutex_unlock(&mx_cola_ready);
	sem_post(&s_ready_execute);
	sem_post(&s_cont_ready);
}

void esperar_cpu(){
	while(1){
		sem_wait(&s_esperar_cpu);
		op_code cop;
		PCB_t* pcb = pcb_create();
		//pthread_mutex_lock(&mx_cpu);
		pthread_mutex_lock(&mx_pageFault);
		if (recv(dispatch_fd, &cop, sizeof(op_code), 0) <= 0) {
			//pthread_mutex_lock(&mx_log);
			log_error(logger,"DISCONNECT FAILURE!");
			//pthread_mutex_unlock(&mx_log);
			exit(-1);
		}
		if(cop!=PAGEFAULT){
			pthread_mutex_unlock(&mx_pageFault);
			pthread_mutex_unlock(&mx_cpu);
		}

		if (!recv_proceso(dispatch_fd, pcb)) {
			//pthread_mutex_lock(&mx_log);
			log_error(logger,"Fallo recibiendo PROGRAMA %d", pcb->pid);
			//pthread_mutex_unlock(&mx_log);
			exit(-1);
		}
		pthread_mutex_lock(&mx_cpu_desocupado);
		cpu_desocupado = true;
		pthread_mutex_unlock(&mx_cpu_desocupado);
		switch (cop) {
			case EXIT:
				send(pcb->cliente_fd,&cop,sizeof(op_code),0);
				execute_a_exit(pcb);
				sem_post(&s_cpu_desocupado);
				sem_post(&s_ready_execute);

	/*			pthread_mutex_lock(&mx_hay_interrupcion);
				if(hay_interrupcion){

					sem_post(&s_pcb_desalojado);
				}
				pthread_mutex_unlock(&mx_hay_interrupcion); */
				break;

			case INTERRUPT:
				//pthread_mutex_lock(&mx_log);
				log_info(logger,"PID: %d - Estado Anterior: EXECUTE - Estado Actual: READY", pcb->pid);
				//pthread_mutex_unlock(&mx_log);
				if(!strcmp(configuracion->ALGORITMO_PLANIFICACION,"RR")){
					pthread_mutex_lock(&mx_cola_ready);
					queue_push(cola_ready, pcb);
					pthread_mutex_unlock(&mx_cola_ready);
				}
				else{
					//pthread_mutex_lock(&mx_log);
					log_info(logger,"A cola secundaria");
					//pthread_mutex_unlock(&mx_log);
					pthread_mutex_lock(&mx_cola_ready_sec);
					queue_push(cola_ready_sec, pcb);
					pthread_mutex_unlock(&mx_cola_ready_sec);
				}
				sem_post(&s_cont_ready);
		//		sem_post(&s_pcb_desalojado);
				sem_post(&s_ready_execute);
				sem_post(&s_cpu_desocupado);
					break;

			case IO:
				//pthread_mutex_lock(&mx_cola_blocked);
				//queue_push(cola_blocked,pcb);
				//pthread_mutex_unlock(&mx_cola_blocked);
				//Creamos el hilo aparte que lo bloquee y se encargue de su io
				pthread_t hilo_bloqueado;
				sem_post(&s_blocked);
				pthread_create(&hilo_bloqueado,NULL,(void*)bloqueando,pcb);
				pthread_detach(hilo_bloqueado);
				 //pthread_mutex_lock(&mx_log);
				log_info(logger, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: BLOCKED", pcb->pid);
				 //pthread_mutex_unlock(&mx_log);
				sem_post(&s_cpu_desocupado);

			/* 	//Por si la interrupcion se mando cuando se estaba procesando la instruccion IO
				pthread_mutex_lock(&mx_hay_interrupcion);
				if(hay_interrupcion){
					pthread_mutex_unlock(&mx_hay_interrupcion);
					sem_post(&s_pcb_desalojado);
				}
				pthread_mutex_unlock(&mx_hay_interrupcion);*/
				break;
			case SIGSEGV:
				send(pcb->cliente_fd,&cop,sizeof(op_code),0);
				 //pthread_mutex_lock(&mx_log);
				log_error(logger,"Error: Segmentation Fault (SIGSEGV).");
				//pthread_mutex_unlock(&mx_log);
				execute_a_exit(pcb);
				sem_post(&s_cpu_desocupado);
				sem_post(&s_ready_execute);
				break;

				case PAGEFAULT:
					pthread_t hilo_pagefault;
					sem_post(&s_blocked);
					sem_post(&s_cpu_desocupado);
					pthread_mutex_lock(&mx_hilo_pageFault);
					pthread_create(&hilo_pagefault,NULL,(void*) pageFault,pcb);
					pthread_join(hilo_pagefault,0);
					pthread_mutex_unlock(&mx_hilo_pageFault);
					//recv(dispatch_fd,&segmento,sizeof(int),0);
					//recv(dispatch_fd,&pagina,sizeof(int),0);
					//log_error(logger,"Page Fault PID: %d - Segmento: %d - Pagina: %d",pcb->pid,segmento,pagina);
					//falta manejo

				break;

			default:
				log_error(logger, "AAAlgo anduvo mal en el server del kernel\n Cop: %d",cop);
		}
	}
}


void bloqueando(PCB_t* pcb){
	int i = 0;
	op_code cop;
	sem_wait(&s_blocked);
	INSTRUCCION* inst = list_get(pcb->instrucciones, pcb->pc - 1);
		//pthread_mutex_lock(&mx_log);
		log_info(logger, "instruccion numer %d",(pcb->pc-1));
		//pthread_mutex_unlock(&mx_log);
	if(!strcmp(inst->parametro,"TECLADO")){
		cop=TECLADO;
		uint16_t registro=0;
		uint32_t valor=0;
		//pthread_mutex_lock(&mx_log);
		log_info(logger, " PID: %d - Bloqueado por: %s", pcb->pid, inst->parametro);
		//pthread_mutex_unlock(&mx_log);
		send(pcb->cliente_fd,&cop,sizeof(op_code),0);
		//pthread_mutex_lock(&mx_log);
		log_info(logger, " lo mande");
		//pthread_mutex_unlock(&mx_log);
		recv(pcb->cliente_fd,&valor,sizeof(uint32_t),0);
		switch(inst->parametro2[1]){
			case 'A':
				registro=0;
				break;
			case 'B':
				registro=1;
			break;
			case 'C':
				registro=2;
			break;
			case 'D':
				registro=3;
			break;
			}
		pcb->registro_cpu[registro]=valor;


		//pthread_mutex_lock(&mx_log);
		log_info(logger, "PID: %d - Estado Anterior: BLOCKED - Estado Actual: READY", pcb->pid);
		//pthread_mutex_unlock(&mx_log);
		pthread_mutex_lock(&mx_cola_ready);
		queue_push(cola_ready, pcb);
		pthread_mutex_unlock(&mx_cola_ready);
		sem_post(&s_ready_execute);
		sem_post(&s_cont_ready);
	}
	else if(!strcmp(inst->parametro,"PANTALLA")){
		uint16_t registro = 0;
		op_code op = PANTALLA;
		send(pcb->cliente_fd,&op,sizeof(op_code),0);
		//pthread_mutex_lock(&mx_log);
		log_info(logger, " lo mande");
		//pthread_mutex_unlock(&mx_log);
		switch(inst->parametro2[1]){
			case 'A':
				registro=0;
				break;
			case 'B':
				registro=1;
			break;
			case 'C':
				registro=2;
			break;
			case 'D':
				registro=3;
			break;
			}
		send(pcb->cliente_fd,&pcb->registro_cpu[registro],sizeof(uint32_t),0);
		//pthread_mutex_lock(&mx_log);
		log_info(logger, " PID: %d - Bloqueado por: %s", pcb->pid, inst->parametro);
		//pthread_mutex_unlock(&mx_log);
		recv(pcb->cliente_fd,&op,sizeof(op_code),0);
		//pthread_mutex_lock(&mx_log);
		log_info(logger, "PID: %d - Estado Anterior: BLOCKED - Estado Actual: READY", pcb->pid);
		//pthread_mutex_unlock(&mx_log);
		pthread_mutex_lock(&mx_cola_ready);
		queue_push(cola_ready, pcb);
		pthread_mutex_unlock(&mx_cola_ready);
		sem_post(&s_ready_execute);
		sem_post(&s_cont_ready);

	}
	else{
		while(strcmp(inst->parametro,configuracion->DISPOSITIVOS_IO[i])){
			i++;}
		if(!strcmp(inst->parametro,configuracion->DISPOSITIVOS_IO[i])){ // HAY QUE VER COMO METERSE EN ESE CHAR ** DISPOSITIVOS IO PARA QUE HAGA EL STRCMP
			//pthread_mutex_lock(&mx_log);
			log_info(logger, " me meti al if");
			//pthread_mutex_unlock(&mx_log);
			sem_wait(&s_ios[i]);
			ejecutar_io(pcb,i);
			}
		}
	}


void ejecutar_io(PCB_t* pcb,int numero) {
		//pthread_mutex_lock(&mx_log);
		log_info(logger, " ejecutar io");
		//pthread_mutex_unlock(&mx_log);

		//pthread_mutex_lock(&mx_cola_blocked);
		/*if (list_size(cola_blocked) == 0){
			//pthread_mutex_lock(&mx_log);
			log_error(logger,"Blocked ejecutó sin un proceso bloqueado");
			//pthread_mutex_unlock(&mx_log);
		}*/

		INSTRUCCION* inst = list_get(pcb->instrucciones, pcb->pc - 1); //-1 porque ya se incremento el PC
		uint32_t tiempo = atoi(inst->parametro2);
		//pthread_mutex_lock(&mx_log);
		log_info(logger, " PID: %d - Bloqueado por: %s", pcb->pid, inst->parametro);
		//pthread_mutex_unlock(&mx_log);
		usleep(tiempo * 1000);
		//pthread_mutex_lock(&mx_log);
		log_info(logger, "PID: %d - Estado Anterior: BLOCKED - Estado Actual: READY", pcb->pid);
		//pthread_mutex_unlock(&mx_log);
		pthread_mutex_lock(&mx_cola_ready);
		queue_push(cola_ready, pcb);
		pthread_mutex_unlock(&mx_cola_ready);
		sem_post(&s_ready_execute);
		sem_post(&s_cont_ready);
		sem_post(&s_ios[numero]);
	}

