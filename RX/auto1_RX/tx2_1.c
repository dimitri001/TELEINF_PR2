#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

//#include "cabeceras.h"
#include "tx2_1.h"
#include "IFTI_P2_automat.h"

/*Cabeceras rx*/
#include <sys/signal.h>
#include <sys/types.h>

/* la tasa de baudios esta definida en <asm/ternbuts.h>, que esta incluida <termios.h> */
#define BAUDRATE B38400

// se puede cambiar esta definicion por el puerto correcto

//#define MODEMDEVICE "/dev/ttyS0"
#define MODEMDEVICE "./tube"
#define ENDMINITERM 2 /* ctrl-b to quit miniterm */

#define _POSIX_SOURCE 1 /* POSIX compliant source */ //fuentes cumple POSIX

#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE; 

/* Solo para transmitir*/
int from_network_layer (packet *paq, FILE *pfe);
void to_physical_layer (frame *tra, int fds);
void formato_trama(frame *tra,packet paq,int n_carac);
void imprime_trama(frame tra);
void camb_intro( char *cad);


/*Automata*/
TAutomat * s1_automat_define( );

void print_estado(TEntity *entidad) ;

/*Solo para recibir ACK/NACK*/
void signal_handler_IO (int status);   /* definicion del manejador de segnal, cabecera de un metodo, se especifica abajo*/
int from_physical_layer (frame *tra, int fde);
void recibe_ACK(Argumentos * argu, TAutomat *automata);


int wait_flag=TRUE;                    /* TRUE mientras no segnal recibida */


main(int argc, char *argv[])
{
int fd,n_carac,n,retardo,i;
/*Para Tx */
 FILE *arch;
struct termios oldtio,newtio;

packet paquete;
frame trama,trama_r;// trama_r es de retransmision

/*Para el autoamata */
TAutomat *automata = NULL;
TEntity  entidad1;
TEntity *active_en = &entidad1;
Argumentos argu1;
Argumentos *argu= &argu1;
int n_rtx;//numero de retransmisiones

argu->paquete= &paquete; //con esto evito hacer un malloc


/*Para la RX, es la se�l*/
 struct sigaction saio;           /* definicion de accion de segnal  */

printf("abre archivo \n" );



/*
  Abre el dispositivo modem para lectura y escritura y no como 
  controlador tty porque no queremos que nos mate si el ruido de la linea
  envia un CRTL-C
Se debe usar O_NONBLOCK ????????? */
 fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK);
 if (fd <0) {perror(MODEMDEVICE); exit(-1); }

/***********SIGIO************************************************************************/
/* instala el manejador de segnal antes de hacer asincrono el dispositivo */
   saio.sa_handler = signal_handler_IO; /* especifica la acci� que se va a asociar con SIGIO*/
   saio.sa_flags = SA_NOMASK; /*especifica un cjto de opciones que modifican el comportamiento del proceso de manejo de se�l*/ 
   saio.sa_restorer = NULL; /*es algo caduco ya no se usa*/
   sigaction(SIGIO,&saio,NULL);/*sigaction, permite especificar explicitamente el comportamiento de los
                                gestores de se�les el reiniciado de las llamadas del sistemaesta deshabilitado
                                por defecto, pero puede activarse si se especica el flag de se�l SA RESTART*/

/*int fcntl(int fd, int ope, long arg); 
   fcntl realiza una ope de control sobre el fichero refenciado por fd*/
   
/* permite al proceso recibir SIGIO */

      fcntl(fd, F_SETOWN, getpid());/*F_SETOWN Establece el ID de proceso o el grupo de procesos que recibir�las se�les SIGIO
                                      para los eventos sobre el descriptor fd.*/

/* Hace el descriptor de archivo asincrono*/

   fcntl(fd, F_SETFL, FASYNC); /*F_SETFL Asigna a las banderas de descriptor el valor asignado por FASYNC que se correspondan*/

/********FIN_SIGIO***************************************************************************/


 tcgetattr(fd,&oldtio); /* Almacenamos la configuracion actual del puerto */

newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;

