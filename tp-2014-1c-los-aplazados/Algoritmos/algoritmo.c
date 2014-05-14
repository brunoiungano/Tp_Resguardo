/*
 * algoritmo.c
 *
 *  Created on: 07/05/2014
 *      Author: utnso
 */
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
		int inicio;
		int tamanio;
		char *ubicacion_memoria;
	};

typedef struct bloqueLibre t_bloque_libre;
struct bloqueLibre{
	char *inicio;
	int tamanio;
};


void poner_segmento_en_diccionario(t_list *,char *);

t_list *sacar_elemento_de_diccionario(char *);


void destruir_segmentos_de_programa(char *);

void crear_segmento(char *,int);

void segments_destroy(t_list *);

 void destruir_un_segmento(t_segmento*);

t_segmento *segmento_create(t_segmento );

int determinar_direccion_logica(t_list *);

t_bloque_libre *bloque_create(t_bloque_libre );

int validacion_en_memoria(int ,t_list *);

void actualizar_memory(int ,t_bloque_libre **);

void imprimir_estado_memory(t_bloque_libre *);

int _es_bloque_accesible(t_bloque_libre *);

char *colocar_en_memoria_FirstFit(int);

int _bloques_mayor_a_menor(t_bloque_libre *, t_bloque_libre *);

int _bloques_ordenados_por_direccion(t_bloque_libre *, t_bloque_libre *);

int _es_mismo_bloque(t_bloque_libre*);

t_bloque_libre *buscar_bloque_en_lista();



t_dictionary *dictionary;
t_list *lista_de_bloquesLibres;

char ID[4];
int variable=1000;
void* memoria_principal;
char* manejador;
int tamanio;
char* direccion;

//-----------------------Prueba del main-----------------------------------
int main(){

//Declaramos los punteros, memoria_principal es el "gran malloc", el manejador es para recorrer dicho malloc, y el resguardo es para
//no perder el puntero a la memoria prncipal;
memoria_principal=malloc(1000);
manejador=memoria_principal;
int b=0;

t_bloque_libre bloqueLibre;
dictionary=dictionary_create();
lista_de_bloquesLibres=list_create();
list_add(lista_de_bloquesLibres,bloque_create(bloqueLibre));
t_bloque_libre *obtenido=list_get(lista_de_bloquesLibres,0);

printf("manejador%p\n",manejador);
printf("Inicio libre %p\n",obtenido->inicio);


crear_segmento("12",20);
crear_segmento("12",40);
crear_segmento("13",60);
crear_segmento("12",80);
crear_segmento("13",140);


printf("El tamaño de la lista de bloques libres es %d\n",list_size(lista_de_bloquesLibres));
//prueba unitaria para probar si los bloques libres se estan agregando
list_sort(lista_de_bloquesLibres,(void*)_bloques_ordenados_por_direccion);
while(b<list_size(lista_de_bloquesLibres)){
	t_bloque_libre* bloque=list_get(lista_de_bloquesLibres,b);
	printf("El inicio de este bloque libre es %p\n",bloque->inicio);
	printf("El tamaño de este bloque es %d\n",bloque->tamanio);
	b++;
}

return 0;
}

//------------------------------------------------------------------------------------------

void actualizar_memory(int cantidad,t_bloque_libre **puntero){
	t_bloque_libre *ptr;
	ptr=*puntero;
	ptr->inicio=(ptr->inicio+(sizeof(char)*cantidad));
	ptr->tamanio=(ptr->tamanio-cantidad);
}

void imprimir_estado_memory(t_bloque_libre *bloque){
printf("El inicio del bloque libre es : %p\n",bloque->inicio);
printf("El tamaño del bloque libre es: %d\n",bloque->tamanio);
}

int validacion_en_memoria(int cantidad,t_list *lista_de_bloques){
	t_bloque_libre *obtenido=list_get(lista_de_bloques,0);
	return (cantidad<=obtenido->tamanio);
}


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
int cantidadDeSegmentos=0;
int i=0;
t_list* lista=sacar_elemento_de_diccionario(id);
while(cantidadDeSegmentos<list_size(lista)){
	t_segmento* segmento=list_get(lista,i);
	t_bloque_libre *bloquelibre=malloc(sizeof(t_bloque_libre));
	bloquelibre->inicio=segmento->ubicacion_memoria;
	bloquelibre->tamanio=segmento->tamanio;
	list_add(lista_de_bloquesLibres,bloquelibre);
	cantidadDeSegmentos++;
	i++;
}
dictionary_remove_and_destroy(dictionary, id,(void*)list_destroy);

}

