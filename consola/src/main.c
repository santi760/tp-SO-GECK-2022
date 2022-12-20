/*
 * main.c
 *
 *  Created on: Sep 6, 2022
 *      Author: utnso
 */

#include "main.h"

int main(int argc, char** argv){
	// ESTO NO VA ES SOLO PARA PROBAR
	//argv[1] = "/home/utnso//Desktop/TP 2do cuatri 2022/tp-2022-2c-Grupo-54/consola/instrucciones.txt";
	//argv[2] = "/home/utnso/Documents/tp-2022-2c-Grupo-54/consola/consola.conf";

	int kernel_fd;
	cargarConfiguracion(argv[2]);
	generar_conexion(&kernel_fd, configuracion);
	uint32_t valor=0;

	/*int i = 0;
	while(segmentos[i]!=NULL){
			printf("SEGMENTO: %s",segmentos[i]);
			i++;
		}*/

	//printf("Path: %s \n", argv[2]);
	// Parseo del archivo y armado de lista
	t_list* listaInstrucciones = list_create();

	// argv[1] es el path de la primera posicion que recibe el main
	parseo_instrucciones(argv[1],listaInstrucciones);

	enviar_instrucciones(kernel_fd, listaInstrucciones, configuracion->SEGMENTOS);

	while(1){
		op_code cod,cop_aux;
		recv(kernel_fd,&cod,sizeof(op_code),MSG_WAITALL);
		switch(cod){
			case PANTALLA:
				cop_aux = PANTALLA;
				recv(kernel_fd,&valor,sizeof(uint32_t),0);
				log_info(logger,"valor registro:%d",valor);
				usleep(configuracion->TIEMPO_PANTALLA*1000);
				send(kernel_fd,&cop_aux,sizeof(op_code),0);
			break;
			case TECLADO:
				log_info(logger,"Ingrese valor:");
				scanf("%d",&valor);
				send(kernel_fd,&valor,sizeof(uint32_t),0);
			break;
			case SIGSEGV:
				log_info(logger, "Se realizo el envio se desconecto del kernel, el proceso tuvo error SIGSEGV");
			    limpiarConfiguracion();
				exit(-1);
			break;
			case EXIT:
				log_info(logger, "Se realizo el envio se desconecto del kernel");
				limpiarConfiguracion();
				exit(-1);
			break;
			default:
				break;
		}
	}
}


void parseo_instrucciones(char* path_instrucciones,t_list* listaInstrucciones){

	int i=0;

		FILE* pseudocode;
		//printf("Path: %s \n", path_instrucciones);
		//printf("Parametro a envia con LISTA: %d \n", atoi(argv[2]));

		pseudocode = fopen(path_instrucciones,"r");   		  // ABRO MODO Lectura
		char bufer[LONGITUD_MAXIMA_LINEA];		  // Genero buffer

		if(!pseudocode){
			printf("No se pudo acceder al archivo\n");
			return ;
		}

		char **linea;
		char *separador = " ";

		//t_list* listaInstrucciones = list_create();

		while(fgets(bufer, LONGITUD_MAXIMA_LINEA, pseudocode))
		{
			// ACA IMPRIMO CONTENIDO DE CADA LINEA
			linea =  string_split(bufer, separador);

			INSTRUCCION *instrucs;
			instrucs = (INSTRUCCION*)malloc(sizeof(INSTRUCCION));

			if(!instrucs){
					printf("No se ha podido reservar memoria.\n");
					exit(1);
			}
			//instrucs->comando = malloc(sizeof(instrucs->comando));
			//instrucs->parametro;
			//instrucs->parametro2;

			for(i = 0; linea[i] != '\0'; i++)
			{
				if( i == 0  )					//==>  1ra posicion
				{
					strcpy(instrucs->comando,linea[i]);
				}

				if( i == 1  ){					//==>  2da posicion   si no es null se mete el dato en la variable
					strcpy(instrucs->parametro,linea[i]);
				}
				if( i == 2  ){					//==>  2da posicion  si no es null se mete el dato en la variable
					strcpy(instrucs->parametro2,linea[i]);
				}
			}

		list_add(listaInstrucciones,instrucs);
		free(linea);
		}
}
