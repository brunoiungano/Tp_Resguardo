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
#include <commons/log.h>
#include "utiles.h"

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

t_bloque_libre *bloque_create(t_bloque_libre );

int determinar_direccion_logica(char *);

int validacion_en_memoria(int ,t_list *);

void actualizar_memory(int ,t_bloque_libre **);

void imprimir_estado_memory(t_bloque_libre *);

char *colocar_en_memoria_FirstFit(int);

char *colocar_en_memoria_WorstFit(int);

int _bloques_mayor_a_menor(t_bloque_libre *, t_bloque_libre *);

int _bloques_ordenados_por_direccion(t_bloque_libre *, t_bloque_libre *);

t_bloque_libre *buscar_bloque_en_lista();

t_id *idEnList_create(char*);

t_segmento* buscar_segmento_segun_posicion();

void _agregar_a_lista(char* );

void hago_lista_con_ids();

void compactar_memoria();

void elimino_bloque(t_bloque_libre *);

void eliminar_bloque_libre();

void eliminar_bloques_libres_vacios();

void actualizar_bloques_libres();

void levantarArchivoUMV();

int hay_algun_bloque_disponible(int);

t_segmento *buscar_segmento_segun_base(int);

void recibirConexiones();

void *conexion(void *numeroConexion);

int escribir_bytes(int,int,int ,char*);

char *leer_bytes(int,int,int);

void _leerDeMemoria_bloquesLibres (t_list* );

int baseDeSegmento_pertenece_a_id(char *,int );

void retardo ();

void armaReporte ();

void *mostrarConsola (void* parametro);

void _disenioConsola ();

void _desarrolloConsola(int);

void _cambiaAlgoritmo();

void _consultaOpcion();

void _haceOperacion();

void buscaContenidoMemoria(int, int);

void buscaContenidoMemoriaYguarda(int,int);

void dump ();

void cambio_de_proceso_activo(char *);


t_dictionary *dictionary;
t_list *lista_de_bloquesLibres;
t_list* lista_de_id;

char ID[4];
char base_segmento[8];
int baseSegmento;
void* memoria_principal;
char* manejador;
char algoritmo_de_ubicacion[20];
char direccion[100], ipUMV[20], *string;
int puertoUMV,capacidad_de_memoria;
Msg mensaje;
Sgm segmento;
char* inicio_en_tabla;
char buffer[BUFF_SIZE];
int  socketNuevaConexion[20];
int nbytesRecibidos, flag, cantidadConexiones;
pthread_t hiloConexion[20];
pthread_t hiloMenu;
pthread_mutex_t semHiloConexion;
char **numConexion;
char* temp_file;
t_log* logger;
char id_proceso_activo[6];

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


pthread_create(&hiloMenu,NULL,mostrarConsola,NULL);
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

