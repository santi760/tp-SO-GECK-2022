#include "memoria_utils.h"

pthread_mutex_t mx_log = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mx_estructuras_clock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mx_bitarray_marcos_ocupados = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mx_memoria = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mx_fd_swaps = PTHREAD_MUTEX_INITIALIZER;

int cliente_kernel;
int cliente_cpu;
uint16_t pid_global;
// Tabla de páginas
//t_list* tabla_de_paginas;

t_list* lista_tablas_de_procesos;
// Estructura clock
t_dictionary* estructuras_clock;

// Memoria real
uint8_t* bitarray_marcos_ocupados;
void* memoria;

// fd de swaps
t_dictionary* fd_swaps;//Hay q cambiarlo a un solo archivo

// Datos config
char* algoritmo;
uint16_t tam_memoria;
uint16_t tam_pagina;
uint16_t entradas_por_tabla;
uint16_t retardo_memoria;
uint16_t marcos_por_proceso;
uint16_t puerto_escucha;

// Extras
uint16_t pid_actual;

uint16_t nro_segmento_anterior;

// funciones para no pasar a char cada vez que accedemos


estructura_clock* get_estructura_clock(uint16_t pid){
	char* key = string_itoa(pid);
	pthread_mutex_lock(&mx_estructuras_clock);
	estructura_clock* e_clock = dictionary_get(estructuras_clock, key);
	pthread_mutex_unlock(&mx_estructuras_clock);
	free(key);
	return e_clock;
}

void set_estructura_clock(uint16_t pid, estructura_clock* e_clock){
	char* key = string_itoa(pid);
	pthread_mutex_lock(&mx_estructuras_clock);
	dictionary_put(estructuras_clock, key, e_clock);
	pthread_mutex_unlock(&mx_estructuras_clock);
	free(key);
}

void del_estructura_clock(uint16_t pid){
	char* key = string_itoa(pid);
	pthread_mutex_lock(&mx_estructuras_clock);
	dictionary_remove_and_destroy(estructuras_clock, key, free);
	pthread_mutex_unlock(&mx_estructuras_clock);
	free(key);
}

////////////////////////////////////////

void inicializar_memoria(){

	tam_memoria = configuracion->TAM_MEMORIA;
	tam_pagina = configuracion->TAM_PAGINA;
	entradas_por_tabla = configuracion->ENTRADAS_POR_TABLA;
	retardo_memoria = configuracion->RETARDO_MEMORIA;
	marcos_por_proceso = configuracion->MARCOS_POR_PROCESO;
	algoritmo = configuracion->ALGORITMO_REEMPLAZO;


	log_info(logger,"Algoritmo %s configurado", configuracion->ALGORITMO_REEMPLAZO);

	log_info(logger, "Enviando a CPU: tam_pagina=%d - cant_ent_paginas=%d", configuracion->TAM_PAGINA, configuracion->ENTRADAS_POR_TABLA);
	//send(cliente_cpu, &entradas_por_tabla, sizeof(uint16_t),0);
	//send(cliente_cpu, &tam_pagina, sizeof(uint16_t),0);

	memoria = malloc(configuracion->TAM_MEMORIA);
	//tabla_de_paginas = list_create();
	lista_tablas_de_procesos = list_create();
	estructuras_clock = dictionary_create();

	bitarray_marcos_ocupados = malloc(tam_memoria / tam_pagina);
	for (int i = 0; i < tam_memoria / tam_pagina; i++)
		bitarray_marcos_ocupados[i] = 0;

	inicializar_swap();
}

/***********************************************************************/
/***************************** HILO KERNEL *****************************/
/***********************************************************************/



//// INICIALIZACIÓN DE PROCESO

// creas los marcos y las tablas necesarias para el proceso
t_list* crear_tabla(uint16_t pid){//Esto capaz funca
	t_list* tabla_de_paginas = list_create();
	for (int i = 0; i < entradas_por_tabla ; i++){
		fila_de_pagina* pagina = malloc(entradas_por_tabla * sizeof(fila_de_pagina));
		//inicializar_tabla_de_paginas(pagina);
		pagina->nro_marco=-1;
		pagina->modificado=0;
		pagina->presencia=0;
		pagina->uso=0;
		list_add(tabla_de_paginas, pagina);
	}
	return tabla_de_paginas;
}

