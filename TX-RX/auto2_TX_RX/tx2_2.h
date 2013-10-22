
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <IFTI_P2_automat.h>

/* ------------------------------------------------------------ 
 * State definitions
 * ------------------------------------------------------------ */

/*
1  STATE_INITIAL = AUTOMAT_INITIAL_STATE,
2  STATE_WAIT_ACK,
3  STATE_FIN,
4  STATE_ERROR 

*/
enum {
  STATE_INITIAL = AUTOMAT_INITIAL_STATE,
  STATE_WAIT_ACK,
  STATE_FIN_ERROR,
  STATE_FIN,
  STATE_ERROR 

  /*SEM_STATE_CARS_RED,
  SEM_STATE_CARS_YELLOW,
  SEM_STATE_CARS_GREEN,
  SEM_STATE_BATTERIES*/
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
  ACK,
  NACK,
  ACK_0,
  N_NACK_MAX,
  TIME_O

};

/* ------------------------------------------------------------ 
 * Action definitions
 * ------------------------------------------------------------ */
TAutomat * s1_automat_define( );

int imp_error(	TEntity * inEntity,
			TState inFromState,
			TEvent inEvent,
			Argumentos * argu );


int sig_trama   (	TEntity * inEntity,
			TState inFromState,
			TEvent inEvent,
			Argumentos * argu );


int reenviar_trama(	TEntity * inEntity,
			TState inFromState,
			TEvent inEvent,
			Argumentos * argu );

int finalizar (	TEntity * inEntity,
			TState inFromState,
			TEvent inEvent,
			Argumentos * argu );
