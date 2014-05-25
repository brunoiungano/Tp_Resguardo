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
#include "utiles.h"

#define BUFF_SIZE 1000 //Tamaño del buffer de datos que se transfieren entre procesos

#define ENVIO_PROGRAMA 1
#define PROGRAMA_TERMINADO 2
#define PROGRAMA_BLOQUEADO 3
#define PROGRAMA_ABORTADO 4
#define RESERVAR_MEMORIA_PCB 5




typedef struct segmento t_segmento;
struct segmento{
		char *inicio;
		int tamanio;
		char *ubicacion_memoria;
	};

typedef struct bloqueLibre t_bloque_libre;
struct bloqueLibre{
	char *inicio;
	int tamanio;
};

typedef struct idEnLista t_id;
struct idEnLista{
	char id[6];
};


void poner_segmento_en_diccionario(t_list *,char *);

t_list *sacar_elemento_de_diccionario(char *);


void destruir_segmentos_de_programa(char *);

int crear_segmento(char *,int);

void segments_destroy(t_list *);

 void destruir_un_segmento(t_segmento*);

t_segmento *segmento_create(t_segmento );


t_bloque_libre *bloque_create(t_bloque_libre );

int validacion_en_memoria(int ,t_list *);

void actualizar_memory(int ,t_bloque_libre **);

void imprimir_estado_memory(t_bloque_libre *);


char *colocar_en_memoria_FirstFit(int);

char *colocar_en_memoria_WorstFit(int);

int _bloques_mayor_a_menor(t_bloque_libre *, t_bloque_libre *);

int _bloques_ordenados_por_direccion(t_bloque_libre *, t_bloque_libre *);

t_bloque_libre *buscar_bloque_en_lista();

void levantarArchivoKernel();

t_id *idEnList_create(t_id);

t_segmento* buscar_id_segun_posicion();

void _agregar_a_lista(char* );

void hago_lista_con_ids();

void compactar_memoria();

void actualizar_memory_compactada(int,t_bloque_libre **);

void elimino_bloque(t_bloque_libre *bloque);

void eliminar_bloque_libre();

void actualizar_bloques_libres();

void eliminar_bloques_libres_vacios();

int _es_bloque_vacio(t_bloque_libre *);

//void actualizar_bloques_libres1(); //prueba

//void juntar_bloques(t_bloque_libre*);//prueba

//int _es_bloque_juntable(t_bloque_libre*);





t_dictionary *dictionary;
t_list *lista_de_bloquesLibres;
t_list* lista_de_id;

//Variables globales, TENER CUIDADO CON SU MANEJO

int variable=1000;
void* memoria_principal;
char* manejador;
char* direccion;
char *string, algoritmo_de_ubicacion[20];

char* inicio_en_tabla;
//char* inicio_bloque;
//int tamanio_bloque_libre;