void inicializar_tabla_de_paginas(fila_de_pagina* pagina){// modificada
	for (int i = 0; i < entradas_por_tabla; i++){
		pagina[i].nro_marco = -1;
	}
}

// FINALIZACIÓN DE PROCESO
void eliminar_estructuras(uint32_t tabla_paginas, uint16_t pid) {
	estructura_clock* estructura = get_estructura_clock(pid);
	fila_estructura_clock* fila_busqueda;
	for (int i = 0; i < list_size(estructura->marcos_en_memoria); i++){
		fila_busqueda = list_get(estructura->marcos_en_memoria, i);
		pthread_mutex_lock(&mx_bitarray_marcos_ocupados);
		bitarray_marcos_ocupados[fila_busqueda->nro_marco_en_memoria] = 0;
		pthread_mutex_unlock(&mx_bitarray_marcos_ocupados);
	}
	list_destroy_and_destroy_elements(estructura->marcos_en_memoria, free);
	del_estructura_clock(pid);
}

/***********************************************************************/
/******************************* HILO CPU ******************************/
/***********************************************************************/

//modificar a una sola tabla
uint32_t obtener_nro_marco_memoria(uint32_t num_segmento, uint32_t num_pagina, uint16_t pid_actual){//corregir lo de buscar en swap

	t_list* tabla_de_proceso=list_create();
	tabla_de_proceso=list_get(lista_tablas_de_procesos,pid_actual);
	t_list* tabla_de_marcos = list_create();
	tabla_de_marcos = list_get(tabla_de_proceso, num_segmento);

	fila_de_pagina* pagina = list_get(tabla_de_marcos, num_pagina);
	if (pagina->presencia == 1){
		log_info(logger, "[CPU] Pagina en memoria principal!");
		pagina->uso = 1;
		uint32_t nro_marco_en_swap = num_segmento*entradas_por_tabla + num_pagina;

		pthread_mutex_lock(&mx_bitarray_marcos_ocupados);
		bitarray_marcos_ocupados[pagina->nro_marco] = 1;
		pthread_mutex_unlock(&mx_bitarray_marcos_ocupados);
		pagina->presencia = 1;
		pagina->uso = 1;

		agregar_pagina_a_estructura_clock(pagina->nro_marco, pagina, nro_marco_en_swap, pid_actual);
		return pagina->nro_marco;
		list_destroy_and_destroy_elements(tabla_de_marcos ,free);
	}
	//list_destroy_and_destroy_elements(tabla_de_marcos ,free);
	return -1;
}

uint32_t tratar_page_fault(uint32_t num_segmento, uint32_t num_pagina, uint16_t pid_actual){
	t_list* tabla_de_proceso=list_create();
	tabla_de_proceso=list_get(lista_tablas_de_procesos,pid_actual);
	t_list* tabla_de_marcos = list_create();
	tabla_de_marcos = list_get(tabla_de_proceso, num_segmento);
	fila_de_pagina* pagina = list_get(tabla_de_marcos, num_pagina);

	log_info(logger, "[CPU][ACCESO A DISCO] PAGE FAULT!!!");
	uint32_t nro_marco_en_swap = num_segmento*entradas_por_tabla + num_pagina;
	void* marco = leer_marco_en_swap(fd, nro_marco_en_swap, tam_pagina);
	int32_t nro_marco;
	if (marcos_en_memoria(pid_actual) == marcos_por_proceso){
		nro_marco = usar_algoritmo(pid_actual);
		log_info(logger, "REEMPLAZO - PID: <%d> - Marco: <%d> - Page In: <SEGMENTO %d>|<PAGINA %d>" ,pid_actual,nro_marco,num_segmento,num_pagina);//borramos page out
	}
	else{
		nro_marco = buscar_marco_libre();
		log_info(logger, "[CPU] El proceso tiene marcos disponibles :)");
		if (nro_marco == -1){
			log_error(logger, "ERROR!!!!! NO HAY MARCOS LIBRES EN MEMORIA!!!");
			exit(EXIT_FAILURE);
		}
	}
	pthread_mutex_lock(&mx_bitarray_marcos_ocupados);
	bitarray_marcos_ocupados[nro_marco] = 1;
	pthread_mutex_unlock(&mx_bitarray_marcos_ocupados);
	pagina->nro_marco = nro_marco;
	pagina->presencia = 1;
	pagina->uso = 1;
	escribir_marco_en_memoria(pagina->nro_marco, marco);

	free(marco);
	agregar_pagina_a_estructura_clock(nro_marco, pagina, nro_marco_en_swap, pid_actual);
	log_info(logger, "[CPU]LECTURA EN SWAP: SWAP IN -  PID: <%d> - Marco: <%d> - Page In: <SEGMENTO %d>|<PÁGINA %d>",pid_actual,nro_marco,num_segmento,num_pagina);
	return nro_marco;
}

