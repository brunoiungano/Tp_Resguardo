#include "utiles.h"
#include "stdlib.h"
#include <time.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <string.h>
#include "error.h"

void empaquetar(char s1[],char s2[],int x,char sf[]){
	int n,j,k;
	char s3[6];
	for(j=0;s1[j]!='\0';){
		sf[j++]=s1[j];
	}
      sf[j++]=':';
      for(k=0;s2[k]!='\0';){
      		sf[j++]=s2[k++];
      	}
       sf[j++]=':';
      itoa(x,s3);
      for(n=0;s3[n]!='\0';){
    	  sf[j++]=s3[n++];
    	  }
      sf[j]='\0';
      //printf("Empaquetado: %s\n",sf);
}
int desempaquetar(char s1[],char sf1[],char sf2[]){
      	int i,n,x,j;
      	char s2[10];
      	for(i=0;s1[i]!=':';){
      		sf1[i++]=s1[i];}
               sf1[i++]='\0';
          for(j=0;s1[i]!=':';){
              sf2[j++]=s1[i++];}
              sf2[j++]='\0';
              i++;
          for(n=0;s1[i]!='\0';){
          	s2[n++]=s1[i++];
      	}
           s2[n]='\0';
           x= atoi(s2);
           //printf("Desempaquetado: %s, %s y %d\n",sf1,sf2,x);
           return x;
      }

void empaquetarCompleto(char s1[],char s2[],int x,char s3[],char s4[],char s5[],char sf[]){
	int n,j;
	char s6[6];
	for(j=0;s1[j]!='\0';){
		sf[j++]=s1[j];
	}
      sf[j++]=':';
      for(n=0;s2[n]!='\0';){
      		sf[j++]=s2[n++];
      	}
       sf[j++]=':';
      itoa(x,s6);
      for(n=0;s6[n]!='\0';){
    	  sf[j++]=s6[n++];
    	  }
      sf[j++]=':';
      for(n=0;s3[n]!='\0';){
            		sf[j++]=s3[n++];
            	}
      sf[j++]=':';
      for(n=0;s4[n]!='\0';){
          	  sf[j++]=s4[n++];
          	  }
            sf[j++]=':';
        for(n=0;s5[n]!='\0';){
              sf[j++]=s5[n++];
            }
        sf[j++]='\0';
      //printf("Empaquetado: %s\n",sf);
}
int desempaquetarCompleto(char s1[],char sf1[],char sf2[],char sf3[],char sf4[],char sf5[]){
      	int i,n,x;
      	char s2[10];
      	for(i=0;s1[i]!=':';){
      		sf1[i++]=s1[i];}
               sf1[i++]='\0';
          for(n=0;s1[i]!=':';){
              sf2[n++]=s1[i++];}
              sf2[n++]='\0';
              i++;
          for(n=0;s1[i]!=':';){
          	s2[n++]=s1[i++];
      	      }
           i++;
           s2[n]='\0';
           x= atoi(s2);
           for(n=0;s1[i]!=':';){
                sf3[n++]=s1[i++];}
                sf3[n++]='\0';
                i++;
           for(n=0;s1[i]!=':';){
                sf4[n++]=s1[i++];}
                sf4[n++]='\0';
                i++;
           for(n=0;s1[i]!='\0';){
                  sf5[n++]=s1[i++];}
                  sf5[n++]='\0';
                  i++;
           //printf("Desempaquetado: %s, %s, %d, %s, %s y %s\n",sf1,sf2,x,sf3,sf4,sf5);
           return x;
      }

/* reverse: reverse string s in place */
void reverse(char s[])
{
int c, i, j;
for (i = 0, j = strlen(s)-1; i < j; i++, j--) {
c = s[i];
s[i] = s[j];
s[j] = c;
}
}


/* itoa: convert n to characters in s */
void itoa(int n, char s[])
{
int i, sign;
if ((sign = n) < 0) /* record sign */
n = -n; /* make n positive */
i = 0;
do { /* generate digits in reverse order */
s[i++] = n % 10 + '0'; /* get next digit */
} while ((n /= 10) > 0); /* delete it */
if (sign < 0)
s[i++] = '-';
s[i] = '\0';
reverse(s);
}
/* atoi: convert s to integer */

int atoi (const char s[])
{
int i, n;
n=0;
for(i = 0; s[i]>='0' && s[i]<='9';++i){
n= 10*(n + (s[i]-'0'));}
return n/10;
}
int comparoPrimerasXletras(int x,char buffer1[],char buffer2[]){
	int i=0,ok=1;
	while((i!=x)&&(ok)){
		if(buffer1[i]!=buffer2[i]){
			ok=0;
		}
	  i++;
	 }
    if(ok)
      return 0;
    else
      return 1;
}

void nivelDeEncabezado(char buffer1[],char buffer2[]){
     int i=0,j=0;
     while(buffer1[i]!='N'){
    	 i++;
     }
     while(buffer1[i]!=']'){
    	 buffer2[j++]=buffer1[i++];
     }
     buffer2[j]='\0';
}
void empaquetarMensaje(Msg mensaje,char sf[]){
	int i=0,k;
    sf[i++]=mensaje.id;
	sf[i++]=':';
	for(k=0;mensaje.payload[k]!='\0';){
      		sf[i++]=mensaje.payload[k++];
      	}
       sf[i]='\0';
       //printf("Empaquetado: %d:%s\n",mensaje.id,mensaje.payload);
}
Msg desempaquetarMensaje(char s1[]){
      	int i,j=0;
        char sf1[20];
      	Msg mensaje;
        mensaje.id=s1[0];
        for(i=2;s1[i]!='\0';){
              sf1[j++]=s1[i++];}
              sf1[j++]='\0';
        strcpy(mensaje.payload,sf1);
        //printf("Desempaquetado: id= %d y payload= %s\n",mensaje.id,mensaje.payload);
        return mensaje;
      }
void dameHora(char tiempo[]) {
	time_t log_time;
	struct tm *log_tm;
	struct timeb tmili;
	char *str_time = string_duplicate("hh:mm:ss:mmmm");

	if ((log_time = time(NULL)) == -1) {
		error_show("Error getting date!");
		return 0;
	}

	log_tm = localtime(&log_time);

	if (ftime(&tmili)) {
		error_show("Error getting time!");
		return 0;
	}

	char *partial_time = string_duplicate("hh:mm:ss");
	strftime(partial_time, 127, "%H:%M:%S", log_tm);
	sprintf(str_time, "%s:%hu", partial_time, tmili.millitm);
	free(partial_time);

	strcpy(tiempo,str_time);
	free(str_time);
}
