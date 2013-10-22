#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>

//#include"cabeceras.h"
#include "rx2_1.h"
#include "IFTI_P2_automat.h"

//define las constantes */

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS0"
//#define MODEMDEVICE "./tube"
#define _POSIX_SOURCE 1 /* fuentes cumple POSIX  */
#define FALSE 0
#define TRUE 1


/* volatile  Lo que significa  ese modificador es que esa variable va a ser modificada por factores externos al compilador*/

volatile int STOP=FALSE;

void signal_handler_IO (int status);   /* definicion del manejador de segnal, cabecera de un metodo, se especifica abajo*/
int wait_flag=TRUE;                    /* TRUE mientras no segnal recibida */

void to_network_layer (packet *paq, FILE *pfs);
int from_physical_layer (frame *tra, int fde);
void imprime_trama(frame tra);
void recu_intro( char *cad);

/*Automata*/
TAutomat * s1_automat_define( );
void print_estado(TEntity *entidad);


/*Tx ACK/NACK*/
void formato_trama(frame *tra);
 void to_physical_layer (frame *tra, int fds);


main(int argc, char *argv[])
{
   int fd,tam_trama;
   struct termios oldtio,newtio; /*estructura termios para com asincronas */
   struct sigaction saio;           /* definicion de accion de segnal  */
   FILE *arch2; 
   packet paquete;
   frame trama;

   /*=========================================================Para el autoamata*/ 
 
  TAutomat *automata = NULL;
   TEntity  entidad1;
   TEntity *active_en = &entidad1;
   Argumentos argu1;
   Argumentos *argu = &argu1; 
       
   /*=========================================================Para Datos Erroneos*/ 
    sigset_t newmask, oldmask; //declaración de variables
    int modo_f, prob_error, err_ack;
    char user_e,intro_b;

   argu->paquete=&paquete; 
   
   //con malloc 
    if ( (argu->trama_e=(frame *) malloc(sizeof(frame)) ) == NULL)
     {
      perror ("No hay suficiente espacio de memoria");
      exit(-1);
      }
   

/* abre el dispositivo en modo no bloqueo (read volvera inmediatamente)
se devolvera en fd el descriptor del archivo */

      fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK);
   if (fd <0) {  perror(MODEMDEVICE); exit(-1);  } //cacha el error

/* instala el manejador de segnal antes de hacer asincrono el dispositivo */

   saio.sa_handler = signal_handler_IO; /* especifica la acciï¿½ que se va a asociar con SIGIO*/
   saio.sa_flags = SA_NOMASK; /*especifica un cjto de opciones que modifican el comportamiento del proceso de manejo de seï¿½l*/ 
   saio.sa_restorer = NULL; /*es algo caduco ya no se usa*/
   sigaction(SIGIO,&saio,NULL);/*sigaction, permite especificar explicitamente el comportamiento de los
                                gestores de seï¿½les el reiniciado de las llamadas del sistemaesta deshabilitado
                                por defecto, pero puede activarse si se especica el flag de seï¿½l SA RESTART*/

/*int fcntl(int fd, int ope, long arg); 
   fcntl realiza una ope de control sobre el fichero refenciado por fd*/
   
/* permite al proceso recibir SIGIO */

      fcntl(fd, F_SETOWN, getpid());/*F_SETOWN Establece el ID de proceso o el grupo de procesos que recibirï¿½las seï¿½les SIGIO
                                      para los eventos sobre el descriptor fd.*/

/* Hace el descriptor de archivo asincrono*/

   fcntl(fd, F_SETFL, FASYNC); /*F_SETFL Asigna a las banderas de descriptor el valor asignado por FASYNC que se correspondan*/
   tcgetattr(fd,&oldtio); /* salvamos conf. actual del puerto, obtiene los parï¿½etros asociados con el objeto 
                        referido por fd y los guarda en la estructura termios referenciada por&oldtio */

/* 
      fija la nueva configuracion del puerto para procesos de entrada canonica
*/

   newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
   newtio.c_iflag = IGNPAR | ICRNL;
   newtio.c_oflag = 0;
   newtio.c_lflag = ICANON; //Activa el modo canonico
   newtio.c_cc[VMIN]=1;
   newtio.c_cc[VTIME]=0;
   tcflush(fd, TCIFLUSH); /* descarta datos recibidos, al objeto referido por fd, pero no leï¿½os,(por TCIFLUSH)*/
   tcsetattr(fd,TCSANOW,&newtio); /* establece los parï¿½etros asociados con la terminal(puerto) desde la estructura 
                                 termios referenciada por &newtio, TCSANOW es una accion opcional que indica
                                  que elcambio ocurre inmediatamente.  */

 printf("Se ha abierto el fichero \n");  