// Vemos que algoritmo usar y lo usamos
uint32_t usar_algoritmo(int pid){//ESTO ESTA BIEN
	if (strcmp(algoritmo, "CLOCK-M") == 0){
		log_info(logger, "[CPU] Reemplazo por CLOCK-M");
		return clock_modificado(pid);
	}
	else if (strcmp(algoritmo, "CLOCK") == 0){
		log_info(logger, "[CPU] Reemplazo por CLOCK");
		return clock_simple(pid);
	}
	else{
		exit(-1);
	}
}

uint32_t clock_simple(int pid_actual){ //ESTO ESTA BIEN
	// hasta que no encuentre uno no para
	estructura_clock* estructura = get_estructura_clock(pid_actual);
	uint16_t puntero_clock = estructura->puntero;
	fila_estructura_clock* fila;
	fila_de_pagina* pagina;
	uint32_t nro_marco_en_swap;
	int32_t nro_marco;
	while(1){
		fila = list_get(estructura->marcos_en_memoria, puntero_clock);
		pagina = fila->pagina;
		nro_marco_en_swap = fila->nro_marco_en_swap;
		nro_marco = fila->nro_marco_en_memoria;
		puntero_clock = avanzar_puntero(puntero_clock);

		if (pagina->uso == 0){ //Encontró a la víctima
			reemplazo_por_clock(nro_marco_en_swap, pagina, pid_actual);
			log_info(logger, "[CPU] Posición final puntero: %d",puntero_clock);
			estructura->puntero = puntero_clock; //Guardo el puntero actualizado.
			return nro_marco;
		}
		else{
			pagina->uso = 0;
		}
	}

	return 0;
}

uint32_t clock_modificado(int pid_actual){//ESTO ESTA BIEN
	estructura_clock* estructura = get_estructura_clock(pid_actual);
	uint16_t puntero_clock = estructura->puntero;
	fila_estructura_clock* fila;
	fila_de_pagina* pagina;
	uint32_t nro_marco_en_swap;
	int32_t nro_marco;
	// hasta que no encuentre uno no para
	while(1){
		// 1er paso del algoritmo: Buscar (0,0)
		while(1){
			fila = list_get(estructura->marcos_en_memoria, puntero_clock);
			pagina = fila->pagina;
			nro_marco_en_swap = fila->nro_marco_en_swap;
			nro_marco = fila->nro_marco_en_memoria;
			puntero_clock = avanzar_puntero(puntero_clock);

			if (pagina->uso == 0 && pagina->modificado == 0){
				reemplazo_por_clock(nro_marco_en_swap, pagina, pid_actual);
				log_info(logger, "[CPU] Posición final puntero: %d",puntero_clock);
				estructura->puntero = puntero_clock; //Guardo el puntero actualizado.
				return nro_marco;
			}
			//Condición para ir al siguiente paso
			if (puntero_clock == estructura->puntero){
				break;
			}
		}
		// 2do paso del algoritmo: Buscar (0,1) y pasar Bit de uso a 0
		while(1){
			fila = list_get(estructura->marcos_en_memoria, puntero_clock);
			pagina = fila->pagina;
			nro_marco_en_swap = fila->nro_marco_en_swap;
			nro_marco = fila->nro_marco_en_memoria;
			puntero_clock = avanzar_puntero(puntero_clock);

			if (pagina->uso == 0 && pagina->modificado == 1){
				reemplazo_por_clock(nro_marco_en_swap, pagina, pid_actual);
				estructura->puntero = puntero_clock; //Guardo el puntero actualizado.
				return nro_marco;
			}
			else{
				pagina->uso = 0;
			}

			//Condición para ir al siguiente paso
			if (puntero_clock == estructura->puntero){
				break;
			}
		}
		// 3er paso del algoritmo, volver a empezar
	}

	return 0;
}

