/*
 * umv.c
 *
 *  Created on: 24/04/2014
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
#define DESTRUIR_SEGMENTO 6



typedef struct segmento t_segmento;
struct segmento{
		char* inicio;
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

int crear_segmento(char *,int); //Se modifico para que devuelva int(1 si entro y 0 si no)

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

char *colocar_en_memoria_WorstFit(int);

int _bloques_mayor_a_menor(t_bloque_libre *, t_bloque_libre *);

int _bloques_ordenados_por_direccion(t_bloque_libre *, t_bloque_libre *);

int _es_mismo_bloque(t_bloque_libre*);

t_bloque_libre *buscar_bloque_en_lista();

t_id *idEnList_create(t_id);

t_segmento* buscar_id_segun_posicion();

void _agregar_a_lista(char* );

int es_puntero_de_segmento(t_segmento *);

void hago_lista_con_ids();

void compactar_memoria();

void actualizar_memory_compactada(int,t_bloque_libre **);

void elimino_bloque(t_bloque_libre *bloque);

void eliminar_bloque_libre();

void actualizar_bloques_libres();

void levantarArchivoUMV();

void recibirConexiones();

t_dictionary *dictionary;
t_list *lista_de_bloquesLibres;
t_list* lista_de_id;

char ID[4];
void* memoria_principal;
char* manejador;
int tamanio;
char* direccion1;
char algoritmo_de_ubicacion[20];
char*puntero;
char direccion[100], ipUMV[20], *string;
int puertoUMV,capacidad_de_memoria;
Msg mensaje;
Sgm segmento;
char* inicio_en_tabla;



//-----------------------Prueba del main-----------------------------------
int main(int argc, char* argv[]){
strcpy(direccion,argv[2]);
string=(char*)malloc(sizeof(char)*100);
levantarArchivoUMV();

//Declaramos los punteros, memoria_principal es el "gran malloc", el manejador es para recorrer dicho malloc, y el resguardo es para
//no perder el puntero a la memoria prncipal;
memoria_principal=malloc(capacidad_de_memoria);
manejador=memoria_principal;

t_bloque_libre bloqueLibre;
dictionary=dictionary_create();
lista_de_bloquesLibres=list_create();
lista_de_id=list_create();
list_add(lista_de_bloquesLibres,bloque_create(bloqueLibre));
t_bloque_libre *obtenido=list_get(lista_de_bloquesLibres,0);

printf("manejador%p\n",manejador);
printf("Inicio libre %p\n",obtenido->inicio);

recibirConexiones();


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
int crear_segmento(char *id,int tamanio){
	t_segmento nuevoSegmento;
	t_list  *list;
	if(validacion_en_memoria(tamanio,lista_de_bloquesLibres)){

		if(dictionary_has_key(dictionary,id)){
			list=dictionary_get(dictionary,id);
			nuevoSegmento.inicio=inicio_en_tabla;
			nuevoSegmento.tamanio=tamanio;
			nuevoSegmento.ubicacion_memoria=colocar_en_memoria_FirstFit(tamanio);
			direccion1=nuevoSegmento.ubicacion_memoria;
			t_bloque_libre *obtenido=buscar_bloque_en_lista();
			actualizar_memory(tamanio,&obtenido);
			printf("Ubicacion del segmento %p\n",nuevoSegmento.ubicacion_memoria);
			printf("Comienzo de bloque Libre %p despues de haber guardado %d bytes\n",obtenido->inicio,tamanio);
			printf("Espacio en memoria disponible %d\n",obtenido->tamanio);
			list_add(list,segmento_create(nuevoSegmento));}
		else{
			t_list *lista=list_create();
			nuevoSegmento.inicio=inicio_en_tabla;
			nuevoSegmento.tamanio=tamanio;
			nuevoSegmento.ubicacion_memoria=colocar_en_memoria_FirstFit(tamanio);
			direccion1=nuevoSegmento.ubicacion_memoria;
			t_bloque_libre *obtenido=buscar_bloque_en_lista();
			actualizar_memory(tamanio,&obtenido);
			printf("Ubicacion del segmento %p\n",nuevoSegmento.ubicacion_memoria);
			printf("Comienzo de bloque Libre %p despues de haber guardado %d bytes\n",obtenido->inicio,tamanio);
			printf("Espacio en memoria disponible %d\n",obtenido->tamanio);
			list_add(lista,segmento_create(nuevoSegmento));
			poner_segmento_en_diccionario(lista,id);

		}
    return 1;
	}
	else
		printf("No hay memoria disponible\n");
	return 0;

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
	new->tamanio=capacidad_de_memoria;
	return new;
}

t_id *idEnList_create(t_id id){
	t_id *new = malloc( sizeof(t_id) );
	strcpy(new->id,id.id);
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

//Levanta el archivo de configuracion
void levantarArchivoKernel(){
	t_config *archivoConf;
	archivoConf=config_create(direccion);
	string=config_get_string_value(archivoConf,"algoritmo");
	strcpy(algoritmo_de_ubicacion,string);
	config_destroy(archivoConf);
}
//----------COMPACTACION--------------------
//Compactar la memoria principal
void compactar_memoria(){
	puntero=memoria_principal;
	int contador=0;
	int cantidad=list_size(lista_de_bloquesLibres);
	actualizar_bloques_libres();
	while(contador<cantidad){

	t_bloque_libre *libre=list_get(lista_de_bloquesLibres,contador);
	printf("La direccion del bloque libre %p \n",libre->inicio);
	puntero=(libre->inicio+libre->tamanio);
	printf("La suma del tamaño del bloque libre y el puntero es %p \n",puntero);
	hago_lista_con_ids();
	t_segmento *unSegmento=buscar_id_segun_posicion();
	printf("El segmento que sigue a ese bloque esta ubicado en la direccion%p\n",unSegmento->ubicacion_memoria);
	char*puntero1=malloc(unSegmento->tamanio);
	puntero1=unSegmento->ubicacion_memoria;
	if(libre->tamanio >= unSegmento->tamanio){
		memcpy(libre->inicio,puntero1,unSegmento->tamanio);
		unSegmento->ubicacion_memoria=libre->inicio;
		t_bloque_libre *libre1=list_get(lista_de_bloquesLibres,contador);
		t_bloque_libre *libre2=list_get(lista_de_bloquesLibres,contador+1);

		if(libre1->inicio+libre1->tamanio==libre2->inicio)
			libre2->inicio=libre1->inicio;
			libre2->tamanio=libre1->tamanio+libre2->tamanio;
			eliminar_bloque_libre(contador);
	}
	else{
		char* aux=malloc(unSegmento->tamanio);
		aux=puntero1;
		memcpy(aux,puntero1,unSegmento->tamanio);
		memcpy(libre->inicio,aux,unSegmento->tamanio);
		printf("Prueba %p\n",libre->inicio);
		unSegmento->ubicacion_memoria=libre->inicio;
		printf("Prueba %p\n",unSegmento->ubicacion_memoria);

		t_bloque_libre *libre1=list_get(lista_de_bloquesLibres,contador);
		t_bloque_libre *libre2=list_get(lista_de_bloquesLibres,contador+1);

				if(libre1->inicio+libre1->tamanio==libre2->inicio)
					libre2->inicio=libre1->inicio;
					libre2->tamanio=libre1->tamanio+libre2->tamanio;
					printf("L %p \n",libre->inicio);
					eliminar_bloque_libre(contador);
					printf("L %p \n",libre->inicio);
					cantidad=list_size(lista_de_bloquesLibres);
	}

	printf("Despues de la compactacion el bloque tiene direccion %p \n",libre->inicio);
	printf("Despues de la compactacion la ubicacion del segmento es: %p\n",unSegmento->ubicacion_memoria);
contador++;
	}

}

//---------FIN COMPACTACION-------------

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

t_segmento *buscar_id_segun_posicion(){
	int i=0;
	int encontrado=0;
	t_segmento* segmento;
	int cant=list_size(lista_de_id);
	while(encontrado==0 && i<cant ){
		t_id* identificador=list_get(lista_de_id,i);
		t_list* lista=sacar_elemento_de_diccionario(identificador->id);
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

//Condicion de que la ubicacion en memoria de un segmento es igual al puntero que se busca
int es_puntero_de_segmento(t_segmento *segmento){
	return (segmento->ubicacion_memoria==puntero);
}

//Se actualizan los bloques libres, para evitar que se busquen direcciones que estan libres
void actualizar_bloques_libres(){
	int contador=0;
	int cantidad=list_size(lista_de_bloquesLibres);
	while(contador<cantidad){
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


//Levanta el archivo de configuracion
void levantarArchivoUMV(){
	t_config *archivoConf;
	archivoConf=config_create(direccion);
	string=config_get_string_value(archivoConf,"ip_umv");
	strcpy(ipUMV,string);
	string=config_get_string_value(archivoConf,"algoritmo");
	strcpy(algoritmo_de_ubicacion,string);
	puertoUMV=config_get_int_value(archivoConf,"puerto_umv");
	capacidad_de_memoria=config_get_int_value(archivoConf,"espacio_memoria");
	config_destroy(archivoConf);
}
void recibirConexiones(){
	//Variables de comunicacion
		int socketEscucha, socketNuevaConexion;
	    char buffer[BUFF_SIZE];
		struct sockaddr_in socketInfo;
		int nbytesRecibidos;
		int optval = 1;
		char* identificador;
		int flag=1;

	//Preparo el socket para recibir conexiones
		if ((socketEscucha = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			return EXIT_FAILURE;
		}

		setsockopt(socketEscucha, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

		socketInfo.sin_family = AF_INET;
		socketInfo.sin_addr.s_addr = INADDR_ANY;//Mi ip
		socketInfo.sin_port = htons(puertoUMV);//Mi puerto

		if (bind(socketEscucha, (struct sockaddr*) &socketInfo, sizeof(socketInfo))!= 0) {
			perror("Error al bindear socket escucha");
			return EXIT_FAILURE;
		}

		//Preparado para escuchar conexiones
		if (listen(socketEscucha, 10) != 0) {
			perror("Error al poner a escuchar socket");
			return EXIT_FAILURE;

		}
		printf("Recibiendo conexiones..\n");

		    //Acepta una conexion
			if ((socketNuevaConexion = accept(socketEscucha, NULL, 0)) < 0) {
				perror("Error al aceptar conexion entrante");
				return EXIT_FAILURE;
		}
		while(1){
		    //Recibe el buffer enviado
			if ((nbytesRecibidos = recv(socketNuevaConexion, buffer, BUFF_SIZE,0))> 0) {
		        buffer[nbytesRecibidos]='\0';
		        printf("Se ha recibido informacion\n");
		        mensaje=desempaquetarMensaje(buffer);
		        switch(mensaje.id){
		        case RESERVAR_MEMORIA_PCB:{
		        	segmento=desempaquetarSegmento(mensaje.payload);
		        	printf("Identificador del programa: %d\n",segmento.idPrograma);
		        	printf("Identificador del segmento: %s\n",segmento.idSegmento);
		        	printf("Direccion del puntero: %p\n",segmento.dirPuntero); //Lo cambie porque me decia que no era de tipo compatible con int
		        	printf("Tamaño del segmento: %d\n",segmento.tam);
		        	inicio_en_tabla=segmento.dirPuntero;
		        	itoa(segmento.idPrograma, identificador);//Se pasa el id de programa a *char porque lo necesito asi en mi diccionario
		        	//flag=crear_segmento(identificador,segmento.tam); //Se agrego crear segmento
		        	if(flag)
		        		strcpy(mensaje.payload,"si");
		        	else{
		        		//Hay que destruir los segmentos anteriores de ese programa
		        		strcpy(mensaje.payload,"no");}

		        	empaquetarMensaje(mensaje,buffer);
		        	if (send(socketNuevaConexion, buffer, strlen(buffer), 0) >= 0)
		        	   printf("Datos enviados al kernel!\n");
		        	else
		        	   perror("Error al enviar datos. Kernel no encontrado.\n");
		        	break;
		        }
		        case DESTRUIR_SEGMENTO:{
		        	strcpy(ID,mensaje.payload);
		        	//Hay que destruir todos los segmentos de ese programa
		        }
		        }
			}
			else
			   perror("Error al recibir datos.");
		}
}