//-----------------------Prueba del main-----------------------------------
int main(){

//Declaramos los punteros, memoria_principal es el "gran malloc", el manejador es para recorrer dicho malloc, y el resguardo es para
//no perder el puntero a la memoria prncipal;
memoria_principal=malloc(100);
manejador=memoria_principal;
inicio_en_tabla=manejador;
int b=0;
int z=0;
int pruebita=0;


t_bloque_libre bloqueLibre;
dictionary=dictionary_create();
lista_de_bloquesLibres=list_create();
lista_de_id=list_create();
list_add(lista_de_bloquesLibres,bloque_create(bloqueLibre));
t_bloque_libre *obtenido=list_get(lista_de_bloquesLibres,0);

printf("manejador%p\n",manejador);
printf("Inicio libre %p\n",obtenido->inicio);


crear_segmento("12",10);
crear_segmento("13",30);
crear_segmento("12",20);
crear_segmento("12",30);
crear_segmento("13",5);
crear_segmento("10",15);

printf("Se destruye segmento 12\n");
destruir_segmentos_de_programa("12");
crear_segmento("12",10);
crear_segmento("12",40); /*Cuando creo este segmento se me genera el problema del segmentation fault,
ahora si no pongo este crear segmento el programa hace lo que quiero*/
crear_segmento("11",80);






hago_lista_con_ids();
int cantidad_libertad=list_size(lista_de_id);
while(pruebita<cantidad_libertad){
	t_id *ident=list_get(lista_de_id,pruebita);
	printf("Identificados %s\n",ident->id);
	pruebita++;
}

eliminar_bloques_libres_vacios();
printf("ANTES DE REALIZAR LA COMPACTACION, LA MEMORIA ESTA ASI \n");
printf("\n");
printf("\n");
printf("\n");
printf("El tamaño de la lista de bloques libres es %d\n",list_size(lista_de_bloquesLibres));
//prueba unitaria para probar si los bloques libres se estan agregando
list_sort(lista_de_bloquesLibres,(void*)_bloques_ordenados_por_direccion);
while(z<list_size(lista_de_bloquesLibres)){
	t_bloque_libre* bloque=list_get(lista_de_bloquesLibres,z);
	printf("El inicio de este bloque libre es %p el fin es %p\n",bloque->inicio,bloque->inicio+((bloque->tamanio)-1));
	printf("El tamaño de este bloque es %d\n",bloque->tamanio);
	z++;
}
printf("DESPUES DE REALIZAR LA COMPACTACION, LA MEMORIA ESTA ASI \n");
compactar_memoria();
printf("\n");
printf("\n");
printf("\n");

printf("El tamaño de la lista de bloques libres es %d\n",list_size(lista_de_bloquesLibres));
//prueba unitaria para probar si los bloques libres se estan agregando
list_sort(lista_de_bloquesLibres,(void*)_bloques_ordenados_por_direccion);
while(b<list_size(lista_de_bloquesLibres)){
	t_bloque_libre* bloque=list_get(lista_de_bloquesLibres,b);
	printf("El inicio de este bloque libre es %p el fin es %p\n",bloque->inicio,bloque->inicio+bloque->tamanio);
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

void actualizar_memory_compactada(int cantidad,t_bloque_libre **puntero){
	t_bloque_libre *ptr;
	ptr=*puntero;
	ptr->inicio=(ptr->inicio+(sizeof(char)*cantidad));
}

void imprimir_estado_memory(t_bloque_libre *bloque){
printf("El inicio del bloque libre es : %p\n",bloque->inicio);
printf("El tamaño del bloque libre es: %d\n",bloque->tamanio);
}

int validacion_en_memoria(int cantidad,t_list *lista_de_bloques){
	int i=0;
	int memoria=0;
	int cantidad_de_bloques=list_size(lista_de_bloques);
	while(i<cantidad_de_bloques){
		t_bloque_libre *obtenido=list_get(lista_de_bloques,i);
		memoria=memoria+obtenido->tamanio;
		i++;
	}
	return (cantidad<=memoria);
}


//Coloco segmento en diccionario
 void poner_segmento_en_diccionario(t_list *list,char *id) {
	dictionary_put(dictionary, id, list);
}
 //Saco elemento de diccionario

 t_list *sacar_elemento_de_diccionario(char *id){

	 t_list * aux = dictionary_get(dictionary, id);
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
int crear_segmento(char *id,int tamanio){
	t_segmento nuevoSegmento;
	t_list  *list;
	if(validacion_en_memoria(tamanio,lista_de_bloquesLibres)){

		if(dictionary_has_key(dictionary,id)){
			list=dictionary_get(dictionary,id);
			nuevoSegmento.inicio=inicio_en_tabla;
			nuevoSegmento.tamanio=tamanio;
			nuevoSegmento.ubicacion_memoria=colocar_en_memoria_FirstFit(tamanio);
			eliminar_bloques_libres_vacios();
			t_bloque_libre *obtenido=buscar_bloque_en_lista(nuevoSegmento.ubicacion_memoria);
			actualizar_memory(tamanio,&obtenido);
			printf("El inicio del bloque es: %p \n",nuevoSegmento.inicio);
			printf("Ubicacion del segmento %p\n",nuevoSegmento.ubicacion_memoria);
			printf("Comienzo de bloque Libre %p despues de haber guardado %d bytes\n",obtenido->inicio,tamanio);
			printf("Espacio en memoria disponible %d\n",obtenido->tamanio);
			list_add(list,segmento_create(nuevoSegmento));}
		else{
			t_list *lista=list_create();
			nuevoSegmento.inicio=inicio_en_tabla;
			nuevoSegmento.tamanio=tamanio;
			nuevoSegmento.ubicacion_memoria=colocar_en_memoria_FirstFit(tamanio);
			t_bloque_libre *obtenido=buscar_bloque_en_lista(nuevoSegmento.ubicacion_memoria);
			eliminar_bloques_libres_vacios();
			actualizar_memory(tamanio,&obtenido);
			printf("El inicio del bloque es: %p \n",nuevoSegmento.inicio);
			printf("Ubicacion del segmento %p\n",nuevoSegmento.ubicacion_memoria);
			printf("Comienzo de bloque Libre %p despues de haber guardado %d bytes\n",obtenido->inicio,tamanio);
			printf("Espacio en memoria disponible %d\n",obtenido->tamanio);
			list_add(lista,segmento_create(nuevoSegmento));
			poner_segmento_en_diccionario(lista,id);

		}
		return 1 ;
	}
	else
	printf("No hay memoria disponible\n");
		{return 0;}

		}



//Creo nodo dinamico para almacenar segmento en dictionary
 t_segmento *segmento_create(t_segmento segmento){
	t_segmento *new = malloc( sizeof(t_segmento) );
	new->inicio = segmento.inicio;
	new->tamanio=segmento.tamanio;
	new->ubicacion_memoria=segmento.ubicacion_memoria;

	return new;
}

t_bloque_libre *bloque_create(t_bloque_libre bloque){
	t_bloque_libre *new = malloc( sizeof(t_bloque_libre) );
	new->inicio =manejador;
	new->tamanio=variable;
	return new;
}

t_id *idEnList_create(t_id id){
	t_id *new = malloc( sizeof(t_id) );
	strcpy(new->id,id.id);
	return new;
}

//Retorno la direccion de memoria segun el Algoritmo First Fit
char *colocar_en_memoria_FirstFit(int tamanio1){
list_sort(lista_de_bloquesLibres,(void*)_bloques_ordenados_por_direccion);

	int _es_bloque_accesible(t_bloque_libre *bloque){ //Condicin es bloque acesible, evalua si el tamaño del segmento es menor que el tamaño libre
	return (tamanio1<=bloque->tamanio);
	}
t_bloque_libre *obtenido=list_find(lista_de_bloquesLibres,(void*)_es_bloque_accesible);
return obtenido->inicio;
}

//Retorno la direccion de memoria segun el Algoritmo Worst Fit
char *colocar_en_memoria_WorstFit(int tamanio1){
	list_sort(lista_de_bloquesLibres,(void*)_bloques_mayor_a_menor);
	int _es_bloque_accesible(t_bloque_libre *bloque){ //Condicin es bloque acesible, evalua si el tamaño del segmento es menor que el tamaño libre
		return (tamanio1<=bloque->tamanio);
	}
	t_bloque_libre *obtenido=list_find(lista_de_bloquesLibres,(void*)_es_bloque_accesible);
	return obtenido->inicio;
}
//Condicion para el algoritmo Worst Fit, necesito ordenar bloques de mayor a menor
int _bloques_mayor_a_menor(t_bloque_libre *libre, t_bloque_libre *libre2) {
		return libre->tamanio > libre2->tamanio;
	}




//Condicion para ordenar la lista de bloques libres por direccion de memoria
int _bloques_ordenados_por_direccion(t_bloque_libre *libre, t_bloque_libre *libre1) {
		return libre->inicio < libre1->inicio;
	}


//Hago esta funcion para que el actualizar memory solo me actualice el bloque de memoria libre correspondiente
t_bloque_libre *buscar_bloque_en_lista(char* direccion1){
	int _es_mismo_bloque(t_bloque_libre* bloque){
		return bloque->inicio==direccion1;

	}
	t_bloque_libre *bloqueFree=list_find(lista_de_bloquesLibres,(void*)_es_mismo_bloque);
	return bloqueFree;
	}

//Levanta el archivo de configuracion
void levantarArchivoKernel(){
	t_config *archivoConf;
	archivoConf=config_create(direccion);
	string=config_get_string_value(archivoConf,"algoritmo");
	strcpy(algoritmo_de_ubicacion[20],string);
	config_destroy(archivoConf);
}
//----------COMPACTACION--------------------
//Compactar la memoria principal
void compactar_memoria(){
	char*puntero=memoria_principal;
	int contador=0;
	int flag=0;
	if(list_size(lista_de_bloquesLibres)>1){
		actualizar_bloques_libres();}//actualizo bloques, para juntar aquellos que sus direcciones son contiguas

	printf("tamaño de bloques %d\n",list_size(lista_de_bloquesLibres));

	while(flag==0){
		t_bloque_libre *libre=list_get(lista_de_bloquesLibres,contador);
		printf("La direccion del bloque libre %p \n",libre->inicio);
		puntero=(libre->inicio+libre->tamanio);
		printf("La suma del tamaño %d del bloque libre y el puntero es %p \n",libre->tamanio,puntero);
		if(buscar_id_segun_posicion(puntero)!=NULL){//pregunto si la direccion de memoria la encuentra
			t_segmento *unSegmento=buscar_id_segun_posicion(puntero);
			printf("El segmento que sigue a ese bloque esta ubicado en la direccion%p\n",unSegmento->ubicacion_memoria);
			char*puntero1=malloc(unSegmento->tamanio);
			puntero1=unSegmento->ubicacion_memoria;

		if(libre->tamanio >= unSegmento->tamanio){ //Pregunto si el tamaño del bloque libre es mayor a la del segmento
		memcpy(libre->inicio,puntero1,unSegmento->tamanio);
		unSegmento->ubicacion_memoria=libre->inicio;
		libre->inicio=libre->inicio+unSegmento->tamanio;
		t_bloque_libre *libre2=list_get(lista_de_bloquesLibres,contador+1);

			if(libre->inicio+libre->tamanio==libre2->inicio){//pregunto si hay dos bloques contiguos que se pueden compactar
			libre2->inicio=libre->inicio;
			libre2->tamanio=libre->tamanio+libre2->tamanio;
			eliminar_bloque_libre(contador);
			}//Aclaro que contador solo se aumenta en alguna ocasiones. porque?
						//Porque sino me va a saltear un bloque libre al que le siguen mas de un segmento
	printf("Despues de la compactacion la ubicacion del segmento es: %p\n",unSegmento->ubicacion_memoria);
		}
		else{
			char* aux1=malloc(unSegmento->tamanio);
			aux1=puntero1;
			memcpy(aux1,puntero1,unSegmento->tamanio);
			memcpy(libre->inicio,aux1,unSegmento->tamanio);
			unSegmento->ubicacion_memoria=libre->inicio;
			libre->inicio=libre->inicio+unSegmento->tamanio;
			t_bloque_libre *libre2=list_get(lista_de_bloquesLibres,contador+1);
			printf("tamaño de bloques %d\n",list_size(lista_de_bloquesLibres));
				if(libre->inicio+libre->tamanio==libre2->inicio){
					libre2->inicio=libre->inicio;
					libre2->tamanio=libre->tamanio+libre2->tamanio;
					eliminar_bloque_libre(contador);
					printf("tamaño de bloques %d\n",list_size(lista_de_bloquesLibres));

					}//Aclaro que contador solo se aumenta en alguna ocasiones. porque?
				//Porque sino me va a saltear un bloque libre al que le siguen mas de un segmento
		printf("Despues de la compactacion la ubicacion del segmento es: %p\n",unSegmento->ubicacion_memoria);
	}
	}
	else {printf("Ya se realizo la compactacion\n");
	flag=1;
	printf("Despues de la compactacion el bloque tiene direccion %p \n",libre->inicio);}
	}
}

//---------FIN COMPACTACION-------------

void eliminar_bloques_libres_vacios(){
	int count=0;
	int elementos=list_size(lista_de_bloquesLibres);
	while(count<elementos){
		t_bloque_libre* bloque=list_get(lista_de_bloquesLibres,count);
		if(bloque->tamanio==0){
			eliminar_bloque_libre(count);
			elementos=list_size(lista_de_bloquesLibres);}
		else count++;
			}
}

int _es_bloque_vacio(t_bloque_libre *bloque){
	return (bloque->tamanio==0);
}
void eliminar_bloque_libre(int contador){
	list_remove_and_destroy_element(lista_de_bloquesLibres, contador, (void*)elimino_bloque);

}


void elimino_bloque(t_bloque_libre *bloque){
	free(bloque);
}

//Creo una lista compuesta por todos los Id's del diccionario
void hago_lista_con_ids(){
dictionary_iterator(dictionary, (void*)_agregar_a_lista);}

void _agregar_a_lista(char* key){
		list_add(lista_de_id,key);
}

t_segmento *buscar_id_segun_posicion(char*puntero){
	int i=0;
	int encontrado=0;
	t_segmento* segmento;
	int cant=list_size(lista_de_id);
	while(encontrado==0 && i<cant ){
		t_id* identificador=list_get(lista_de_id,i);
		t_list* lista=sacar_elemento_de_diccionario(identificador->id);
		int es_puntero_de_segmento(t_segmento *segmento){
			return (segmento->ubicacion_memoria==puntero);
		}
		segmento=list_find(lista,(void*)es_puntero_de_segmento);
		if(segmento!=NULL){
				encontrado=1;
			}
		else i++;
		}
	if(encontrado!=0)
		return segmento;
	else return NULL;
}


//Se actualizan los bloques libres, para evitar que se busquen direcciones que estan libres
void actualizar_bloques_libres(){
	int contador=0;
	int cantidad=list_size(lista_de_bloquesLibres);
	while(contador<cantidad && cantidad>1){
		if((cantidad-contador)==1){
				t_bloque_libre *libre1=list_get(lista_de_bloquesLibres,contador-1);
				t_bloque_libre *libre2=list_get(lista_de_bloquesLibres,contador);
				if(libre1->inicio+libre1->tamanio==libre2->inicio){
								libre2->inicio=libre1->inicio;
								libre2->tamanio=libre1->tamanio+libre2->tamanio;
								eliminar_bloque_libre(contador);
								cantidad=list_size(lista_de_bloquesLibres);
								contador=0;}
									else contador++;}
				else
					{t_bloque_libre *libre1=list_get(lista_de_bloquesLibres,contador);
					t_bloque_libre *libre2=list_get(lista_de_bloquesLibres,contador+1);
					if(libre1->inicio+libre1->tamanio==libre2->inicio){
									libre2->inicio=libre1->inicio;
									libre2->tamanio=libre1->tamanio+libre2->tamanio;
									eliminar_bloque_libre(contador);
									cantidad=list_size(lista_de_bloquesLibres);
									contador=0;}
										else contador++;}

	}
}


/*
void actualizar_bloques_libres1(){
	list_iterate(lista_de_bloquesLibres,(void*)juntar_bloques);
}

void juntar_bloques(t_bloque_libre* bloque){
	inicio_bloque=bloque->inicio;
	tamanio_bloque_libre=bloque->tamanio;
	t_bloque_libre* obtenido=list_find(lista_de_bloquesLibres,(void*)_es_bloque_juntable);
	if(obtenido!=NULL){
		obtenido->inicio=bloque->inicio;
		obtenido->tamanio=obtenido->tamanio+bloque->tamanio;
		free(bloque);

	}
}

int _es_bloque_juntable(t_bloque_libre* bloque){
	return ((inicio_bloque+tamanio_bloque_libre)==bloque->inicio);
}
*/