void reemplazo_por_clock(uint32_t nro_marco_en_swap, fila_de_pagina* entrada, int pid_actual){//ESTO ESTA BIEN
	// logica de swappeo de marco
	log_info(logger, "[CPU] Pagina victima elegida: %d",nro_marco_en_swap);
	// si tiene el bit de modificado en 1, hay que actualizar el archivo swap
	if (entrada->modificado == 1){
		actualizar_marco_en_swap(fd, nro_marco_en_swap, obtener_marco(entrada->nro_marco), tam_pagina);
		//uint32_t nro_marco_en_swap = num_segmento*entradas_por_tabla + num_pagina;
		uint16_t num_segmento = floor(nro_marco_en_swap / entradas_por_tabla);
		uint16_t num_pagina = nro_marco_en_swap % entradas_por_tabla;

		log_info(logger, "[CPU] ESCRITURA SWAP OUT -  PID: <%d> - Marco: <%d> - Page Out: <SEGMENTO %d>|<PÁGINA %d>",pid_actual,entrada->nro_marco,num_segmento,num_pagina);
		usleep(configuracion->RETARDO_SWAP * 1000); // tenemos el retardo por swappear un marco modificado
		entrada->modificado = 0;
	}
	entrada->presencia = 0;
	pthread_mutex_lock(&mx_bitarray_marcos_ocupados);
	bitarray_marcos_ocupados[entrada->nro_marco] = 0;
	pthread_mutex_unlock(&mx_bitarray_marcos_ocupados);
}

// Dado un nro de marco y un desplazamiento, devuelve el dato en concreto
uint32_t read_en_memoria(uint32_t nro_marco, uint32_t desplazamiento, uint16_t pid_actual){
	uint32_t desplazamiento_final = nro_marco * tam_pagina + desplazamiento;
	uint32_t dato;
	pthread_mutex_lock(&mx_memoria);
	memcpy(&dato, memoria + desplazamiento_final, sizeof(dato));
	pthread_mutex_unlock(&mx_memoria);
	fila_de_pagina* pagina_actual = obtener_pagina(pid_actual, nro_marco);
	pagina_actual->uso = 1;
	log_info(logger, "[CPU] Dato '%d' leido en MARCO: %d, DESPLAZAMIENTO: %d",dato, nro_marco,desplazamiento);
	return dato;
}

// Dado un nro de marco, un desplazamiento y un dato, escribe el dato en dicha posicion
void write_en_memoria(uint32_t nro_marco, uint32_t desplazamiento, uint32_t dato, uint16_t pid_actual) {//VAN??
	uint32_t desplazamiento_final = nro_marco * tam_pagina + desplazamiento;
	pthread_mutex_lock(&mx_memoria);
	memcpy(memoria + desplazamiento_final, &dato, sizeof(dato));
	pthread_mutex_unlock(&mx_memoria);
	fila_de_pagina* pagina_actual = obtener_pagina(pid_actual, nro_marco);//ACA NO ENCUENTRA PAGINA
	pagina_actual->uso = 1;
	pagina_actual->modificado = 1;
}

////////////////////////// FUNCIONES GENERALES //////////////////////////

// Devuelve el contenido de un marco que está en memoria.
void* obtener_marco(uint32_t nro_marco){//ESTO ESTA BIEN
	void* marco = malloc(tam_pagina);
	pthread_mutex_lock(&mx_memoria);
	memcpy(marco, memoria + nro_marco * tam_pagina, tam_pagina);
	pthread_mutex_unlock(&mx_memoria);
	return marco;
}