/*
 Ignore bytes with parity errors and make terminal raw and dumb.
 No hace falta ICRNL xk aki transmito, no recibo
*/
/***********************Debo usar  ICRNL?????*/
 newtio.c_iflag = IGNPAR | ICRNL;

/*
 Raw output.
*/
 newtio.c_oflag = 0;

/*
 Don't echo characters because if you connect to a host it or your
 modem will echo characters for you. Don't generate signals.
 ********************************Se debe usar ICANON ?????  */
 newtio.c_lflag = ICANON;// 0;

/* blocking read until 1 char arrives */
 newtio.c_cc[VMIN]=1;
 newtio.c_cc[VTIME]=0;

/* Ahora limpiamos la linea del modem y activamos la configuracion del puerto */
 tcflush(fd, TCIFLUSH);
 tcsetattr(fd,TCSANOW,&newtio);

/* terminal settings done, now handle output */

printf("abre archivo : %s \n",argv[3]);

arch = fopen(argv[3],"r");
 if (arch == NULL) {printf("Error al abrir el fichero \n"); exit(-1); }
 /* guardamos el numero de n*/

  printf(" arch = %i \n", arch );

 retardo= *argv[2];
    
 retardo= retardo * 100000;

 n =  atoi(argv[1]);

 printf(" n = %i \n", n);

 if (n>260){ perror("Lo siento sale del rango de SDU \n");exit(-1); }

/*Gurado el n de retransmisiones*/
n_rtx =*argv[4]; 

/* Define automata */
/*Se inicializa la matriz de mi automata */

 automata = s1_automat_define( );

 /*Empieza en el estado inicial */
 entidad1.myCurrentState= STATE_INITIAL;
 printf(" ***************** Se ha creado el automata y se inicializa \n");
//printf("Estoy en el estado =  %i ",argu->active_en->myCurrentState);


/* ********************** */
/*Se envia primera trama*/

  if ( (argu->trama_e =  (frame *) malloc(sizeof(frame)) )== NULL)
     {
       perror("No hay suficiente espacio de memoria \n");
       exit (-1); 
     }
 

 argu->paquete->n = n;
 argu->n_rtx=n_rtx;
/*Se le pasa el puerto*/
 argu->fde = fd;
          /*argu.paquete.data== argu y paquete son pteros a struct  */
           memset(argu->paquete->data,'\0',sizeof(argu->paquete->data));
         /*devuelve el nde caract leidos*de arch */
            n_carac= from_network_layer (argu->paquete,arch);

/*==============================================================

/*=======================================================*/
 
/*  */


        argu->retardo = retardo;
printf(" Empieza el retardo  \n" );
  //usleep(retardo);
printf(" 1) el retardo1 es = %i \n", retardo );
  //usleep(argu->retardo);

printf(" 2) Retardo2 es = %i \n", argu->retardo );


 argu->active_en=active_en;
printf("Estoy en el estado =  %i \n",argu->active_en->myCurrentState);

                /* entidad , automata, evento, argumentos de evento */
automat_transit(argu->active_en,automata,EV_INI, argu);

trama_r = trama;

printf(" *************************dos \n");
printf("Estoy en el estado =  %i \n",argu->active_en->myCurrentState);
recibe_ACK(argu, automata);

printf("Estoy en el estado =  %i \n",argu->active_en->myCurrentState);

print_estado(argu->active_en);
printf(" va al bucle \n");
  while (feof(arch)==0)
   {
       
              printf("entra en el bucle \n");

/*se guarda el n de caracte que queremos leer*/
              paquete.n = n;
/*argu.paquete.data== argu y paquete son pteros a struct  */
           memset(argu->paquete->data,'\0',sizeof(argu->paquete->data));
         /*devuelve el nde caract leidos*de arch */
            n_carac= from_network_layer (argu->paquete,arch);

            automat_transit(argu->active_en,automata,ACK, argu);
  
/*
===========================================================================*/
 //espero ACK
                recibe_ACK(argu, automata);

//*******************************************************
           } //while

//Se envia la trama de finalizacion 
 trama.data[0]='\0';
 trama.control='T';//0x09;