arch2 = fopen(argv[1],"w+");
 if (arch2 == NULL) {perror("Error al abrir el fichero"); exit(-1); }
 
printf("El puntero es %d \n", &arch2);

  /*=========================================================Para Datos Erroneos*/
  
modo_f = atoi(argv[2]);

prob_error = atoi(argv[3]);

/*===========================Automata===================================================*/
/* Define automata */
/*Se inicializa la matriz de mi automata */
 automata = s1_automat_define( );
 /*Empieza en el estado inicial */
 entidad1.myCurrentState= STATE_INITIAL;
/* ********************** */
 argu->active_en=&entidad1;//active_en;
 
/*Se le pasa el puerto*/
  argu->fde = fd;
print_estado(argu->active_en);
     /* entidad , automata, evento, argumentos de evento */
     automat_transit(argu->active_en,automata,EV_INI, argu);  

/* bucle de espera para entrada. Normalmente se haria algo util aqui  */

 //Envio_ACK (argu->active_en,0,0,argu );

   while (STOP==FALSE) {
                       printf(".");
                       printf("entra en el bucle \n");
  
	               usleep(100000);
/* 
tras recibir SIGIO, wait_flag = FALSE, la entrada esta disponible y puede ser leida
 */
                if (wait_flag==FALSE) {
		     
		                  tam_trama = from_physical_layer(&trama, fd);//en trama recobo la info del TX
				  printf("control : %c \n", trama.control);
				  printf(" *************************uno se ve el modo \n");
				  printf("%i \n",modo_f);
				  //usleep(2000000);
				switch (modo_f)
   				      {
   				       case 0: { printf("Modo automático (=0) \n" );
   				               //Modo automático (=0)
         				          sigemptyset(&oldmask);
         				          sigemptyset(&newmask);
                                                  sigaddset(&newmask, SIGIO);
                  				  sigprocmask(SIG_BLOCK, &newmask, &oldmask);
         				          err_ack=error(prob_error);
          				          sigprocmask(SIG_SETMASK, &oldmask, NULL);
								 break; 
								
						}
   				       case 1: { printf("Modo manual (=1) \n" ); 
					//Modo manual (=1)
					          sigemptyset(&oldmask);
         				          sigemptyset(&newmask);
                                                  sigaddset(&newmask, SIGIO);
                  				  sigprocmask(SIG_BLOCK, &newmask, &oldmask);
						  //leemos decisión del usuario por entrada estándar
                                                  printf("Introduzca : \n"); 
                                                  printf("   0 = No hay perdida de trama \n"); 
                                                  printf("   1 = Hay perdida de trama \n"); 
                                                  user_e=getchar();
						   intro_b =getchar(); 
                                                  err_ack= atoi(&user_e);
          				          sigprocmask(SIG_SETMASK, &oldmask, NULL);
					       printf(" Se ha introducido %i \n",err_ack);
					       break;
					
                                                 }
    				        default : { printf("es un estado no designado = %i \n"); break; } 
      				   }

                                    if ((err_ack == 1)&&(strlen(trama.data) !=0)){
                                            trama.control='E';        
                                                  }


			  if (trama.control=='T'){
				  /*copiamos los datos de la trama al paquete*/
				  printf(" trama de datos \n");
				    strcpy(paquete.data,trama.data);  
                                    //***********************/
                                    recu_intro(paquete.data);
				   printf("paquete %s",paquete.data);                                 
				    if (strlen(paquete.data) ==0 ){
				               STOP=TRUE; /* para el bucle si solo entra un CR */
				               printf(" \n Para \n");
                                         /* entidad , automata, evento, argumentos de evento */
                                         automat_transit(argu->active_en,automata,Tra_Vacia, argu);  
                                             
				     }
				      else {
				           to_network_layer(&paquete, arch2);
				            printf("Escribe en el fichero");   
                                               /* entidad , automata, evento, argumentos de evento*/ 
                                           automat_transit(argu->active_en,automata,Tra_Bien,argu);  
                                            
				          }
				      
                                   wait_flag = TRUE;   /* espera una nueva entrada */
                                 }//if
				 else
				      {
				      printf("Recibido trama mal \n");
				      wait_flag = TRUE;   /* espera una nueva entrada */
                                      /* entidad , automata, evento, argumentos de evento */
                                      automat_transit(argu->active_en,automata,Tra_Mal, argu );  
                                                  
			            }
				      
                             }//if     
		       }//while
		     
      /* restaura la configuracion original del puerto  */
      tcsetattr(fd,TCSANOW,&oldtio);
      close(fd);
      fclose(arch2);

  // automat_transit(argu->active_en,automata,Tra_Vacia, NULL);

    automat_destroy( automata );  
}