// Cuenta la cantidad de marcos en memoria que tiene un proceso
uint32_t marcos_en_memoria (uint16_t pid_actual){//ESTO ESTA BIEN
	estructura_clock* estructura = get_estructura_clock(pid_actual);
	return list_size(estructura->marcos_en_memoria);
}

void escribir_marco_en_memoria(uint32_t nro_marco, void* marco){//ESTO ESTA BIEN
	uint32_t marco_en_memoria = nro_marco * tam_pagina;
	pthread_mutex_lock(&mx_memoria);
	memcpy(memoria + marco_en_memoria, marco, tam_pagina);
	pthread_mutex_unlock(&mx_memoria);
}


// Buscar el primer marco libre
int buscar_marco_libre(){//ESTO ESTA BIEN
	pthread_mutex_lock(&mx_bitarray_marcos_ocupados);
	for (int i = 0; i < tam_memoria / tam_pagina; i++){
		if (bitarray_marcos_ocupados[i] == 0){
			pthread_mutex_unlock(&mx_bitarray_marcos_ocupados);
			return i;
		}
	}
	pthread_mutex_unlock(&mx_bitarray_marcos_ocupados);
	return -1;
}

// Devuelve la cantidad de marcos que requiere un proceso del tamanio especificado
uint32_t calcular_cant_marcos(uint16_t tamanio){//ESTO ESTA BIEN
	int  cant_marcos = tamanio / tam_pagina;
	if (tamanio % tam_pagina != 0)
		cant_marcos++;
	return cant_marcos;
}

int marcos_actuales(int tamanio, int entrada){
	return tamanio * entradas_por_tabla + entrada;
}


////////////////////////// ESTRUCTURA CLOCK //////////////////////////

void agregar_pagina_a_estructura_clock(int32_t nro_marco, fila_de_pagina* pagina, uint32_t nro_marco_en_swap, uint16_t pid_actual){//ESTO ESTA BIEN
	//Si ya está ese marco en la estructura, actualizo su página.
	estructura_clock* estructura = get_estructura_clock(pid_actual);
	fila_estructura_clock* fila_busqueda;
	for (int i = 0; i < list_size(estructura->marcos_en_memoria); i++){
		fila_busqueda = list_get(estructura->marcos_en_memoria, i);
		if (nro_marco == fila_busqueda->nro_marco_en_memoria){
			fila_busqueda->pagina = pagina;
			fila_busqueda->nro_marco_en_swap = nro_marco_en_swap;
			return;
		}
	}
	//Si no está el marco en la estructura, lo agrego al final
	fila_estructura_clock* fila_nueva = malloc(sizeof(fila_estructura_clock));
	fila_nueva->nro_marco_en_memoria = nro_marco;
	fila_nueva->pagina = pagina;
	fila_nueva->nro_marco_en_swap = nro_marco_en_swap;
	list_add(estructura->marcos_en_memoria, fila_nueva);

}

void crear_estructura_clock(uint16_t pid){
	estructura_clock* estructura = malloc(sizeof(estructura_clock));
	estructura->pid = pid;
	estructura->marcos_en_memoria = list_create();
	estructura->puntero = 0;
	set_estructura_clock(pid, estructura);
	log_info(logger,"[CPU] Creada estructura clock del proceso: %d", pid);
}


// avanza al proximo marco del proceso
uint16_t avanzar_puntero(uint16_t puntero_clock){
    puntero_clock++;
    if(puntero_clock == marcos_por_proceso)
        return 0;
    return puntero_clock;
}

// Devuelve el puntero a una página
fila_de_pagina* obtener_pagina(uint16_t pid_actual, int32_t nro_marco){//ESTO ESTA BIEN
	estructura_clock* estructura = get_estructura_clock(pid_actual);//EN LA SEGUNDA VUELTA VUELVE VACIO
	fila_estructura_clock* fila_busqueda;
	for (int i = 0; i < list_size(estructura->marcos_en_memoria); i++){
		fila_busqueda = list_get(estructura->marcos_en_memoria, i);
		if (nro_marco == fila_busqueda->nro_marco_en_memoria){
			return fila_busqueda->pagina;
		}
	}
	log_error(logger,"Pagina no encontrada :(");
	return NULL;
}
