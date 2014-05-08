/*
 * algoritmo.c
 *
 *  Created on: 07/05/2014
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
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>



typedef struct segmento t_segmento;
struct segmento{
		char Id_Programa[4];
		int inicio;
		int tamanio;
		int ubicacion_memoria;
	};
typedef struct bloqueLibre t_bloque_libre;
struct bloqueLibre{
	int inicio;
	int tamanio;
};

void poner_segmento_en_diccionario(t_list *,char *);

t_list *sacar_elemento_de_diccionario(char *);


void destruir_segmentos_de_programa(char *);

void crear_segmento(char *,int );

static void segments_destroy(t_list *);

static void destruir_un_segmento(t_segmento*);

static  t_segmento *segmento_create(t_segmento );

int determinar_direccion_logica(t_list *);
static  t_bloque_libre *bloque_create(t_bloque_libre );

t_dictionary *dictionary;
char ID[4];
int variable=100;


//-----------------------------------------------------------------------------
int main(){
int *puntero,variable;
void* memoria_principal=malloc(100);
int* mallocInt=(int*)memoria_principal;
int cantidad,i=0;
t_bloque_libre bloqueLibre;
t_list *lista_de_bloquesLibres=list_create();
list_add(lista_de_bloquesLibres,bloque_create(bloqueLibre));
printf("Ponga bytes para almacenar:");
scanf("%d",&cantidad);
t_bloque_libre *obtenido=list_get(lista_de_bloquesLibres,0);
obtenido->inicio=(obtenido->inicio+cantidad);
obtenido->tamanio=(obtenido->tamanio-cantidad);
printf("%d\n",obtenido->inicio);
printf("%d\n",obtenido->tamanio);
mallocInt[i]=cantidad;
printf("%p",&mallocInt[i]);




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

//Elimino todos los segmentos de un programa
void destruir_segmentos_de_programa(char *id){

dictionary_remove_and_destroy(dictionary, id,(void*)list_destroy);
}

//Elimina lista de segmento
static void segments_destroy(t_list *list){
	list_clean(list);
}

static void destruir_un_segmento(t_segmento *p){
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

static  t_bloque_libre *bloque_create(t_bloque_libre bloque){
	t_bloque_libre *new = malloc( sizeof(t_bloque_libre) );
	new->inicio = 0;
	new->tamanio=variable;
	return new;
}

void guardoBytes(int cantidad,t_list *lista){
	t_bloque_libre *obtenido=list_get(lista,0);
	obtenido->inicio=(obtenido->inicio+cantidad);
	obtenido->tamanio=(obtenido->tamanio-cantidad);

}