/***************************************************************************
* manipulacion de segnales. pone wait_flag a FALSE, para indicar al bucle  *
* anterior que los caracteres han sido recibidos                           *
***************************************************************************/

void signal_handler_IO (int status)
{
   printf("\nRecibida cadena de caracteres\n");
   wait_flag = FALSE;
}

void to_network_layer (packet *paq, FILE *pfs)
{
  char *cade;
  int i=0;
 // cade = "uno";
  //fallo????
  while (paq->data[i] !='\0') 
  {
  fwrite(&paq->data[i],sizeof(paq->data[i]),1,pfs);
  i++;
  } 
// fwrite(paq->data,sizeof(paq->data),1,pfs);
 //fwrite(cade,sizeof(cade),1,pfs);
 printf("Se han escrito los datos: %s  \n ",paq->data);
 printf("Tam datos escritos %i : \n ", i);//sizeof(paq->data));
}


int from_physical_layer (frame *tra, int fde)
{ int res,i;
  char buf[267];//de 0 a 266
  res = read(fde,buf,267); //sizeof(frame)
 printf("\n Esto es lo recibod en puerto %s \n",buf);
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
 printf(" el resto %c    %c    %c  :\n", buf[i+3], buf[i+4], buf[i+5] );
  printf(" el resto2 %c    %c    %c : \n ", tra->FCS1, tra->FCS2, tra->flag_e );
  read(fde,NULL,1);
   imprime_trama(*tra);
 printf("el tam de trama recibida es : %i \n",res);
 printf("datos trama recibida ahora : %s \n", tra->data);
 //se imprime la trama con imprimir trama
  return(res);
}


void imprime_trama(frame tra)
 {
   printf("la los datos de la trama son: \n");

   printf( " flag_s  address control FCS1  FCS2 flag_e \n");

    printf( "  %c           %c         %c     %c    %c    %c \n",tra.flag_s, tra.address, tra.control, tra.FCS1, tra.FCS2, tra.flag_e );
    printf("   data  = %s \n",tra.data);
 }

void recu_intro( char *cad)
{
  int i;
  
   i=0;
      while (cad[i] != '\0')
        {
         if (cad[i] == 0x7E) cad [i]= '\n';
          i++;        
          }   

}

