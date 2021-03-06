  /********************************************************************\
  * BitlBee -- An IRC to other IM-networks gateway                     *
  *                                                                    *
  * Copyright 2002-2004 Wilmer van der Gaast and others                *
  \********************************************************************/

/* Account management functions                                         */

/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License with
  the Debian GNU/Linux distribution in /usr/share/common-licenses/GPL;
  if not, write to the Free Software Foundation, Inc., 59 Temple Place,
  Suite 330, Boston, MA  02111-1307  USA
*/

#define BITLBEE_CORE
#include "bitlbee.h"
#include "account.h"

account_t *account_add( irc_t *irc, struct prpl *prpl, char *user, char *pass )
{
	account_t *a;
	set_t *s;
	
	if( irc->accounts )
	{
		for( a = irc->accounts; a->next; a = a->next );
		a = a->next = g_new0( account_t, 1 );
	}
	else
	{
		irc->accounts = a = g_new0 ( account_t, 1 );
	}
	
	a->prpl = prpl;
	a->user = g_strdup( user );
	a->pass = g_strdup( pass );
	a->auto_connect = 1;
	a->irc = irc;
	
	s = set_add( &a->set, "auto_connect", "true", set_eval_account, a );
	s->flags |= ACC_SET_NOSAVE;
	
	s = set_add( &a->set, "auto_reconnect", "true", set_eval_bool, a );
	
	s = set_add( &a->set, "password", NULL, set_eval_account, a );
	s->flags |= ACC_SET_NOSAVE | SET_NULL_OK;
	
	s = set_add( &a->set, "username", NULL, set_eval_account, a );
	s->flags |= ACC_SET_NOSAVE | ACC_SET_OFFLINE_ONLY;
	set_setstr( &a->set, "username", user );
	
	a->nicks = g_hash_table_new_full( g_str_hash, g_str_equal, g_free, g_free );
	
	/* This function adds some more settings (and might want to do more
	   things that have to be done now, although I can't think of anything. */
	if( prpl->init )
		prpl->init( a );
	
	return( a );
}

char *set_eval_account( set_t *set, char *value )
{
	account_t *acc = set->data;
	
	/* Double-check: We refuse to edit on-line accounts. */
	if( set->flags & ACC_SET_OFFLINE_ONLY && acc->ic )
		return SET_INVALID;
	
	if( strcmp( set->key, "server" ) == 0 )
	{
		g_free( acc->server );
		if( value && *value )
		{
			acc->server = g_strdup( value );
			return value;
		}
		else
		{
			acc->server = g_strdup( set->def );
			return g_strdup( set->def );
		}
	}
	else if( strcmp( set->key, "username" ) == 0 )
	{
		g_free( acc->user );
		acc->user = g_strdup( value );
		return value;
	}
	else if( strcmp( set->key, "password" ) == 0 )
	{
		if( value )
		{
			g_free( acc->pass );
			acc->pass = g_strdup( value );
			return NULL;	/* password shouldn't be visible in plaintext! */
		}
		else
		{
			/* NULL can (should) be stored in the set_t
			   variable, but is otherwise not correct. */
			return SET_INVALID;
		}
	}
	else if( strcmp( set->key, "auto_connect" ) == 0 )
	{
		if( !is_bool( value ) )
			return SET_INVALID;
		
		acc->auto_connect = bool2int( value );
		return value;
	}
	
	return SET_INVALID;
}

account_t *account_get( irc_t *irc, char *id )
{
	account_t *a, *ret = NULL;
	char *handle, *s;
	int nr;
	
	/* This checks if the id string ends with (...) */
	if( ( handle = strchr( id, '(' ) ) && ( s = strchr( handle, ')' ) ) && s[1] == 0 )
	{
		struct prpl *proto;
		
		*s = *handle = 0;
		handle ++;
		
		if( ( proto = find_protocol( id ) ) )
		{
			for( a = irc->accounts; a; a = a->next )
				if( a->prpl == proto &&
				    a->prpl->handle_cmp( handle, a->user ) == 0 )
					ret = a;
		}
		
		/* Restore the string. */
		handle --;
		*handle = '(';
		*s = ')';
		
		if( ret )
			return ret;
	}
	
	if( sscanf( id, "%d", &nr ) == 1 && nr < 1000 )
	{
		for( a = irc->accounts; a; a = a->next )
			if( ( nr-- ) == 0 )
				return( a );
		
		return( NULL );
	}
	
	for( a = irc->accounts; a; a = a->next )
	{
		if( g_strcasecmp( id, a->prpl->name ) == 0 )
		{
			if( !ret )
				ret = a;
			else
				return( NULL ); /* We don't want to match more than one... */
		}
		else if( strstr( a->user, id ) )
		{
			if( !ret )
				ret = a;
			else
				return( NULL );
		}
	}
	
	return( ret );
}

