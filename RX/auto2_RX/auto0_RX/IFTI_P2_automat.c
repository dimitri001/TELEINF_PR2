/* ----------------------------------------------------------- *
 * Author: Alvaro Paricio Garcia aparicio@aut.auh.es
 * Date: november 2006
 * All rights reserved
 *	Generic automat for communications layers
 * ----------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <IFTI_P2_automat.h>

/* -----------------------------------------------------
 * Create a new automat
 * If some error occurs, return NULL
 * ----------------------------------------------------- */
TAutomat * automat_create(	TState	inToStateWhenUndefined,
				TAction	inDoActionWhenUndefined )
{
  TAutomat * automat = (TAutomat *) calloc( 1, sizeof(TAutomat) );
  if (automat)
  {
    automat->myState_whenUndefined = inToStateWhenUndefined;
    automat->myAction_whenUndefined = inDoActionWhenUndefined;
  }
  return automat;
}

/* -----------------------------------------------------
 * Destroy an automat
 * ----------------------------------------------------- */
int automat_destroy( TAutomat * inAutomat )
{
  int rc = AUTOMAT_OK;

  if ( inAutomat == NULL )
    return (rc=AUTOMAT_NOT_CREATED);

  if ( inAutomat )
	free( inAutomat );
  return rc;
}

/* -----------------------------------------------------
 * Declare a new transition
 * ----------------------------------------------------- */
int automat_declare_transition(	TAutomat *	inAutomat,
				TState		inFromState,
				TState		inToState,
				TEvent		inEvent, 
				TAction		inAction )
{
  int rc = AUTOMAT_OK;

  if ( inAutomat == NULL )
    return (rc=AUTOMAT_NOT_CREATED);

  inAutomat->myMatrix[inEvent][inFromState].toState	= inToState;
  inAutomat->myMatrix[inEvent][inFromState].doAction	= inAction;

  return rc;
}

/* -----------------------------------------------------
 * When an event arrives over an entity, perform the transition
 * defined by the corresponding automat.
 * ----------------------------------------------------- */
int automat_transit(	TEntity *	inEntity,
			TAutomat *	inAutomat,
			TEvent		inEvent,
		        Argumentos  *	inEventArgs
			)
{
  int		rc = AUTOMAT_OK;
  TState	fromState	= AUTOMAT_UNDEFINED_STATE;
  TState	toState		= AUTOMAT_UNDEFINED_STATE;
  TAction	doAction	= AUTOMAT_UNDEFINED_ACTION;

  if ( inAutomat == NULL )
    return (rc=AUTOMAT_NOT_CREATED);

  if ( inEntity == NULL )
    return (rc=ENTITY_NOT_CREATED);

  fromState=inEntity->myCurrentState;
  toState  = inAutomat->myMatrix[inEvent][fromState].toState;
  doAction = inAutomat->myMatrix[inEvent][fromState].doAction;

  /* if the transition has a state change defined, change entity state */
  if ( toState != AUTOMAT_UNDEFINED_STATE )
	inEntity->myCurrentState = toState;
  /* undefined transition: react to undefined conditions */
  else
  {
	inEntity->myCurrentState = inAutomat->myState_whenUndefined;
	if ( inAutomat->myAction_whenUndefined != AUTOMAT_UNDEFINED_ACTION )
	  rc = (*inAutomat->myAction_whenUndefined)( inEntity, fromState, inEvent, inEventArgs );
	else
	{
	  /* no action: do nothing */
	}
  }

  /* if the transition has an action defined, call it */
  if ( doAction != AUTOMAT_UNDEFINED_ACTION )
	rc = (*doAction)( inEntity, fromState, inEvent, inEventArgs );
  else
  {
	/* no action: do nothing */
  }

  /* don't add nothing here */
  return rc;
}
