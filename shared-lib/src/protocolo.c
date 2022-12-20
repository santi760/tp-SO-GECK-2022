/*
 * protocolo.c
 *
 *  Created on: p 6, 2022
 *      Author: utnso
 */


#include "protocolo.h"

// DEBUG
bool send_debug(int fd) {
    op_code cop = DEBUG;
    if (send(fd, &cop, sizeof(op_code), 0) != sizeof(op_code)){
        return false;
    }
    return true;
}

//INSTRUCCIONES CONSOLA A KERNEL
void enviar_instrucciones(int socket_fd, t_list* lista, char** segmentos){

	uint32_t size = calcular_instrucciones_buffer_size(lista,segmentos);
	uint32_t sizeBuffer=size+ sizeof(int);
	void* stream = serializar_instrucciones_tam(size, lista, segmentos);
	t_buffer* buffer=malloc(sizeBuffer);
	buffer->size=size;
	buffer->stream=stream;

	void* a_enviar=malloc(sizeBuffer);
	int offset=0;

	memcpy(a_enviar + offset, &(buffer->size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(a_enviar + offset, buffer->stream, buffer->size);



	send(socket_fd, a_enviar,buffer->size+sizeof(uint32_t) ,0);

	free(a_enviar);
    free(buffer->stream);
    free(buffer);

}

t_instrucciones* recibir_instrucciones(int socket_fd)
{
	t_buffer* buffer=malloc(sizeof(t_buffer));

	recv(socket_fd, &(buffer->size), sizeof(int), MSG_WAITALL);
	buffer->stream=malloc(buffer->size);
	recv(socket_fd, buffer->stream, buffer->size, MSG_WAITALL);

	t_instrucciones* mensaje = deserializar_instrucciones(buffer);

	free(buffer->stream);
    free(buffer);

	return mensaje;
}

void* serializar_instrucciones_tam(uint32_t size, t_list* lista, char** segmentos) {

    INSTRUCCION* aux;

    aux = list_get(lista,0);
    uint32_t elementosLista= list_size(lista);

    uint32_t cantSegmentos= string_array_size(segmentos);

    uint32_t offset = 0;
    void* stream = malloc(size);

    memcpy(stream + offset, &elementosLista, sizeof(uint32_t));
    offset+= sizeof(uint32_t);

    t_link_element* aux1 = lista->head;

    while( aux1!=NULL )
	{
		INSTRUCCION* auxl2 = aux1->data;
		//printf("Verificamos la lista:\n");
		//printf("Comando: %s | Par1: %s | Par2: %s \n\n", auxl2->comando, auxl2->parametro, auxl2->parametro2 );

		memcpy(stream + offset, &auxl2->comando, sizeof(aux->comando));
		offset += sizeof(aux->comando);
		memcpy(stream + offset, &auxl2->parametro, sizeof(aux->parametro));
		offset += sizeof(aux->parametro);
		memcpy(stream + offset, &auxl2->parametro2, sizeof(aux->parametro2));
		offset += sizeof(aux->parametro2);
		aux1 = aux1->next;
	}

    memcpy(stream + offset, &cantSegmentos, sizeof(uint32_t));
    offset+= sizeof(uint32_t);

    int i=0;
    while(i<cantSegmentos){
    	uint32_t tamSegmento=atoi(segmentos[i]);
    	memcpy(stream + offset, &tamSegmento, sizeof(uint32_t));
    	offset += sizeof(uint32_t);
    	i++;
    }

    free(aux);
    return stream;
}

t_instrucciones* deserializar_instrucciones(t_buffer* buffer){
    int i=0, c=0;
	t_instrucciones* mensaje=malloc(sizeof(t_instrucciones));


	void* stream = buffer->stream;

	memcpy(&(mensaje->elementosLista), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	mensaje->listaInstrucciones=list_create();
	mensaje->listaTamSegmentos=list_create();
	while(i!=mensaje->elementosLista)
	{
		INSTRUCCION* aux=malloc(sizeof(INSTRUCCION));
		memcpy(&(aux->comando), stream, sizeof(aux->comando));
	    stream += sizeof(aux->comando);
	    memcpy(&(aux->parametro),stream , sizeof(aux->parametro));
	    stream += sizeof(aux->parametro);
	    memcpy(&(aux->parametro2), stream, sizeof(aux->parametro2));
	    stream += sizeof(aux->parametro2);

	    list_add(mensaje->listaInstrucciones,aux);
	    i++;
	}

	memcpy(&(mensaje->cantSegmentos), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    while (c<mensaje->cantSegmentos)
    {
    	uint32_t aux=0;
    	memcpy(&aux, stream, sizeof(uint32_t));
    	stream += sizeof(uint32_t);

    	list_add(mensaje->listaTamSegmentos,aux);
    	c++;
    }

	return mensaje;
}

uint32_t calcular_instrucciones_buffer_size(t_list* lista, char** segmentos){
	uint32_t size=0;
	int i=0;
	uint32_t cantSegmentos= string_array_size(segmentos);
	//t_list_iterator* listaIns = list_iterator_create(lista);
	//INSTRUCCIONES* aux = list_get(lista, 0);

	while(i<=lista->elements_count){
		size += 25;
		i++;
		//aux = list_iterator_next(listaIns);
		//list_iterator_next(listaIns);
	}
	size += (cantSegmentos+2)*sizeof(uint32_t);
	//free(listaIns);
	//free(aux);
	return size;
}

//ENVIAR PCB KERNEL->CPU_DISPATCH

bool send_proceso(int fd, PCB_t *proceso,op_code codigo) {
    size_t size;
    void* stream = serializar_proceso(&size, proceso,codigo);
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}


static void* serializar_proceso(size_t* size, PCB_t *proceso, op_code codigo) {


	uint32_t elementosLista= list_size(proceso->instrucciones);

	uint32_t cantSegmentos= list_size(proceso->segmentos);

	*size= sizeof(op_code)+   // 4
		   sizeof(size_t)+ //SIZE PAYLOAD
		   sizeof(uint16_t)+ //SIZE PID
		   sizeof(uint32_t)+ //SIZE PC
		   (sizeof(uint32_t)*4)+ //SIZE REGISTROS
		   sizeof(uint32_t)+ //SIZE elementosLista
		   ((3*25)*elementosLista)+ //SIZE LISTA INSTRUCCIONES
		   sizeof(uint32_t)+//SIZE cantSegmentos
		   sizeof(uint32_t)*cantSegmentos + //SIZE SEGMENTOS SIN LOS ID
		   sizeof(uint32_t)*cantSegmentos + //SIZE SEGMENTOS ID
		   sizeof(int); //SIZE CONSOLA FD

	size_t size_payload=*size- sizeof(op_code) -sizeof(size_t);
	uint32_t offset = 0;
	void* stream = malloc(*size);

	op_code cop = codigo;
	memcpy(stream + offset, &cop, sizeof(op_code));
	offset+= sizeof(op_code);

	memcpy(stream + offset, &size_payload, sizeof(size_t));
	offset+= sizeof(size_t);
	memcpy(stream + offset, &proceso->pid, sizeof(uint16_t));
	offset+= sizeof(uint16_t);
	memcpy(stream + offset, &proceso->pc, sizeof(uint32_t));
	offset+= sizeof(uint32_t);

	memcpy(stream + offset, &proceso->registro_cpu[0], sizeof(uint32_t));
	offset+= sizeof(uint32_t);
	memcpy(stream + offset, &proceso->registro_cpu[1], sizeof(uint32_t));
	offset+= sizeof(uint32_t);
	memcpy(stream + offset, &proceso->registro_cpu[2], sizeof(uint32_t));
	offset+= sizeof(uint32_t);
	memcpy(stream + offset, &proceso->registro_cpu[3], sizeof(uint32_t));
	offset+= sizeof(uint32_t);

	memcpy(stream + offset, &elementosLista, sizeof(uint32_t));
	offset+= sizeof(uint32_t);

	t_link_element* aux1 = proceso->instrucciones->head;

	while( aux1!=NULL )
	{
		INSTRUCCION* auxl2 = aux1->data;
		//printf("Verificamos la lista:\n");
		//printf("Comando: %s | Par1: %s | Par2: %s \n\n", auxl2->comando, auxl2->parametro, auxl2->parametro2 );

		memcpy(stream + offset, &auxl2->comando, sizeof(auxl2->comando));
		offset += sizeof(auxl2->comando);
		memcpy(stream + offset, &auxl2->parametro, sizeof(auxl2->parametro));
		offset += sizeof(auxl2->parametro);
		memcpy(stream + offset, &auxl2->parametro2, sizeof(auxl2->parametro2));
		offset += sizeof(auxl2->parametro2);
		aux1 = aux1->next;
	}

	memcpy(stream + offset, &cantSegmentos, sizeof(uint32_t));
	offset+= sizeof(uint32_t);

	int i=0;
	while(i<cantSegmentos){
		uint32_t auxSeg=list_get(proceso->segmentos,i);
		memcpy(stream + offset, &auxSeg, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		i++;
	}

	i=0;
	while(i<cantSegmentos){
		uint32_t auxSeg=list_get(proceso->nros_segmentos,i);
		memcpy(stream + offset, &auxSeg, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		i++;
		}

	memcpy(stream + offset, &proceso->cliente_fd, sizeof(int));

	//free(aux);
	return stream;
}

//Recepcion y deserializacion
bool recv_proceso(int fd, PCB_t* proceso) {
    size_t size_payload;
    if (recv(fd, &size_payload, sizeof(size_t), MSG_WAITALL) != sizeof(size_t))
        return false;
    void* stream = malloc(size_payload);
    if (recv(fd, stream, size_payload, MSG_WAITALL) != size_payload) {
        free(stream);
        return false;
    }
    deserializar_proceso(stream, proceso);
    free(stream);
    return true;
}

static void deserializar_proceso(void* stream, PCB_t* proceso) {
	int i=0,c=0;

	uint32_t elementosLista=0, cantSegmentos=0;

	memcpy(&(proceso->pid), stream, sizeof(uint16_t));
	stream+=sizeof(uint16_t);

	memcpy(&(proceso->pc), stream, sizeof(uint32_t));
	stream+=sizeof(uint32_t);

	memcpy(&(proceso->registro_cpu[0]),stream, sizeof(uint32_t));
	stream+=sizeof(uint32_t);
	memcpy(&(proceso->registro_cpu[1]),stream, sizeof(uint32_t));
	stream+=sizeof(uint32_t);
	memcpy(&(proceso->registro_cpu[2]),stream, sizeof(uint32_t));
	stream+=sizeof(uint32_t);
	memcpy(&(proceso->registro_cpu[3]),stream, sizeof(uint32_t));
	stream+=sizeof(uint32_t);
	//CHEQUEAR

	memcpy(&elementosLista, stream, sizeof(uint32_t));
	stream+=sizeof(uint32_t);

	proceso->instrucciones=list_create();
	proceso->segmentos=list_create();
	while(i!=elementosLista)
		{
			INSTRUCCION* aux=malloc(sizeof(INSTRUCCION));
			memcpy(&(aux->comando), stream, sizeof(aux->comando));
		    stream += sizeof(aux->comando);
		    memcpy(&(aux->parametro),stream , sizeof(aux->parametro));
		    stream += sizeof(aux->parametro);
		    memcpy(&(aux->parametro2), stream, sizeof(aux->parametro2));
		    stream += sizeof(aux->parametro2);

		    list_add(proceso->instrucciones,aux);
		    i++;
		}

	memcpy(&cantSegmentos, stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	while (c<cantSegmentos)
	    {
	    	uint32_t aux=0;
	    	memcpy(&aux, stream, sizeof(uint32_t));
	    	stream += sizeof(uint32_t);

	    	list_add(proceso->segmentos,aux);
	    	c++;
	    }

	c=0;
	while (c<cantSegmentos)
		{
			uint32_t aux=0;
			memcpy(&aux, stream, sizeof(uint32_t));
			stream += sizeof(uint32_t);

			list_add(proceso->nros_segmentos,aux);
			c++;
		}

	memcpy(&proceso->cliente_fd, stream, sizeof(int));


}