printf("Se envia la trama vacia %s: ,tam : %d  \n",trama.data,strlen(trama.data));
  to_physical_layer(&trama,fd);

  //usleep(retardo);
    usleep(100000);
    tcsetattr(fd,TCSANOW,&oldtio); /* restore old modem setings */
   close(fd);
 
   fclose(arch);


   automat_transit(active_en,automata,ACK_0, argu);
   argu->active_en=active_en;

    free (argu->trama_e);
    automat_destroy( automata );
}


int from_network_layer (packet *paq, FILE *pfe)
 { 
   int retorno;
 
   retorno = fread(paq->data,sizeof(char),paq->n,pfe);
   
   return(retorno);
 }


 void to_physical_layer (frame *tra, int fds)
 {
   int pru;
   char intro;

//    pru=write(fds,tra,sizeof(frame));
write(fds,&tra->flag_s,sizeof(char));

write(fds,&tra->address,sizeof(char));

write(fds,&tra->control,sizeof( unsigned char));

     write(fds,&tra->data,sizeof(tra->data));

write(fds,&tra->FCS1,sizeof(unsigned char));

write(fds,&tra->FCS2,sizeof(unsigned char));

write(fds,&tra->flag_e,sizeof(char));
          

 //   printf("n datos escrit: %i, tam: %i \n ", pru,sizeof(frame));
     intro='\n';
    write(fds,&intro,1);
   imprime_trama(*tra);
    printf("\n ***********Se ha enviado la trama  \n ");
    usleep(100000);
        
}


void formato_trama(frame *tra,packet paq,int n_carac)
 {

    /* creamos la estrustura de trama*/
      tra->flag_s = 0x7E;
      tra->address= 'A'; //0x08;
 
    /*copiamos los datos a la trama*/
     strcpy(tra->data,paq.data);
    /*ponemos caracter de fin de linea*/
    //strcat(trama.data,"\0");
    tra->data[n_carac]='\0'  ;
  
    printf("Estos son los datos de la trama: %s \n",tra->data);
       printf("Este es el tama� de SDU: %i \n",n_carac);
  
    tra->FCS1 = 'C';
 
    tra->FCS2 = 'C';
  
    tra->flag_e = 0x7E;

 }

void imprime_trama(frame tra)
 {
   printf("la los datos de la trama son: \n");

   printf( " flag_s  address control FCS1  FCS2 flag_e \n");

    printf( "  %c           %c         %c     %c    %c    %c \n",tra.flag_s, tra.address, tra.control, tra.FCS1, tra.FCS2, tra.flag_e );
    printf("   data  = %s \n",tra.data);
 }

void camb_intro( char *cad)
{
  int i;

   i=0;
      while (cad[i] != '\0')
        {
         if (cad[i] =='\n') {cad[i]=0x7E;} 
          i++;        
          }   
  //printf("Cambiados los intros Paquete data : %s \n ",cad  );

}

/*Para recibir ACK */

int from_physical_layer (frame *tra, int fde)
{ int res,i;
  char buf[267];//de 0 a 266
  res = read(fde,buf,267); //sizeof(frame)
 printf("\n Esto es lo recibido en puerto %s \n",buf);
tra->flag_s=buf[0];
tra->address =buf[1];
tra->control=buf[2];
 for (i=0;i<=260;i++) tra->data[i]=buf[i+3];//+3
 tra->FCS1 =buf[i+3];//261+3= 264
  tra->FCS2 =buf[i+4];//261+4=265
  tra->flag_e =buf[i+5];//261+5=266

    //printf("\n ***********Se ha enviado la trama   ");
 //   printf("n datos escrit: %i, tam: %i \n ", pru,sizeof(frame));
 //printf( "  %c           %c         %c     %c    %c    %c \n",tra.flag_s, tra.address, tra.control, tra.FCS1, tra.FCS2, tra.flag_e );
 //printf(" el resto %c    %c    %c  :\n", buf[i+3], buf[i+4], buf[i+5] );
 // printf(" el resto2 %c    %c    %c : \n ", tra->FCS1, tra->FCS2, tra->flag_e );
  read(fde,NULL,1);
   imprime_trama(*tra);
 printf("el tam de trama recibida es : %i \n",res);
 //printf("datos trama recibida ahora : %s \n", tra->data);
 //se imprime la trama con imprimir trama
  return(res);
}

  /*frame *trama_e, int fde,TEntity *active_en, TAutomat *automata */
