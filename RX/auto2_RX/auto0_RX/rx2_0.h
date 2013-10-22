
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <IFTI_P2_automat.h>

/* ------------------------------------------------------------ 
 * State definitions
 * ------------------------------------------------------------ */
enum {
  STATE_INITIAL = AUTOMAT_INITIAL_STATE,
  STATE_WAIT_Tra,
  STATE_FIN,
  STATE_ERROR 
};

#define STATE_TEXT(X) \
(X==STATE_INITIAL)? "STATE_INITIAL" : \
(X==STATE_ERROR)? "STATE_ERROR" : \
(X==STATE_WAIT_ACK)? "STATE_WAIT_ACK" : \
(X==STATE_FIN)? "STATE_FIN" : \
"UNDEF"
/*(X==SEM_STATE_CARS_GREEN)? "SEM_STATE_CARS_GREEN" : \
(X==SEM_STATE_BATTERIES)? "SEM_STATE_BATTERIES" : \*/




/* ------------------------------------------------------------ 
 * Event definitions
 * ------------------------------------------------------------ */
enum {
    EV_INI= 1, 
    Tra_Bien,
    Tra_Mal,
    Tra_Vacia
    
};

/* ------------------------------------------------------------ 
 * Action definitions
 * ------------------------------------------------------------ */
TAutomat * s1_automat_define( );

int imp_error(	TEntity * inEntity,
			TState inFromState,
			TEvent inEvent,
			Argumentos * argu );

int Recib_Trama (	TEntity * inEntity,
			TState inFromState,
			TEvent inEvent,
			Argumentos * argu );

			
int Envio_ACK (	TEntity * inEntity,
			TState inFromState,
			TEvent inEvent,
			Argumentos * argu );

			
int Envio_NACK (	TEntity * inEntity,
			TState inFromState,
			TEvent inEvent,
			Argumentos * argu );
			

int finalizar (	TEntity * inEntity,
			TState inFromState,
			TEvent inEvent,
			Argumentos * argu );