void account_del( irc_t *irc, account_t *acc )
{
	account_t *a, *l = NULL;
	
	if( acc->ic )
		/* Caller should have checked, accounts still in use can't be deleted. */
		return;
	
	for( a = irc->accounts; a; a = (l=a)->next )
		if( a == acc )
		{
			if( l )
				l->next = a->next;
			else
				irc->accounts = a->next;
			
			while( a->set )
				set_del( &a->set, a->set->key );
			
			g_hash_table_destroy( a->nicks );
			
			g_free( a->user );
			g_free( a->pass );
			g_free( a->server );
			if( a->reconnect )	/* This prevents any reconnect still queued to happen */
				cancel_auto_reconnect( a );
			g_free( a );
			
			break;
		}
}

void account_on( irc_t *irc, account_t *a )
{
	if( a->ic )
	{
		/* Trying to enable an already-enabled account */
		return;
	}
	
	cancel_auto_reconnect( a );
	
	a->reconnect = 0;
	a->prpl->login( a );
}

void account_off( irc_t *irc, account_t *a )
{
	imc_logout( a->ic, FALSE );
	a->ic = NULL;
	if( a->reconnect )
	{
		/* Shouldn't happen */
		cancel_auto_reconnect( a );
	}
}

struct account_reconnect_delay
{
	int start;
	char op;
	int step;
	int max;
};

int account_reconnect_delay_parse( char *value, struct account_reconnect_delay *p )
{
	memset( p, 0, sizeof( *p ) );
	/* A whole day seems like a sane "maximum maximum". */
	p->max = 86400;
	
	/* Format: /[0-9]+([*+][0-9]+(<[0-9+]))/ */
	while( *value && isdigit( *value ) )
		p->start = p->start * 10 + *value++ - '0';
	
	/* Sure, call me evil for implementing my own fscanf here, but it's
	   dead simple and I immediately know where to continue parsing. */
	
	if( *value == 0 )
		/* If the string ends now, the delay is constant. */
		return 1;
	else if( *value != '+' && *value != '*' )
		/* Otherwise allow either a + or a * */
		return 0;
	
	p->op = *value++;
	
	/* + or * the delay by this number every time. */
	while( *value && isdigit( *value ) )
		p->step = p->step * 10 + *value++ - '0';
	
	if( *value == 0 )
		/* Use the default maximum (one day). */
		return 1;
	else if( *value != '<' )
		return 0;
	
	p->max = 0;
	value ++;
	while( *value && isdigit( *value ) )
		p->max = p->max * 10 + *value++ - '0';
	
	return p->max > 0;
}

char *set_eval_account_reconnect_delay( set_t *set, char *value )
{
	struct account_reconnect_delay p;
	
	return account_reconnect_delay_parse( value, &p ) ? value : SET_INVALID;
}

int account_reconnect_delay( account_t *a )
{
	char *setting = set_getstr( &a->irc->set, "auto_reconnect_delay" );
	struct account_reconnect_delay p;
	
	if( account_reconnect_delay_parse( setting, &p ) )
	{
		if( a->auto_reconnect_delay == 0 )
			a->auto_reconnect_delay = p.start;
		else if( p.op == '+' )
			a->auto_reconnect_delay += p.step;
		else if( p.op == '*' )
			a->auto_reconnect_delay *= p.step;
		
		if( a->auto_reconnect_delay > p.max )
			a->auto_reconnect_delay = p.max;
	}
	else
	{
		a->auto_reconnect_delay = 0;
	}
	
	return a->auto_reconnect_delay;
}