int hay_algun_bloque_disponible(int cantidad){
	int _es_bloque_accesible(t_bloque_libre *bloque){ //Condicin es bloque acesible, evalua si el tamaño del segmento es menor que el tamaño libre
		return (cantidad<=bloque->tamanio);
		}
return list_any_satisfy(lista_de_bloquesLibres,(void*)_es_bloque_accesible);//Me retorna true si por lo menos hay un bloque libre que permita la creacion de dicho segmento
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
 		int _es_mismo_id(t_id *id1){
 			return (strcmp(id1->id,id)==0);
 		}
 		void _destruir_id(t_id *id2){
 			free(id2);
 		}
 	list_remove_and_destroy_by_condition(lista_de_id, (void*)_es_mismo_id,(void*)_destruir_id); //Se agrego esta linea de codigo para eliminar el ID del programa que se desea eliminar
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
	if(hay_algun_bloque_disponible(tamanio)){

			if(dictionary_has_key(dictionary,id)){
				list=dictionary_get(dictionary,id);
				nuevoSegmento.inicio=determinar_direccion_logica(id);
				baseSegmento=nuevoSegmento.inicio;
				nuevoSegmento.tamanio=tamanio;
				if(strcmp(algoritmo_de_ubicacion,"First Fit")==0)
					nuevoSegmento.ubicacion_memoria=colocar_en_memoria_FirstFit(tamanio);
					else nuevoSegmento.ubicacion_memoria=colocar_en_memoria_WorstFit(tamanio);
				eliminar_bloques_libres_vacios();
				t_bloque_libre *obtenido=buscar_bloque_en_lista(nuevoSegmento.ubicacion_memoria);
				actualizar_memory(tamanio,&obtenido);
				printf("La base del segmento es: %d \n",nuevoSegmento.inicio);
				printf("Ubicacion del segmento %p\n",nuevoSegmento.ubicacion_memoria);
				printf("Comienzo de bloque Libre %p despues de haber guardado %d bytes\n",obtenido->inicio,tamanio);
				printf("Espacio en memoria disponible %d\n",obtenido->tamanio);
				list_add(list,segmento_create(nuevoSegmento));}
			else{
				t_list *lista=list_create();
				nuevoSegmento.inicio=0;
				baseSegmento=nuevoSegmento.inicio;
				nuevoSegmento.tamanio=tamanio;
				if(strcmp(algoritmo_de_ubicacion,"First Fit")==0)
					nuevoSegmento.ubicacion_memoria=colocar_en_memoria_FirstFit(tamanio);
					else nuevoSegmento.ubicacion_memoria=colocar_en_memoria_WorstFit(tamanio);
				t_bloque_libre *obtenido=buscar_bloque_en_lista(nuevoSegmento.ubicacion_memoria);
				list_add(lista_de_id,idEnList_create(id));//Se grega ID en lista, al agregarlo en este lado del case solamente lo agregamos una vez
				eliminar_bloques_libres_vacios();
				actualizar_memory(tamanio,&obtenido);
				printf("La base del segmento es: %d \n",nuevoSegmento.inicio);
				printf("Ubicacion del segmento %p\n",nuevoSegmento.ubicacion_memoria);
				printf("Comienzo de bloque Libre %p despues de haber guardado %d bytes\n",obtenido->inicio,tamanio);
				printf("Espacio en memoria disponible %d\n",obtenido->tamanio);
				list_add(lista,segmento_create(nuevoSegmento));
				poner_segmento_en_diccionario(lista,id);

		}
		return 1 ;
	}
	else{
	printf("No hay memoria disponible\n");
		return 0;}


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
	new->tamanio=capacidad_de_memoria;
	return new;
}

//Criterio a tomar para determinar el inicio de mi segmento en la tabla de segmentos
int determinar_direccion_logica(char *id){
	int maximo;
	t_list *lista=dictionary_get(dictionary,id);
	int elementos_en_lista=list_size(lista);
	t_segmento *ultimo_segmento=list_get(lista,elementos_en_lista-1);
	maximo=(ultimo_segmento->inicio + ultimo_segmento->tamanio)+1;
	return maximo;
}

