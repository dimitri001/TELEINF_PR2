#include <stdio.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define FALSE 0
#define TRUE 1


/*int from_network_layer (packet *ps, FILE *pfe);
void to_network_layer (packet *pr, FILE *pfs);
void to_physical_layer (frame *fs, int fds);
int from_physical_layer (frame *fr, int fde);*/

typedef struct {
     char data[261]; //cadena de caracteres a LEER/ESCRIBIR
     int n; /*número de caracteres a LEER/ESCRIBIR (260 caracteres comomáximo)*/
}packet;

typedef struct
{
     char flag_s;
     char address;
     unsigned char control;
     char data[261];
     unsigned char FCS1;
     unsigned char FCS2;
     char flag_e;
}frame;

/*
typedef struct
{
    frame *trama_e;
    packet *paquete;
    int fde;
    TEntity *active_en;
    TAutomat *automata
    //FILE *arch;

}Argumentos;*/



