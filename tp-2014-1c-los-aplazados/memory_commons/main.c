/*
 * main.c
 *
 *  Created on: 07/05/2014
 *      Author: utnso
 */


/*
 * memory.c
 *
 *  Created on: 01/05/2014
 *      Author: utnso
 */


/*
 * compactar.c
 *
 *  Created on: 01/05/2014
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h> //Biblioteca de la catedra
#include <commons/temporal.h> //Biblioteca de la catedra
#include <curses.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "pthread.h" //Biblioteca de hilos
#include "semaphore.h"//Biblioteca de semaforos
#include<commons/collections/dictionary.h>
#include<commons/collections/list.h>

typedef struct segmento t_segmento;
struct segmento{
		char Id_Programa[4];
		int inicio;
		int tamanio;
		int ubicacion_memoria;
	};

void poner_segmento_en_diccionario(t_list *,char *);

t_list *sacar_elemento_de_diccionario(char *);


void destruir_segmentos_de_programa(char *);

void crear_segmento(char *,int );



static void destruir_un_segmento(t_list*);

static  t_segmento *segmento_create(t_segmento );

int determinar_direccion_logica(t_list *);

static void romperSegmento(t_segmento *);

t_dictionary *dictionary;
char ID[4];


//-----------------------------------------------------------------------------

int main(){

	t_list *obtenido;
	t_segmento *segmento4;
	int i=0,cantidad_segmentos;



//Creo diccionario como tabla de segmentos

	dictionary=dictionary_create();

	printf("Ingrese ID clave a retirar:");
	scanf("%s",ID);
 //Creo segmentos con Key y tamanio aleatorio
	crear_segmento("12",12);
	crear_segmento("13",10);
	crear_segmento("12",11);
	crear_segmento("12",24);

	//Guardo en obtenido el elemento sacado del diccionario con la key value
	obtenido=sacar_elemento_de_diccionario(ID);
	cantidad_segmentos=list_size(obtenido);
	while(i<cantidad_segmentos){
		segmento4=list_get(obtenido,i);
		printf("%d\n",segmento4->inicio);
		i++;

	}

	destruir_segmentos_de_programa(ID);
	if(dictionary_is_empty(dictionary)){
			printf("Vacio");
	}
	else
		printf("no vacio %d",list_size(obtenido));


	return 0;

}

//------------------------------------------------------------------------------------------

//Creo nodo dinamico para almacenar segmento en dictionary


//Coloco segmento en diccionario
 void poner_segmento_en_diccionario(t_list *list,char *id) {
	dictionary_put(dictionary, id, list);
}
 //Saco elemento de diccionario

 t_list *sacar_elemento_de_diccionario(char *id){
	 t_list *aux = dictionary_get(dictionary, id);
	 return aux;
 }

//Elimino todos los segmentos de un programa, ingresando el ID del mismo, ya que es la Key
void destruir_segmentos_de_programa(char *id){

dictionary_remove_and_destroy(dictionary,id,(void*)destruir_un_segmento);
}

//Funcion para destruir los elementos de la lista
static void destruir_un_segmento(t_list *list){

	list_destroy_and_destroy_elements(list,(void*)romperSegmento);
}

//Funcion para eliminar un segmento
static void romperSegmento(t_segmento *p){
	free(p);

}


//Creo segmento,le paso como parametro el ID del programa y el tamaño del segmento
void crear_segmento(char *id,int tamanio){
	t_segmento nuevoSegmento;
	t_list  *list;
	if(dictionary_has_key(dictionary,id)){
	list=dictionary_get(dictionary,id);
	nuevoSegmento.inicio=determinar_direccion_logica(list);
	nuevoSegmento.tamanio=tamanio;
	nuevoSegmento.ubicacion_memoria=24;
	list_add(list,segmento_create(nuevoSegmento));
	poner_segmento_en_diccionario(list,id);}
		else{
			t_list *lista=list_create();
			nuevoSegmento.inicio=0;
			nuevoSegmento.tamanio=tamanio;
			nuevoSegmento.ubicacion_memoria=24;
			list_add(lista,segmento_create(nuevoSegmento));
			poner_segmento_en_diccionario(lista,id);

	}

}

static  t_segmento *segmento_create(t_segmento segmento){
	t_segmento *new = malloc( sizeof(t_segmento) );
	new->inicio = segmento.inicio;
	new->tamanio=segmento.tamanio;
	new->ubicacion_memoria=segmento.ubicacion_memoria;

	return new;
}


//Criterio a tomar para determinar el inicio de mi segmento en la tabla de segmentos
int determinar_direccion_logica(t_list *lista){
	int maximo;
	int elementos_en_lista=list_size(lista);
	t_segmento *ultimo_segmento=list_get(lista,elementos_en_lista-1);
	maximo=(ultimo_segmento->inicio + ultimo_segmento->tamanio)+1;
	return maximo;
}
