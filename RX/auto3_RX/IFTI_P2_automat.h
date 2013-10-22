#ifndef IFTI_P2_AUTOMAT_INCLUDE_H
#define IFTI_P2_AUTOMAT_INCLUDE_H
/* ----------------------------------------------------------- *
 * Author: Alvaro Paricio Garcia aparicio@aut.auh.es
 * Date: november 2006
 * All rights reserved
 * ----------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cabeceras.h"

/* The state is just an ENUM type */
typedef unsigned int	TState;

/* The event is just an ENUM type */
typedef unsigned int	TEvent;

/* A transitable entity */
typedef struct { TState	myCurrentState;
		 void *	myData;
		} TEntity;


/* Whenever an event comes, it could have some arguments attached */
typedef struct
{
    frame *trama_e;
    packet *paquete;
    int fde;
    TEntity *active_en;
    int retardo;
    int n_rtx;
    int tempo;
    int tr_perdidas;
    int tra_retx;
    char sec_actual;
    char sec_siguiente;
    char sec_ACK;
    char sec_NAK;
}Argumentos;

/* Whenever a transition ocuurs the following action function (callback)
 * will be called */
typedef int	(*TAction)( TEntity * inEntity, TState inFromState, TEvent inEvent, Argumentos * argu );

//typedef void		TEventArgs;


/* A transition is determined by the current state of the automat and the
 * incoming event */
typedef struct TTransition {
		 TState		toState;
		 TAction	doAction;
		 } TTransition;


/* The transition matrix TAutomat is made of a 2 dimensional array of
 * 	events x fromStates
 * The matrix size is limited for the moment */
#define IFTI_P2_AUTOMAT_MAX_EVENTS	50
#define IFTI_P2_AUTOMAT_MAX_STATES	50

/* A transition matrix is a two entries structure that sets what will occur
 * next. The pair (current_state, incoming_event) determines what will be the
 * next state and what to do then.
 */

#define	AUTOMAT_UNDEFINED_ACTION	0
#define	AUTOMAT_UNDEFINED_STATE		0
#define	AUTOMAT_INITIAL_STATE		1


/* An automat has a transition matrix and a default state and action to
 * invoice when the selected pair (current_state, incoming_event) is empty. 
 */
typedef struct TAutomat {
  TTransition	myMatrix[IFTI_P2_AUTOMAT_MAX_EVENTS][IFTI_P2_AUTOMAT_MAX_STATES];
  TState	myState_whenUndefined;
  TAction	myAction_whenUndefined;
} TAutomat;


/* Reset to 0 all the automat struct */
#define	TAutomat_reset(X) memset(&X,0x00,sizeof(TAutomat))

/* ================================================================== *
 *			ERROR CODES
 * ================================================================== */

/* No error */
#define	AUTOMAT_OK			0

/* No automat was already created (null) */
#define	AUTOMAT_NOT_CREATED		1

/* No entity was already created (null) */
#define	ENTITY_NOT_CREATED		2

/* ================================================================== *
 *			FUNCTIONAL API
 * ================================================================== */

/* -----------------------------------------------------
 * Create a new automat. At creation time, set the state and action
 * for undefined transitions.
 * If some error occurs, return NULL
 * ----------------------------------------------------- */
TAutomat * automat_create(	TState	inToStateWhenUndefined,
				TAction	inDoActionWhenUndefined );

/* -----------------------------------------------------
 * Destroy an automat
 * ----------------------------------------------------- */
int automat_destroy( TAutomat * inAutomat );

/* -----------------------------------------------------
 * Declare a new transition
 * ----------------------------------------------------- */
int automat_declare_transition(	TAutomat *	inAutomat,
				TState		inFromState,
				TState		inToState,
				TEvent		inEvent, 
				TAction		inAction );

/* -----------------------------------------------------
 * When an event arrives over an entity, perform the transition
 * defined by the corresponding automat.
 * ----------------------------------------------------- */
int automat_transit(	TEntity *	inEntity,
			TAutomat *	inAutomat,
			TEvent		inEvent,
			Argumentos *	argu
			);

#endif /* IFTI_P2_AUTOMAT_INCLUDE_H */