//Creo nodo dinamico crear el nodo id de la lista de id
t_id *idEnList_create(char*id){
	t_id *new = malloc( sizeof(t_id) );
	strcpy(new->id,id);
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

//Condicion de t_bloque_libre *buscar_bloque_en_lista() para que me encuentre el bloque con dicha direccion




//----------COMPACTACION--------------------
//Compactar la memoria principal
void compactar_memoria(){
	char*puntero=memoria_principal;
	int contador=0;
	int flag=0;
	if(list_size(lista_de_bloquesLibres)>1){
		actualizar_bloques_libres();}//actualizo bloques, para juntar aquellos que sus direcciones son contiguas
	list_sort(lista_de_bloquesLibres,(void*)_bloques_ordenados_por_direccion);
	eliminar_bloques_libres_vacios();
	printf("tamaño de bloques %d\n",list_size(lista_de_bloquesLibres));

	while(flag==0){
		t_bloque_libre *libre=list_get(lista_de_bloquesLibres,contador);
		printf("La direccion del bloque libre %p \n",libre->inicio);
		puntero=(libre->inicio+libre->tamanio);
		printf("La suma del tamaño %d del bloque libre y el puntero es %p \n",libre->tamanio,puntero);
		if(buscar_segmento_segun_posicion(puntero)!=NULL){//pregunto si la direccion de memoria la encuentra
			t_segmento *unSegmento=buscar_segmento_segun_posicion(puntero);
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

t_segmento *buscar_segmento_segun_posicion(char*puntero){
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
	list_sort(lista_de_bloquesLibres,(void*)_bloques_ordenados_por_direccion);
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

char *leer_bytes(int base,int offset,int cantidad){
	char*puntero=memoria_principal;
	int total=offset+cantidad;
	char*segmento_leido=malloc(cantidad);
	t_segmento *segmento;
	segmento=buscar_segmento_segun_base(base);
	if(segmento!=NULL){
		if(total<=segmento->tamanio){ //offset<segmento->tamanio && cantidad<segmento->tamanio &&
		puntero=segmento->ubicacion_memoria+offset;
		memcpy(segmento_leido,puntero,cantidad);
		return segmento_leido;}
		else{printf("Se desea leer bytes que no pertenecen a este segmento\n");
		return NULL;
		}}
	else{printf("La base elegida no corresponde a una base de un segmento generado");
	return NULL;}
}

int baseDeSegmento_pertenece_a_id(char *identificador,int base){ //Condicion de seguridad, para saber si el programa puede escribir o leer de la base que me esta mandando
	t_list *lista=dictionary_get(dictionary,identificador);
		int es_base_de_segmento(t_segmento *segmento){
			return (segmento->inicio==base);
			}
return list_any_satisfy(lista,(void*)es_base_de_segmento);

}

int escribir_bytes(int base,int offset,int cantidad,char* BUFFER){
	int total;
	char*puntero=memoria_principal;
	t_segmento *segmento;
	total=offset+cantidad;
	segmento=buscar_segmento_segun_base(base);
	if(segmento!=NULL){
		if( total<=segmento->tamanio){//offset<segmento->tamanio && cantidad<segmento->tamanio &&
		puntero=segmento->ubicacion_memoria+offset;
		memcpy(puntero,BUFFER,cantidad);
		return 1;}
		else { printf("SIGVE:Segmentation Fault,se desea escribir fuera de los rangos permitidos\n");
		return 0;}}
	else {printf("La base elegida no corresponde a una base de un segmento generado");
	return 0;
	}
}

t_segmento *buscar_segmento_segun_base(int base){
	int i=0;
	int encontrado=0;
	t_segmento* segmento;
	int cant=list_size(lista_de_id);
	while(encontrado==0 && i<cant ){
		t_id* identificador=list_get(lista_de_id,i);
		t_list* lista=sacar_elemento_de_diccionario(identificador->id);
		int es_base_de_segmento(t_segmento *segmento){
			return (segmento->inicio==base);
		}
		segmento=list_find(lista,(void*)es_base_de_segmento);
		if(segmento!=NULL){
				encontrado=1;
			}
		else i++;
		}
	if(encontrado!=0)
		return segmento;
	else return NULL;
}

void _leerDeMemoria_bloquesLibres (t_list* lista){
int z = 0;
char variable_prueba[150];

printf("Espacios Libres: \n");
printf("\n");


while (list_size(lista)>z){

t_bloque_libre *bloque_libre = list_get(lista_de_bloquesLibres,z);
printf("| Bloque libre: %d,Inicio de bloque Libre: %p tamaño: %d\n",z+1,bloque_libre->inicio,bloque_libre->tamanio);
sprintf(variable_prueba,"| Bloque libre: %d,Inicio de bloque Libre: %p , tamaño: %d\n",z+1,bloque_libre->inicio,bloque_libre->tamanio);
log_info(logger, "| Log de bloques libres %s", variable_prueba);

z++;
}

}


void retardo (){
int a;

printf("Ingrese la cantidad de segundos que desea retardar el sistema: ");
scanf("%d",&a);
sleep (a);
}


void armaReporte (){



	printf("\n");
	printf("\n");
	printf("-----------------------------------\n");
	printf("La información del diccionario es: \n");
	printf("-----------------------------------\n");


	void _leerDeDiccionario(t_id* ident, t_list* lista) {
	char variable_prueba[150];
	int z = 0;



	while (list_size(lista)>z){
	t_segmento *obtenido = list_get(lista,z);
	sprintf(variable_prueba,"| ID: %s  | Inicio: %d  | Ubicacion en Memoria: %p  | Tamanio: %d  |\n", ident->id,obtenido->inicio,obtenido->ubicacion_memoria,obtenido->tamanio);
	log_info(logger, "| Log de estado memoria %s", variable_prueba);
	z++;
	}
		}



	dictionary_iterator(dictionary, (void*) _leerDeDiccionario);
	printf("\n");
	_leerDeMemoria_bloquesLibres(lista_de_bloquesLibres);


}

void *mostrarConsola (void *parametro){
t_config *archivoConf;
int validador;
archivoConf=config_create(direccion);
validador=config_get_int_value(archivoConf,"reporte_en_pantalla");
config_destroy(archivoConf);
temp_file = tmpnam(NULL);
logger = log_create(temp_file, "Memory",validador, LOG_LEVEL_INFO);

_disenioConsola();

}



void _disenioConsola () {
int op=0;



printf("\n-----------Consola UMV-----------\n");
printf("\n");
printf("\n");
printf("Ingrese la opcion deseada: \n");
printf("1 - Operacion\n");
printf("2 - Retardo\n");
printf("3 - Algoritmo\n");
printf("4 - Compactacion\n");
printf("5 - Dump \n");
printf("\n");
printf("Ingrese 0 para salir de la consola\n");


scanf("%d",&op);

_desarrolloConsola(op);

}

void _desarrolloConsola(int opcion){


switch(opcion){
	case 1: _haceOperacion();

	break;

	case 2: retardo();
	_consultaOpcion();
	break;

	case 3: _cambiaAlgoritmo();
	_consultaOpcion();
	break;

	case 4: {
		pthread_mutex_lock (&semHiloConexion);
		compactar_memoria();
		pthread_mutex_unlock (&semHiloConexion);
	_consultaOpcion();}
	break;

	case 5: dump();
	_consultaOpcion();
	break;

	case 0: printf("Usted ha salido de la consola\n");
	break;

	default: printf("Usted ha ingresado una opcion no valida, por favor vuelva a seleccionar una opcion.\n");
	_consultaOpcion();

}

}


void _cambiaAlgoritmo(){
int elige_algoritmo=0;


printf("Ingrese el algoritmo que desea cambiar \n");
printf("1 - First Fit / 2 - Worst Fit\n");
scanf("%d",&elige_algoritmo);

switch(elige_algoritmo){

case 1 : strcpy(algoritmo_de_ubicacion,"First Fit");
         printf("Se ha cambiado el algoritmo por First Fit\n");
         break;

case 2 : strcpy(algoritmo_de_ubicacion,"Worst Fit");
         printf("Se ha cambiado el algoritmo por Worst Fit\n");
         break;

default: printf("El numero ingresado no corresponde a ningun algoritmo disponible\n");
		 _cambiaAlgoritmo();
		 break;
		 }
}


void _consultaOpcion(){
	int decision=0;
	printf("\n");
	printf("\n");
	printf("\n");
	printf("Desea salir de la consola? Ingrese 0 para salir o cualquier digito para volver al menú\n");
	scanf("%d",&decision);

	switch(decision){
	case 0: printf("Ha salido de la consola\n");
	log_destroy(logger);
	break;

	default: _disenioConsola();
	break;
	}
}


void _haceOperacion(){
int opcion2 = 0;
int base;
int bandera;
int bandera1;
int offset = 0;
int cantidad = 0;
char *palabra='\0';
char id[6];
int tamanio=0;
char variable_prueba[200];


printf("Ingrese el tipo de operación que desea realizar: \n");
printf("1 - Solicitar bytes\n");
printf("2 - Enviar bytes\n");
printf("3 - Crear Segmento\n");
printf("4 - Destruir Segmento\n");
printf("5 - Cambiar proceso activo \n");
printf("6 - Volver al menu principal\n");
scanf("%d",&opcion2);

switch(opcion2){

case 1: printf("Ingrese la base: \n");
        scanf("%d",&base);
		printf("Ingrese el offset: \n");
	    scanf("%d",&offset);
		printf("Ingrese la cantidad de bytes: \n");
		scanf("%d",&cantidad);

		pthread_mutex_lock (&semHiloConexion);
		if(leer_bytes(base, offset, cantidad)==NULL){
				  sprintf(variable_prueba,"| Base: %d  | Offset: %d  | CantidadBytes: %d |\n", base, offset, cantidad);
				  log_error(logger, "| Se ha solicitado leer la siguiente información pero no esta disponible %s \n", variable_prueba);

				  }

			  else {
				  leer_bytes(base, offset, cantidad);

				  sprintf(variable_prueba,"| Base: %d | Offset: %d  | CantidadBytes: %d |\n", base, offset, cantidad);
			  	  log_info(logger, "| Se ha solicitado leer la siguiente información %s \n", variable_prueba);


			  }
		pthread_mutex_unlock (&semHiloConexion);
			  _consultaOpcion();
			  break;

case 2: printf("Ingrese la base: \n");
		scanf("%d",&base);
		printf("Ingrese el offset: \n");
		scanf("%d",&offset);
		printf("Ingrese la cantidad de bytes: \n");
		scanf("%d",&cantidad);
		printf("Ingrese la palabra: \n");
		scanf("%s",palabra);
		pthread_mutex_lock (&semHiloConexion);
		bandera=escribir_bytes(base, offset, cantidad, palabra);
		pthread_mutex_unlock (&semHiloConexion);
		if(bandera){
			sprintf(variable_prueba,"| Base: %d  | Offset: %d  | CantidadBytes: %d | Palabra: %s \n", base, offset, cantidad, palabra);
			log_info(logger, "| Se ha solicitado escribir la siguiente información %s \n", variable_prueba);}
		else {sprintf(variable_prueba,"| Base: %d  | Offset: %d  | CantidadBytes: %d | Palabra: %s \n", base, offset, cantidad, palabra);
		log_error(logger, "| Se ha solicitado escribir en zona de memoria no permitida %s \n", variable_prueba);}




		_consultaOpcion();
		break;

case 3: printf("Ingrese el id del segmento: \n");
			 scanf("%s",id);
		printf("Ingrese el tamanio: \n");
			 scanf("%d",&tamanio);
			 if(tamanio>0){
				 pthread_mutex_lock (&semHiloConexion);
				 bandera1=crear_segmento(id,tamanio);
				 pthread_mutex_unlock (&semHiloConexion);
				 if(bandera1){
					 printf("Se creo segmento %s de tamaño %d \n",id, tamanio);
					 sprintf(variable_prueba,"| Segmento : %s  | Tamaño: %d | Base: %d\n", id, tamanio,baseSegmento);
					 log_info(logger, "| Se ha solicitado crear el siguiente segmento: %s \n", variable_prueba);
			 	 	 }
				 else {
					 printf("Segmento con id %s de tamaño %d no creado \n",id, tamanio);
					 sprintf(variable_prueba,"| Segmento : %s  | Tamaño: %d |\n", id, tamanio);
					 log_error(logger, "| No hay memoria disponible para crear este segmento: %s \n", variable_prueba);
				 	 }
			 }
			else printf("No se crean segmentos nulos\n");







		 	_consultaOpcion();
			 break;

case 4: printf("Ingrese el segmento que desea destruir: \n");
			scanf("%s",id);
			if (dictionary_has_key(dictionary,id)){
			pthread_mutex_lock (&semHiloConexion);
			destruir_segmentos_de_programa(id);
			pthread_mutex_unlock (&semHiloConexion);
			sprintf(variable_prueba,"| Segmento : %s |\n", id);
			log_info(logger, "| Se ha solicitado eliminar el siguiente segmento: %s \n", variable_prueba);}
			else{
				printf("No es un segmento valido\n");
				sprintf(variable_prueba,"| Segmento : %s |\n", id);
				log_error(logger, "|Se quiere eliminar un segmento inexistente: %s \n", variable_prueba);
			}


		_consultaOpcion();
		 break;


case 5:   printf("Ingrese el ID de proceso que desea cambiar: \n");
		  scanf("%s",id);
		  if(dictionary_has_key(dictionary,id)){
			  cambio_de_proceso_activo(id);
			  printf("Se ha cambiado al proceso %s",id);
			  sprintf(variable_prueba,"| ID : %s |\n", id);
			  log_info(logger, "| Se solicita cambiar el proceso activo por el siguiente: %s \n", variable_prueba);}
		  else {
			  printf("Se ha solicitado cambiar a un proceso inexistente");
			  sprintf(variable_prueba,"| ID : %s |\n", id);
			  log_error(logger, "|Se desea cambiar a un proceso inexistente: %s \n", variable_prueba);

		  }

break;

case 6: _disenioConsola();
		 break;

}

}


void cambio_de_proceso_activo(char *id){

strcpy(id_proceso_activo,id);

}


void buscaContenidoMemoriaYguarda(int offset, int bytes){
char *puntero = memoria_principal+offset;
char info [200];
int z=0;

while (z < bytes && bytes < capacidad_de_memoria){

sprintf(info,"| Elemento : %p \n", puntero);
log_info(logger, "| Los elementos leidos son: %s \n",info);


puntero++;
z++;
}
}

void buscaContenidoMemoria(int offset, int bytes){
char *puntero = memoria_principal+offset;

int z=0;

while (z < bytes && bytes < capacidad_de_memoria){

printf("Elemento %p\n", puntero);


puntero++;
z++;
}
}


void dump (){
int opcion=0;
int offset=0,bytes=0;
char variable_prueba[200];

printf("Ingrese 1 si desea ver y generar un reporte de memoria \n");
printf("Ingrese 2 si desea ver el contenido en memoria de un offset especifico \n");
scanf("%d",&opcion);
switch(opcion){
case 1: armaReporte ();
printf("\n Acceda a la siguiente ruta para revisar el archivo de log que se creo en: %s\n", temp_file);
break;

case 2: printf("Ingrese el offset: \n");
		scanf("%d",&offset);
		printf("Ingrese la cantidad de bytes que desea leer de memoria: \n");
		scanf("%d",&bytes);

		sprintf(variable_prueba,"| Offset : %d | Cantidad bytes: %d \n", offset,bytes);
		log_info(logger, "| Se ha solicitado leer la siguiente información de memoria: %s \n", variable_prueba);
		buscaContenidoMemoriaYguarda(offset,bytes);


		 _consultaOpcion();
	  break;

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
		int socketEscucha;
		struct sockaddr_in socketInfo;
		int optval = 1;
		char **numConexion=(char**)malloc(sizeof(char)*20);
		pthread_mutex_init (&semHiloConexion, NULL);

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
        while(1){
		    //Acepta una conexion
			if ((socketNuevaConexion[cantidadConexiones] = accept(socketEscucha, NULL, 0)) < 0) {
				perror("Error al aceptar conexion entrante");
				return EXIT_FAILURE;}

			*numConexion=(char*)malloc(sizeof(char)*4);
			itoa(cantidadConexiones,*numConexion);
			pthread_create(&hiloConexion[cantidadConexiones++],NULL,conexion,*numConexion);
			numConexion++;
        }
}
void *conexion(void *numeroConexion){
	int nroConexion=atoi(numeroConexion);
	while(1){
	nroConexion=atoi(numeroConexion);
	//Recibe el buffer enviado
	if ((nbytesRecibidos = recv(socketNuevaConexion[nroConexion], buffer, BUFF_SIZE,0))> 0) {
		 buffer[nbytesRecibidos]='\0';
		 pthread_mutex_lock (&semHiloConexion);
		 printf("Se ha recibido informacion\n");
		 mensaje=desempaquetarMensaje(buffer);
		 switch(mensaje.id){
		 case RESERVAR_MEMORIA_PCB:{
			  segmento=desempaquetarSegmento(mensaje.payload);
			  printf("Identificador del programa: %d\n",segmento.idPrograma);
			  printf("Identificador del segmento: %s\n",segmento.idSegmento);
			  printf("Direccion del puntero: %p\n",segmento.dirPuntero); //Lo cambie porque me decia que no era de tipo compatible con int
			  printf("Tamaño del segmento: %d\n",segmento.tam);
			  itoa(segmento.idPrograma, ID);//Se pasa el id de programa a *char porque lo necesito asi en mi diccionario
			  flag=crear_segmento(ID,segmento.tam); //Se agrego crear segmento
			  if(flag){
				 itoa(baseSegmento,base_segmento);
			     strcpy(mensaje.payload,base_segmento);
			  }
			  else{									//Si no encuentra espacio en memoria
				  compactar_memoria();
				  flag=crear_segmento(ID,segmento.tam);
				  if(flag){
					  itoa(baseSegmento,base_segmento);
					  strcpy(mensaje.payload,base_segmento);
				  }
				  else{
					  if(dictionary_has_key(dictionary,ID)){   //Para saber si hay algun segmento de ese id guardado, porque sino pincha
						  destruir_segmentos_de_programa(ID);
					  	  strcpy(mensaje.payload,"no");}
					  else strcpy(mensaje.payload,"no");
				  }
			  }

			  empaquetarMensaje(mensaje,buffer);
			  if (send(socketNuevaConexion[nroConexion], buffer, strlen(buffer), 0) >= 0)
			      printf("Datos enviados al kernel!\n");
			  else
			      perror("Error al enviar datos. Kernel no encontrado.\n");
			  break;}

		 case DESTRUIR_SEGMENTO:{
			  strcpy(ID,mensaje.payload);
			  if(dictionary_has_key(dictionary,ID)){
					  destruir_segmentos_de_programa(ID);
			  	  	  printf("Destruccion de segmentos del programa %s\n",ID);
			  	  	  strcpy(mensaje.payload,"ok");}
			  else{
				  printf("Se quiere borrar un ID que no corresponde a ningun programa \n");
				  strcpy(mensaje.payload,"no");
			  }
			  empaquetarMensaje(mensaje,buffer);
			  if (send(socketNuevaConexion[nroConexion], buffer, strlen(buffer), 0) >= 0)
			    printf("Datos enviados al kernel!\n");
			  else
			    perror("Error al enviar datos. Kernel no encontrado.\n");
			  break;}
	      }
		}
	    else{
		  perror("Error al recibir datos.");
		  break;}
	 pthread_mutex_unlock (&semHiloConexion);
	 }
}

