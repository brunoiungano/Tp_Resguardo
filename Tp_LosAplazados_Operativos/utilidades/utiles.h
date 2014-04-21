#ifndef __LIBUTIL__
#define __LIBUTIL__

typedef struct msg
{
    char id;
    char payload[20];
}Msg;
int atoi (const char s[]);
void itoa(int n, char s[]);
void reverse(char s[]);
void empaquetar(char s1[],char s2[],int x,char sf[]);
int desempaquetar(char s1[],char sf1[],char sf2[]);
void empaquetarCompleto(char s1[],char s2[],int x,char s3[],char s4[],char s5[],char sf[]);
int desempaquetarCompleto(char s1[],char sf1[],char sf2[],char sf3[],char sf4[],char sf5[]);
void nivelDeEncabezado(char buffer1[],char buffer2[]);
int comparoPrimerasXletras(int x,char buffer1[],char buffer2[]);
void empaquetarMensaje(Msg mensaje,char sf[]);
Msg desempaquetarMensaje(char s1[]);
void dameHora(char tiempo[]);
#endif