//Elimina lista de segmento
void segments_destroy(t_list *list){
	list_clean(list);
}

 void destruir_un_segmento(t_segmento *p){
	free(p);
}

//Creo segmento,le paso como parametro el ID del programa y el tamaño del segmento
void crear_segmento(char *id,int tamanio){
	t_segmento nuevoSegmento;
	t_list  *list;
	if(validacion_en_memoria(tamanio,lista_de_bloquesLibres)){

		if(dictionary_has_key(dictionary,id)){
			list=dictionary_get(dictionary,id);
			nuevoSegmento.inicio=determinar_direccion_logica(list);
			nuevoSegmento.tamanio=tamanio;
			nuevoSegmento.ubicacion_memoria=colocar_en_memoria_FirstFit(tamanio);
			direccion=nuevoSegmento.ubicacion_memoria;
			t_bloque_libre *obtenido=buscar_bloque_en_lista();
			actualizar_memory(tamanio,&obtenido);
			printf("Ubicacion%p\n",nuevoSegmento.ubicacion_memoria);
			printf("Comienzo de bloque Libre %p despues de haber guardado %d bytes\n",obtenido->inicio,tamanio);
			printf("Espacio en memoria disponible %d\n",obtenido->tamanio);
			list_add(list,segmento_create(nuevoSegmento));
			poner_segmento_en_diccionario(list,id);}
		else{
			t_list *lista=list_create();
			nuevoSegmento.inicio=0;
			nuevoSegmento.tamanio=tamanio;
			nuevoSegmento.ubicacion_memoria=colocar_en_memoria_FirstFit(tamanio);
			direccion=nuevoSegmento.ubicacion_memoria;
			t_bloque_libre *obtenido=buscar_bloque_en_lista();
			actualizar_memory(tamanio,&obtenido);
			printf("Ubicacion%p\n",nuevoSegmento.ubicacion_memoria);
			printf("Comienzo de bloque Libre %p despues de haber guardado %d bytes\n",obtenido->inicio,tamanio);
			printf("Espacio en memoria disponible %d\n",obtenido->tamanio);
			list_add(lista,segmento_create(nuevoSegmento));
			poner_segmento_en_diccionario(lista,id);
		}
	}
	else{
		printf("no hay memoria disponible");

	}

}

//Creo nodo dinamico para almacenar segmento en dictionary
 t_segmento *segmento_create(t_segmento segmento){
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

t_bloque_libre *bloque_create(t_bloque_libre bloque){
	t_bloque_libre *new = malloc( sizeof(t_bloque_libre) );
	new->inicio =manejador;
	new->tamanio=variable;
	return new;
}

//Retorno la direccion de memoria segun el Algoritmo First Fit
char *colocar_en_memoria_FirstFit(tamanio){
list_sort(lista_de_bloquesLibres,(void*)_bloques_ordenados_por_direccion);
t_bloque_libre *obtenido=list_find(lista_de_bloquesLibres,(void*)_es_bloque_accesible);
return obtenido->inicio;
}

//Retorno la direccion de memoria segun el Algoritmo Worst Fit
char *colocar_en_memoria_WorstFit(tamanio){
	list_sort(lista_de_bloquesLibres,(void*)_bloques_mayor_a_menor);
	t_bloque_libre *obtenido=list_find(lista_de_bloquesLibres,(void*)_es_bloque_accesible);
	return obtenido->inicio;
}
//Condicion para el algoritmo Worst Fit, necesito ordenar bloques de mayor a menor
int _bloques_mayor_a_menor(t_bloque_libre *libre, t_bloque_libre *libre2) {
		return libre->tamanio > libre2->tamanio;
	}

//Condicin es bloque acesible, evalua si el tamaño del segmento es menor que el tamaño libre
int _es_bloque_accesible(t_bloque_libre *bloque){
	return (tamanio<=bloque->tamanio);
}

//Condicion para ordenar la lista de bloques libres por direccion de memoria
int _bloques_ordenados_por_direccion(t_bloque_libre *libre, t_bloque_libre *libre1) {
		return libre->inicio < libre1->inicio;
	}


//Hago esta funcion para que el actualizar memory solo me actualice el bloque de memoria libre correspondiente
t_bloque_libre *buscar_bloque_en_lista(){
	t_bloque_libre *bloqueFree=list_find(lista_de_bloquesLibres,(void*)_es_mismo_bloque);
	return bloqueFree;
	}

//Condicion de t_bloque_libre *buscar_bloque_en_lista() para que me encuentre el bloque con dicha direccion
int _es_mismo_bloque(t_bloque_libre* bloque){
	return bloque->inicio==direccion;

}