void recibe_ACK(Argumentos * argu, TAutomat *automata)
{
 int tam_trama;
     while (STOP==FALSE) {
                       printf(".");
                       printf("entra en el bucle - espero ACK\n");
  
	               usleep(100000);
/*tras recibir SIGIO, wait_flag = FALSE, la entrada esta disponible y puede ser leida */

                if (wait_flag==FALSE) {
                                 
		                  tam_trama = from_physical_layer(argu->trama_e, argu->fde);  
				  printf("control : %c \n", argu->trama_e->control);
			  if ( argu->trama_e->control=='A'/*128 ACK*/){
				    /*copiamos los datos de la trama al paquete*/
                                    printf(" *************siete. \n");
   	                            printf(" Recibimos el ACK \n");
     	                            STOP=TRUE; /* para el bucle si solo entra un CR */
				    printf(" \n Para, y enviamos la siguiente trama \n");          	                  
                   	           /* entidad , automata, evento, argumentos de evento */
                     	    	   automat_transit(argu->active_en, automata, ACK, argu);
                                  // argu->active_en=active_en;
                                     /*Se puede borrar ya que paramos*/
				    wait_flag = TRUE;  /* espera una nueva entrada */
                            }//if
				 else
				      {
				      printf("Recibido Recibo Nack\n");
                                  automat_transit(argu->active_en, automata, NACK, argu);
                                  //argu->active_en=active_en;    
                                      //Reeeenvio trama  \n"); 
                                    //  to_physical_layer(trama_e,fde);
                                      imprime_trama(*argu->trama_e);
				      printf("Se ha reenviado la trama\n");

				      wait_flag = TRUE;   /* espera una nueva entrada */
			      }
				      
                             }//if     
		       }//while

}


/***************************************************************************
* manipulacion de segnales. pone wait_flag a FALSE, para indicar al bucle  *
* anterior que los caracteres han sido recibidos                           *
***************************************************************************/

void signal_handler_IO (int status)
{
   printf("\nRecibida Trama\n");
   wait_flag = FALSE;
}


/* ------------------------------------------------------------ 
 * Automat creation
 * ------------------------------------------------------------ */
 /*Aqui se inicalizan todas las transiciones de mi matriz, el procedimiento automat_declare_transition 
 guarda los eventos y sus transiciones en myMAtrix de TTransition,esta matriz
esta dentro de mi automata autom->myMatrix[inEvent][inFromState] */ 
TAutomat * s1_automat_define( )
{
  int rc = AUTOMAT_OK;
  /*AUTOMAT_UNDEFINED_ACTION= 0 => se pomndria en lugar de la acion para que no haya ninguna accion */
                                         /*Estado y accion (la accion devuelve un int)*/
  TAutomat * autom = automat_create( STATE_ERROR,imp_error );
                   /*automata, estado_partida, estado siguiente,evento, accion*/
  automat_declare_transition( autom,
		STATE_INITIAL, STATE_WAIT_ACK,
		EV_INI, sig_trama );

  automat_declare_transition( autom,
		STATE_WAIT_ACK, STATE_WAIT_ACK,
		ACK,sig_trama );

  automat_declare_transition( autom,
		STATE_WAIT_ACK, STATE_WAIT_ACK,
		NACK, reenviar_trama );

  automat_declare_transition( autom,
		STATE_WAIT_ACK,  STATE_FIN,
		ACK_0, finalizar );

  automat_declare_transition( autom,
		STATE_WAIT_ACK,  STATE_FIN_ERROR,
		 N_NACK_MAX, finalizar );

  /*Fallos para STATE_INITIAL */

  automat_declare_transition( autom,
		STATE_INITIAL, STATE_ERROR,
		ACK, imp_error );

  automat_declare_transition( autom,
		STATE_INITIAL, STATE_ERROR,
		NACK, imp_error );

  automat_declare_transition( autom,
		STATE_INITIAL, STATE_ERROR,
		 ACK_0, imp_error );

  /*Fallos WAIT_ACK */

  automat_declare_transition( autom,
		STATE_WAIT_ACK, STATE_ERROR,
		EV_INI, imp_error );

/*
  automat_declare_transition( autom,
		SEM_STATE_CARS_YELLOW, SEM_STATE_BATTERIES,
		SEM_EV_POWEROFF, activate_batteries );

  automat_declare_transition( autom,
		SEM_STATE_BATTERIES, SEM_STATE_BATTERIES,
		SEM_EV_POWEROFF, activate_batteries );
  


  automat_declare_transition( autom,
		SEM_STATE_BATTERIES, SEM_STATE_INITIAL,
		SEM_EV_POWERON, deactivate_batteries_and_restart );
*/
  return autom;
}


