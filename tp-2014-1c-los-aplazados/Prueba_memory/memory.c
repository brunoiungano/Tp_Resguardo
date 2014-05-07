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
		t_segmento *siguiente;

	};

void poner_segmento_en_diccionario(t_segmento *,char *);

void MuestraLista(t_segmento *);

t_segmento *sacar_elemento_de_diccionario(char *);

void InsertaLista( t_segmento **, t_segmento);

t_segmento *CrearLista(t_segmento);

void destruir_segmentos_de_programa(char *);

void crear_segmento(char *,int );

static void segment_destroy(t_list *);

t_dictionary *dictionary;
char ID[4];


//-----------------------------------------------------------------------------

int main(){

	t_segmento *obtenido;
	t_segmento segmento1,segmento2,segmento3,segmento4;
	t_segmento *lista;


//Creo diccionario como tabla de segmentos

	dictionary=dictionary_create();

//Creo segmentos de  prueba
	strcpy(segmento1.Id_Programa,"12");
	segmento1.inicio=0;
	segmento1.tamanio=5;
	segmento1.ubicacion_memoria=13;
	strcpy(segmento2.Id_Programa,"12");
	segmento2.inicio=1;
	segmento2.tamanio=6;
	segmento2.ubicacion_memoria=14;
	strcpy(segmento3.Id_Programa,"13");
	segmento3.inicio=5;
	segmento3.tamanio=62;
	segmento3.ubicacion_memoria=144;
	strcpy(segmento4.Id_Programa,"12");
	segmento4.inicio=5;
	segmento4.tamanio=62;
	segmento4.ubicacion_memoria=144;
	printf("Ingrese ID clave a retirar:");
	scanf("%s",ID);

	//Creo lista con los distintos segmentos de cada programa
	lista=CrearLista(segmento1);
	InsertaLista(&lista,segmento2);
	poner_segmento_en_diccionario(lista,segmento2.Id_Programa);//guardo lista en diccionario con Id_Programa como key value
	lista=CrearLista(segmento3);
	poner_segmento_en_diccionario(lista,segmento3.Id_Programa);
	crear_segmento("12",12);//Creo un segmento del programa con ID 12 y tamaño 12;

	//Guardo en obtenido el elemento sacado del diccionario con la key value
	obtenido=sacar_elemento_de_diccionario(ID);
	MuestraLista(obtenido);
	destruir_segmentos_de_programa(ID);//me destruye la lista de segmentos del ID ingresado
	printf("%d",dictionary_size(dictionary));
	return 0;

}

//------------------------------------------------------------------------------------------

//Creo nodo dinamico para almacenar segmento en dictionary


//Coloco segmento en diccionario
 void poner_segmento_en_diccionario(t_segmento*list,char *id) {
	t_segmento *p1 = list;
	dictionary_put(dictionary, id, p1);
}
 //Saco elemento de diccionario

 t_segmento *sacar_elemento_de_diccionario(char *id){
	 t_segmento *aux = dictionary_get(dictionary, id);
	 return aux;
 }

 //Muestro lista, en este caso muestro tamanio, pero lo podemos hacer variar
 void MuestraLista(t_segmento *inicial){
	 if(inicial!=NULL){
	 printf("El tamanio del segmento es %d\n",inicial->tamanio);
	 MuestraLista(inicial->siguiente);
	 }
 }

//Creo la lista de segmentos que se guardara en cada nodo Key value
 t_segmento *CrearLista(t_segmento segmento) {  /* Crea la lista, claro */
   t_segmento* r;        /* Variable auxiliar */
   r = (t_segmento*)
     malloc (sizeof(t_segmento)); /* Reserva memoria */
   r->inicio = segmento.inicio;            /* Guarda el valor */
   r->tamanio=segmento.tamanio;
   r->ubicacion_memoria=segmento.ubicacion_memoria;
   r->siguiente = NULL;           /* No hay siguiente */
   return r;               /* Crea el struct lista* */
 }

 //Inserta segmento en la lista correspondiente
 void InsertaLista( t_segmento **lista, t_segmento segmento) {
  t_segmento* r;        /* Variable auxiliar, para reservar */
   t_segmento* actual;   /* Otra auxiliar, para recorrer */

   actual = *lista;
   if (actual)                           /*  Si hay lista */
     if (actual->inicio< segmento.inicio)         /*    y todavía no es su sitio */
       InsertaLista(&actual->siguiente,segmento); /*   mira la siguiente posición */
     else {                     /* Si hay lista pero ya es su sitio */
       r = CrearLista(segmento);   /* guarda el dato */
       r->siguiente = actual;         /* pone la lista a continuac. */
       *lista = r;              /* Y hace que comience en el nuevo dato */
     }
   else {              /* Si no hay lista, hay que crearla */
     r = CrearLista(segmento);
     *lista = r;       /* y hay que indicar donde debe comenzar */
    }
 }

//Elimino todos los segmentos de un programa
void destruir_segmentos_de_programa(char *id){
dictionary_remove_and_destroy(dictionary, id, (void*)segment_destroy);
}

//Elimina lista de segmento
static void segment_destroy(t_list *ptr){
list_destroy(ptr);
}

//Creo segmento,le paso como parametro el ID del programa y el tamaño del segmento
void crear_segmento(char *id,int tamanio){
	t_segmento nuevoSegmento;
	t_segmento *list;
	nuevoSegmento.inicio=0;
	nuevoSegmento.tamanio=tamanio;
	nuevoSegmento.ubicacion_memoria=14;
	list=dictionary_remove(dictionary,id);
	InsertaLista(&list,nuevoSegmento);
	poner_segmento_en_diccionario(list,id);

}


