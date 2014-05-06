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
t_segmento *sacar_elemento_de_diccionario();
void InsertaLista( t_segmento **, int,int,int);
t_segmento *CrearLista(int ,int ,int );
void destroy_All_segment(char *);
static void segment_destroy(t_segmento *);

t_dictionary *dictionary;
char ID[4];


//-----------------------------------------------------------------------------

int main(){

	t_segmento *obtenido;
	t_segmento segmento1,segmento2,segmento3;
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
	printf("Ingrese ID clave a retirar:");
	scanf("%s",ID);

	//Creo lista con los distintos segmentos de cada programa
	lista=CrearLista(segmento1.inicio,segmento1.tamanio,segmento1.ubicacion_memoria);
	InsertaLista(&lista,segmento2.inicio,segmento2.tamanio,segmento2.ubicacion_memoria);
	poner_segmento_en_diccionario(lista,segmento2.Id_Programa);//guardo lista en diccionario con Id_Programa como key value
	lista=CrearLista(segmento3.inicio,segmento3.tamanio,segmento3.ubicacion_memoria);
	poner_segmento_en_diccionario(lista,segmento3.Id_Programa);

	//Guardo en obtenido el elemento sacado del diccionario con la key value
	obtenido=sacar_elemento_de_diccionario();
	MuestraLista(obtenido);
	destroy_All_segment(ID);
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
 t_segmento *sacar_elemento_de_diccionario(){
	 t_segmento *aux = dictionary_get(dictionary, ID);
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
 t_segmento *CrearLista(int inicio,int tamanio,int direccion) {  /* Crea la lista, claro */
   t_segmento* r;        /* Variable auxiliar */
   r = (t_segmento*)
     malloc (sizeof(t_segmento)); /* Reserva memoria */
   r->inicio = inicio;            /* Guarda el valor */
   r->tamanio=tamanio;
   r->ubicacion_memoria=direccion;
   r->siguiente = NULL;           /* No hay siguiente */
   return r;               /* Crea el struct lista* */
 }

 //Inserta segmento en la lista correspondiente
 void InsertaLista( t_segmento **lista, int inicio,int tamanio,int direccion) {
  t_segmento* r;        /* Variable auxiliar, para reservar */
   t_segmento* actual;   /* Otra auxiliar, para recorrer */

   actual = *lista;
   if (actual)                           /*  Si hay lista */
     if (actual->inicio< inicio)         /*    y todavía no es su sitio */
       InsertaLista(&actual->siguiente,inicio,tamanio,direccion); /*   mira la siguiente posición */
     else {                     /* Si hay lista pero ya es su sitio */
       r = CrearLista(inicio,tamanio,direccion);   /* guarda el dato */
       r->siguiente = actual;         /* pone la lista a continuac. */
       *lista = r;              /* Y hace que comience en el nuevo dato */
     }
   else {              /* Si no hay lista, hay que crearla */
     r = CrearLista(inicio,tamanio,direccion);
     *lista = r;       /* y hay que indicar donde debe comenzar */
    }
 }
//Elimino todos los segmentos
void destroy_All_segment(char *id){
dictionary_remove_and_destroy(dictionary, id, (void*) segment_destroy);
}

//Elimina segmento individual
static void segment_destroy(t_segmento *ptr){
while(ptr!=NULL){
	free(ptr);
	ptr++;
}
}