/* ------------------------------------------------------------ 
 * Error
 * ------------------------------------------------------------ */
int imp_error(	TEntity * inEntity,
			TState inFromState,
			TEvent inEvent,
			Argumentos * argu )
{
  int rc = AUTOMAT_OK;
  printf(" ===> Internal error. Inconsistent state. \n");
   print_estado(argu->active_en);
  return rc;
}

/* ------------------------------------------------------------ 
 *  sig_trama 
 * ------------------------------------------------------------ */
int sig_trama   (	TEntity * inEntity,
			TState inFromState,
			TEvent inEvent,
			Argumentos * argu )
{
      int rc = AUTOMAT_OK;
      int n_carac;
      packet *paquete;
      packet paq;
  
      printf(" ===> Enviamos trama. \n");

         //para la trama con datos
            memset(argu->trama_e->data,'\0',sizeof(argu->trama_e->data));
             argu->trama_e->control='T';//0x09;
               
         /***********************************/
            camb_intro(argu->paquete->data);
//void formato_trama(frame *tra,packet paq,int n_carac)
           //paquete = argu->paquete;
          // paq= *argu->paquete;
           //paq = *paquete;
  
            formato_trama(argu->trama_e, *argu->paquete/*paq*/, argu->paquete->n);
            printf(" *************cuatro. \n");
           //enviamos trama de datos 
             to_physical_layer(argu->trama_e,argu->fde);
              //  printf("adress , control \n  %c  %c: \n",argu->trama_e->address, argu->trama_e->control);
              //  printf("% i, %i",0x00,0x08);
         /* ********************** */
              print_estado(argu->active_en);   
             usleep(argu->retardo);

  return rc;
}


/* ------------------------------------------------------------ 
 * reenviar_trama
 * ------------------------------------------------------------ */
int reenviar_trama(	TEntity * inEntity,
			TState inFromState,
			TEvent inEvent,
			Argumentos * argu )
{
  int rc = AUTOMAT_OK;
  printf(" ===> Reenviamos trama de datos. \n");
  
           //enviamos trama de datos 
               to_physical_layer(argu->trama_e,argu->fde);
                printf("adress , control %i %i: \n",argu->trama_e->address, argu->trama_e->control);
                printf("% i, %i \n",0x00 );
         /* ********************** */
                 print_estado(argu->active_en);

             usleep(argu->retardo);
  return rc;
}


/* ------------------------------------------------------------ 
 * finalizar
 * ------------------------------------------------------------ */
int finalizar (	TEntity * inEntity,
			TState inFromState,
			TEvent inEvent,
			Argumentos * argu )
{
  int rc = AUTOMAT_OK;
  printf(" ===> Se finaliza el automata. \n");
   print_estado(argu->active_en);
 
  return rc;
}

 void print_estado(TEntity *entidad)
 {
     TState enti;
         enti= entidad->myCurrentState;
   printf("Entro a printf \n");
        switch (enti)
         {
          case 1: { printf("es el estado STATE_INITIAL   = %i \n", enti ); break; }
          case 2: { printf("es el estado STATE_WAIT_ACK  = %i \n", enti ); break; }
          case 3: { printf("es el estado STATE_FIN = %i \n", enti ); break; }
          case 4: { printf("es el estado STATE_ERROR = %i \n", enti ); break; }
          default : { printf("es un estado no designado = %i \n", enti ); break; } 
          }

  }