void formato_trama(frame *tra)
 {
    /* creamos la estrustura de trama*/
      tra->flag_s = 0x7E;
      tra->address= 'A'; //0x08;

    //  tra->control='A';
    /*copiamos los datos a la trama */
    // strcpy(tra->data,paq.data);
    /*ponemos caracter de fin de linea*/
    //strcat(trama.data,"\0");
    //tra->data="ACK";
    //tra->data[n_carac]='\0'  ;

    printf("Estos son los datos de la trama: %s \n",tra->data);
   // printf("Este es el tamaï¿½ de SDU: %i \n",n_carac);
  
    tra->FCS1 = 'C';
 
    tra->FCS2 = 'C';
  
    tra->flag_e = 0x7E;

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
    printf("\n ***********Se ha enviado la trama  \n ");
 //   printf("n datos escrit: %i, tam: %i \n ", pru,sizeof(frame));
     intro='\n';
    write(fds,&intro,1);
        //imprime_trama(*tra);
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
  /*AUTOMAT_UNDEFINED_ACTION= 0 => se pondria en lugar de la acion para que no haya ninguna accion */
 
 TAutomat * autom = automat_create( STATE_ERROR,imp_error );
    /*automata, estado_partida, estado siguiente,evento, accion*/
 
 automat_declare_transition( autom,
		STATE_INITIAL,  STATE_WAIT_Tra,
		EV_INI,Recib_Trama);
 /*Wait_Tra*/

  automat_declare_transition( autom,
		STATE_WAIT_Tra, STATE_WAIT_Tra,
		Tra_Bien,Envio_ACK );

  automat_declare_transition( autom,
		STATE_WAIT_Tra, STATE_WAIT_Tra,
		Tra_Mal, Envio_NACK );

  automat_declare_transition( autom,
		STATE_WAIT_Tra,  STATE_FIN,
		Tra_Vacia, finalizar );

 /*Wait_ReTra*/
/*=======Dejarlo asi===================0=
  automat_declare_transition( autom,
		STATE_WAIT_ReTra, STATE_WAIT_Tra
		Tra_Bien,Envio_ACK );

  automat_declare_transition( autom,
		STATE_WAIT_ReTra, STATE_WAIT_ReTra
		Tra_Mal, Envio_NACK );

  automat_declare_transition( autom,
		STATE_WAIT_ReTra,  STATE_FIN,
		Tra_Vacia, Print_Fin );

*/

  /*Fallos para STATE_INITIAL */

  automat_declare_transition( autom,
		STATE_INITIAL, STATE_ERROR,
		Tra_Bien,imp_error );

  automat_declare_transition( autom,
		STATE_INITIAL, STATE_ERROR,
		Tra_Mal, imp_error );

  automat_declare_transition( autom,
		STATE_INITIAL, STATE_ERROR,
		Tra_Vacia, imp_error );

  /*Fallos STATE_WAIT_Tra */

  automat_declare_transition( autom,
		STATE_WAIT_Tra, STATE_ERROR,
		EV_INI, imp_error );

   /*Fallos STATE_WAIT_ReTra */
/*
   automat_declare_transition( autom,
		STATE_WAIT_ReTra, STATE_ERROR,
		EV_INI, imp_error ); 
/**************************************************
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
 * Recib_Trama
 * ------------------------------------------------------------ */
int Recib_Trama (	TEntity * inEntity,
			TState inFromState,
			TEvent inEvent,
			Argumentos * argu )
{
  int rc = AUTOMAT_OK;
  printf(" ===> Se espera recibir la primera trama. \n");
  print_estado(argu->active_en);
  return rc;
}


/* ------------------------------------------------------------ 
 * Envio_ACK
 * ------------------------------------------------------------ */
int Envio_ACK (	TEntity * inEntity,
			TState inFromState,
			TEvent inEvent,
			Argumentos * argu )
{
  int rc = AUTOMAT_OK;
  printf(" ===> Se Recibio bien la trama \n");
  
  /*Envio ACK*/
               argu->trama_e->control='A';
               strcpy(argu->trama_e->data,"ACK");                                       
               formato_trama(argu->trama_e);
               to_physical_layer(argu->trama_e, argu->fde);  
      printf(" ===> Se envia ACK. \n");
   print_estado(argu->active_en);

  return rc;
}


/* ------------------------------------------------------------ 
 * Envio_NACK
 * ------------------------------------------------------------ */
int Envio_NACK (	TEntity * inEntity,
			TState inFromState,
			TEvent inEvent,
			Argumentos * argu )
{
  int rc = AUTOMAT_OK;
   printf(" ===> Se Recibio mal la trama \n");
   printf(" Se enviara un NACK \n");
  /*Envio NACK*/
               argu->trama_e->control='N';
               strcpy(argu->trama_e->data,"NACK");                                       
               formato_trama(argu->trama_e);
               to_physical_layer(argu->trama_e, argu->fde);  
     printf(" ===> Se envia NACK. \n");
                                      
          print_estado(argu->active_en);   
  return rc;
}


/* ------------------------------------------------------------ 
 * finalizar
 * ------------------------------------------------------------ */
int finalizar     (	TEntity * inEntity,
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
   
        switch (enti)
         {
          case 1: { printf("es el estado STATE_INITIAL   = %i \n", enti ); break; }
          case 2: { printf("es el estado STATE_WAIT_Tra  = %i \n", enti ); break; }
          case 3: { printf("es el estado STATE_FIN = %i \n", enti ); break; }
          case 4: { printf("es el estado STATE_ERROR = %i \n", enti ); break; }
          default : { printf("es un estado no designado = %i \n", enti ); break; } 
          }

  }
  
 //*** Función de probabilidad de error ******************************** 
 int error(double prob_error_perdida) 
 {  long r;
    double x;
    static double two31;
    int valid; 
    r = random(); 
    two31=1.0; 
    
    for (valid=0;valid<31;valid++)
         two31*=2.0; 
 
       x = ((double)r)/two31; 
	 
	 if ( x<= prob_error_perdida*0.01)
	       return 1; //error-perdida en la trama 
	 else
	 
	   return 0; //no hay error-perdida en la trama 
	    
}

