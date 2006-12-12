/*
 *  QWProgs-DM
 *  Copyright (C) 2004  [sd] angel
 *
 *  This code is based on Q3 VM code by Id Software, Inc.
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *  $Id: g_utils.c,v 1.71 2006/12/12 01:57:17 qqshka Exp $
 */

#include "g_local.h"

void Sc_Stats(float on);


int PASSFLOAT(float x)
{
	fi_t rc;
	rc._float = x;
	return rc._int;
}


int NUM_FOR_EDICT( gedict_t * e )
{
	int     b;

	b = ( byte * ) e - ( byte * ) g_edicts;
	b = b / sizeof( gedict_t );

	if ( b < 0 || b >= MAX_EDICTS )
		DebugTrap( "NUM_FOR_EDICT: bad pointer" );
	return b;
}


float g_random(  )
{
	return ( rand(  ) & 0x7fff ) / ( ( float ) 0x8000 );
}

float crandom(  )
{
	return 2 * ( g_random(  ) - 0.5 );
}

int i_rnd( int from, int to )
{
	float r;

	if ( from >= to )
		return from;

	r = (int)(from + (1.0 + to - from) * g_random());

	return bound(from, r, to);
}

gedict_t *spawn(  )
{
	gedict_t *t = &g_edicts[trap_spawn(  )];

	if ( !t || t == world )
		DebugTrap( "spawn return world\n" );
	return t;
}

void ent_remove( gedict_t * t )
{
	if ( !t || t == world )
		DebugTrap( "BUG BUG BUG remove world\n" );

	if ( NUM_FOR_EDICT( t ) <= MAX_CLIENTS ) // debug
		G_Error ("remove client");

	trap_remove( NUM_FOR_EDICT( t ) );
}



gedict_t *nextent( gedict_t * ent )
{
	int     entn;

	if ( !ent )
		G_Error( "find: NULL start\n" );
	entn = trap_nextent( NUM_FOR_EDICT( ent ) );
	if ( entn )
		return &g_edicts[entn];
	else
		return NULL;
}

/*gedict_t *find( gedict_t * start, int fieldoff, char *str )
{
	gedict_t *e;
	char   *s;

	if ( !start )
		G_Error( "find: NULL start\n" );
	for ( e = nextent( start ); e; e = nextent( e ) )
	{
		s = *( char ** ) ( ( byte * ) e + fieldoff );
		if ( s && !strcmp( s, str ) )
			return e;
	}
	return NULL;
}*/
gedict_t *find( gedict_t * start, int fieldoff, char *str )
{
	return trap_find( start, fieldoff, str );
}

// find count of "good" edicts
int find_cnt( int fieldoff, char *str )
{
	int cnt;
	gedict_t *p;

	for ( cnt = 0, p = world; (p = find( p, fieldoff, str )); )
		cnt++;

	return cnt;
}

gedict_t *find_idx( int idx, int fieldoff, char *str )
{
	int cnt;
	gedict_t *p;

	if ( idx < 0 )
		return NULL;

	for ( cnt = 0, p = world; (p = find( p, fieldoff, str )); cnt++ )
		if ( cnt == idx )
			break;

	return p;
}


void normalize( vec3_t value, vec3_t newvalue )
{
	float   new;


	new = value[0] * value[0] + value[1] * value[1] + value[2] * value[2];
	new = sqrt( new );

	if ( new == 0 )
		value[0] = value[1] = value[2] = 0;
	else
	{
		new = 1 / new;
		newvalue[0] = value[0] * new;
		newvalue[1] = value[1] * new;
		newvalue[2] = value[2] * new;
	}

}
void aim( vec3_t ret )
{
	VectorCopy( g_globalvars.v_forward, ret );
}

const char    null_str[] = "";

int streq( const char *s1, const char *s2 )
{
	if ( !s1 )
		s1 = null_str;
	if ( !s2 )
		s2 = null_str;

	return ( !strcmp( s1, s2 ) );
}

int strneq( const char *s1, const char *s2 )
{
	if ( !s1 )
		s1 = null_str;
	if ( !s2 )
		s2 = null_str;

	return ( strcmp( s1, s2 ) );
}

int strnull( const char *s1 )
{
	return (!s1 || !*s1);
}

// qqshka - not sure is this a good idea replacing max/min with real function

#ifdef KTX_MIN

float min( float a, float b )
{
	return ( a < b ? a : b );
}

#endif

#ifdef KTX_MAX

float max( float a, float b )
{
	return ( a > b ? a : b );
}

#endif

float bound( float a, float b, float c )
{
	return ( a >= c ? a : b < a ? a : b > c ? c : b);
}

/*
=================
vlen

scalar vlen(vector)
=================
*/
float vlen( vec3_t value1 )
{
//      float   *value1;
	float   new;


	new = value1[0] * value1[0] + value1[1] * value1[1] + value1[2] * value1[2];
	new = sqrt( new );

	return new;
}

float vectoyaw( vec3_t value1 )
{
	float   yaw;


	if ( value1[1] == 0 && value1[0] == 0 )
		yaw = 0;
	else
	{
		yaw = ( int ) ( atan2( value1[1], value1[0] ) * 180 / M_PI );
		if ( yaw < 0 )
			yaw += 360;
	}

	return yaw;
}


void vectoangles( vec3_t value1, vec3_t ret )
{
	float   forward;
	float   yaw, pitch;


	if ( value1[1] == 0 && value1[0] == 0 )
	{
		yaw = 0;
		if ( value1[2] > 0 )
			pitch = 90;
		else
			pitch = 270;
	} else
	{
		yaw = ( int ) ( atan2( value1[1], value1[0] ) * 180 / M_PI );
		if ( yaw < 0 )
			yaw += 360;

		forward = sqrt( value1[0] * value1[0] + value1[1] * value1[1] );
		pitch = ( int ) ( atan2( value1[2], forward ) * 180 / M_PI );
		if ( pitch < 0 )
			pitch += 360;
	}


	ret[0] = pitch;
	ret[1] = yaw;
	ret[2] = 0;
}


/*
=================
Returns a chain of entities that have origins within a spherical area

findradius (origin, radius)
=================
*/
#if 0 // worked, but unused since we have trap_findradius
gedict_t *findradius( gedict_t * start, vec3_t org, float rad )
{
	gedict_t *ent;
	vec3_t  eorg;
	int     j;

	for ( ent = nextent( start ); ent; ent = nextent( ent ) )
	{
		if ( ent->s.v.solid == SOLID_NOT )
			continue;
		for ( j = 0; j < 3; j++ )
			eorg[j] = org[j] - ( ent->s.v.origin[j] + ( ent->s.v.mins[j] + ent->s.v.maxs[j] ) * 0.5 );
		if ( VectorLength( eorg ) > rad )
			continue;
		return ent;

	}
	return NULL;

}

// just ignore solid field
gedict_t *findradius2( gedict_t * start, vec3_t org, float rad )
{
	gedict_t *ent;
	vec3_t  eorg;
	int     j;

	for ( ent = nextent( start ); ent; ent = nextent( ent ) )
	{
//		if ( ent->s.v.solid == SOLID_NOT )
//			continue;
		for ( j = 0; j < 3; j++ )
			eorg[j] = org[j] - ( ent->s.v.origin[j] + ( ent->s.v.mins[j] + ent->s.v.maxs[j] ) * 0.5 );
		if ( VectorLength( eorg ) > rad )
			continue;
		return ent;

	}
	return NULL;

}
#endif
/*
==============
changeyaw

This was a major timewaster in progs, so it was converted to C
==============
*/
void changeyaw( gedict_t * ent )
{
	float   ideal, current, move, speed;

	current = anglemod( ent->s.v.angles[1] );
	ideal = ent->s.v.ideal_yaw;
	speed = ent->s.v.yaw_speed;

	if ( current == ideal )
		return;
	move = ideal - current;
	if ( ideal > current )
	{
		if ( move >= 180 )
			move = move - 360;
	} else
	{
		if ( move <= -180 )
			move = move + 360;
	}
	if ( move > 0 )
	{
		if ( move > speed )
			move = speed;
	} else
	{
		if ( move < -speed )
			move = -speed;
	}

	ent->s.v.angles[1] = anglemod( current + move );
}

/*
==============
PF_makevectors

Writes new values for v_forward, v_up, and v_right based on angles
makevectors(vector)
==============
*/
/* replaced with trap_makevectors
void makevectors( vec3_t vector )
{
	AngleVectors( vector, g_globalvars.v_forward, g_globalvars.v_right, g_globalvars.v_up );
}
*/

/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday
============
*/

char	*va(char *format, ...)
{
	va_list		argptr;
	static char		string[MAX_STRINGS*2][1024]; // qqshka - brrr
	static int		index = 0;
	
	index %= MAX_STRINGS;
	va_start (argptr, format);
	vsnprintf (string[index], sizeof(string[0]), format, argptr);
	va_end (argptr);

	string[index][ sizeof( string[0] ) - 1 ] = '\0';

	return string[index++];
}

char *redtext(const char *format, ...)
{
// >>>> like va(...)
	va_list		argptr;
	static char	string[MAX_STRINGS][1024];
	static int		index = 0;
	
	index %= MAX_STRINGS;
	va_start (argptr, format);
	vsnprintf (string[index], sizeof(string[0]), format, argptr);
	va_end (argptr);

	string[index][ sizeof( string[0] ) - 1 ] = '\0';
// <<<<

	{ // convert to red
		unsigned char *i = (unsigned char *) string[index];

		for ( ; *i; i++ )
			if ( *i > 32 && *i < 128 )
				*i |= 128;

		return string[index++];
	}
}

char *dig3(int d)
{
	static char	string[MAX_STRINGS][32];
	static int		index = 0;

	index %= MAX_STRINGS;

	snprintf(string[index], sizeof( string[0] ), "%d", d);
	string[index][ sizeof( string[0] ) - 1 ] = '\0';

	{ // convert digits
		unsigned char *i = (unsigned char *) (string[index]);

		for ( ; *i; i++ )
			if ( *i >= '0' && *i <= '9' )
				*i += 98;
	}

	return string[index++];
}

char *dig3s(const char *format, ...)
{
// >>>> like va(...)
	va_list		argptr;
	static char	string[MAX_STRINGS][32];
	static int		index = 0;
	
	index %= MAX_STRINGS;
	va_start (argptr, format);
	vsnprintf (string[index], sizeof(string[0]), format, argptr);
	va_end (argptr);

	string[index][ sizeof( string[0] ) - 1 ] = '\0';
// <<<<

	{ // convert digits
		unsigned char *i = (unsigned char *) (string[index]);

		for ( ; *i; i++ )
			if ( *i >= '0' && *i <= '9' )
				*i += 98;
	}

	return string[index++];
}

char *striphigh(const char *format, ...)
{
// >>>> like va(...)
	va_list		argptr;
	static char	string[MAX_STRINGS][1024];
	static int		index = 0;
	
	index %= MAX_STRINGS;
	va_start (argptr, format);
	vsnprintf (string[index], sizeof(string[0]), format, argptr);
	va_end (argptr);

	string[index][ sizeof( string[0] ) - 1 ] = '\0';
// <<<<

	{
		unsigned char *i = (unsigned char *) string[index];

		for ( ; *i; i++ )
				*i &= 127;

		return string[index++];
	}
}


/*
==============
print functions
==============
*/
void G_sprint( gedict_t * ed, int level, const char *fmt, ... )
{
	va_list argptr;
	char    text[1024];

	va_start( argptr, fmt );
	vsprintf( text, fmt, argptr );
	va_end( argptr );

	trap_SPrint( NUM_FOR_EDICT( ed ), level, text );
}

void G_bprint( int level, const char *fmt, ... )
{
	va_list argptr;
	char    text[1024];

	va_start( argptr, fmt );
	vsprintf( text, fmt, argptr );
	va_end( argptr );

	trap_BPrint( level, text );
}

void G_cprint( const char *fmt, ... )
{
	va_list argptr;
	char    text[1024*4];

	va_start( argptr, fmt );
	vsnprintf( text, sizeof(text), fmt, argptr );
	va_end( argptr );

	text[sizeof(text)-1] = 0;
	
	trap_conprint( text );
}

void G_centerprint( gedict_t * ed, const char *fmt, ... )
{
	va_list argptr;
	char    text[1024];

	va_start( argptr, fmt );
	vsprintf( text, fmt, argptr );
	va_end( argptr );

	trap_CenterPrint( NUM_FOR_EDICT( ed ), text );
}

// centerprint too all clients
void G_cp2all(const char *fmt, ... )
{
	va_list argptr;
	char    text[1024];

	va_start( argptr, fmt );
	vsnprintf (text, sizeof(text), fmt, argptr);
	text[sizeof(text)-1] = 0;
	va_end( argptr );

	WriteByte(MSG_ALL, SVC_CENTERPRINT);
	WriteString(MSG_ALL, text);
}


void G_dprint( const char *fmt, ... )
{
	va_list argptr;
	char    text[1024];

	va_start( argptr, fmt );
	vsprintf( text, fmt, argptr );
	va_end( argptr );

	trap_DPrintf( text );
}

void localcmd( const char *fmt, ... )
{
	va_list argptr;
	char    text[1024];

	va_start( argptr, fmt );
	vsprintf( text, fmt, argptr );
	va_end( argptr );

	trap_localcmd( text );
}

void stuffcmd( gedict_t * ed, const char *fmt, ... )
{
	va_list argptr;
	char    text[1024];

	va_start( argptr, fmt );
	vsprintf( text, fmt, argptr );
	va_end( argptr );

	trap_stuffcmd( NUM_FOR_EDICT( ed ), text );
}


void setorigin( gedict_t * ed, float origin_x, float origin_y, float origin_z )
{
	trap_setorigin( NUM_FOR_EDICT( ed ), origin_x, origin_y, origin_z );
}

void setsize( gedict_t * ed, float min_x, float min_y, float min_z, float max_x, float max_y, float max_z )
{
	trap_setsize( NUM_FOR_EDICT( ed ), min_x, min_y, min_z, max_x, max_y, max_z );
}

void setmodel( gedict_t * ed, char *model )
{
	trap_setmodel( NUM_FOR_EDICT( ed ), model );
}

void sound( gedict_t * ed, int channel, char *samp, float vol, float att )
{
	if ( !samp || !*samp )
		return; // ignore null sample

	trap_sound( NUM_FOR_EDICT( ed ), channel, samp, vol, att );
}

gedict_t *checkclient(  )
{
	return &g_edicts[trap_checkclient(  )];
}
void traceline( float v1_x, float v1_y, float v1_z, float v2_x, float v2_y, float v2_z, int nomonst, gedict_t * ed )
{
	trap_traceline( v1_x, v1_y, v1_z, v2_x, v2_y, v2_z, nomonst, NUM_FOR_EDICT( ed ) );
}

void TraceCapsule( float v1_x, float v1_y, float v1_z, float v2_x, float v2_y, float v2_z, int nomonst, gedict_t * ed ,
			float min_x, float min_y, float min_z, 
			float max_x, float max_y, float max_z)
{
	trap_TraceCapsule( v1_x, v1_y, v1_z, v2_x, v2_y, v2_z, nomonst, NUM_FOR_EDICT( ed ) ,
	min_x, min_y, min_z, max_x, max_y, max_z);
}

int droptofloor( gedict_t * ed )
{
	return trap_droptofloor( NUM_FOR_EDICT( ed ) );
}

int checkbottom( gedict_t * ed )
{
	return trap_checkbottom( NUM_FOR_EDICT( ed ) );
}

void makestatic( gedict_t * ed )
{
	trap_makestatic( NUM_FOR_EDICT( ed ) );
}

void setspawnparam( gedict_t * ed )
{
	trap_setspawnparam( NUM_FOR_EDICT( ed ) );
}

void logfrag( gedict_t * killer, gedict_t * killee )
{
	trap_logfrag( NUM_FOR_EDICT( killer ), NUM_FOR_EDICT( killee ) );
}

// WARNING: this function doest support 'cmd info' keys and this is MUST be so
char *infokey( gedict_t * ed, char *key, char *valbuff, int sizebuff )
{
	trap_infokey( NUM_FOR_EDICT( ed ), key, valbuff, sizebuff );

	return valbuff;
}

char *ezinfokey( gedict_t * ed, char *key )
{
	static char		string[MAX_STRINGS][1024];
	static int		index = 0;

	if ( ed->ct == ctPlayer || ed->ct == ctSpec ) {
		char *v = cmdinfo_getkey( ed, key );

		if ( v ) // key supported so does't need to search in userinfo even key is empty ""
			return v;
	}

	index %= MAX_STRINGS;

	trap_infokey( NUM_FOR_EDICT( ed ), key, string[index], sizeof( string[0] ) );

	return string[index++];
}

int  iKey( gedict_t * ed, char *key )
{
	char		string[128]; // which size will be best?

	if ( ed->ct == ctPlayer || ed->ct == ctSpec ) {
		char *v = cmdinfo_getkey( ed, key );

		if ( v ) // key supported so does't need to search in userinfo even key is empty ""
			return atoi( v );
	}

	trap_infokey( NUM_FOR_EDICT( ed ), key, string, sizeof( string ) );
	return atoi( string );
}

float fKey( gedict_t * ed, char *key )
{
	char		string[128]; // which size will be best?

	if ( ed->ct == ctPlayer || ed->ct == ctSpec ) {
		char *v = cmdinfo_getkey( ed, key );

		if ( v ) // key supported so does't need to search in userinfo even key is empty ""
			return atof( v );
	}

	trap_infokey( NUM_FOR_EDICT( ed ), key, string, sizeof( string ) );
	return atof( string );
}

void WriteEntity( int to, gedict_t * ed )
{
	trap_WriteEntity( to, NUM_FOR_EDICT( ed ) );
}

void WriteByte( int to, int data )
{
	trap_WriteByte( to, data );
}

void WriteShort( int to, int data )
{
	trap_WriteShort( to, data );
}

void WriteLong( int to, int data )
{
	trap_WriteLong( to, data );
}

void WriteString( int to, char *data )
{
	trap_WriteString( to, data );
}

void WriteAngle( int to, float data )
{
	trap_WriteAngle( to, data );
}

void WriteCoord( int to, float data )
{
	trap_WriteCoord( to, data );
}

void disableupdates( gedict_t * ed, float time )
{
	trap_disableupdates( NUM_FOR_EDICT( ed ), time );
}

int walkmove( gedict_t * ed, float yaw, float dist )
{
	gedict_t*saveself,*saveother,*saveactivator;
	int retv;

	saveself	= self ;
	saveother	= other;
	saveactivator = activator;

	retv = trap_walkmove( NUM_FOR_EDICT( ed ), yaw,  dist );

	self 	= saveself;
	other	= saveother;
	activator= saveactivator;
	return retv;
}

float cvar( const char *var )
{
	if ( strnull( var ) )
		G_Error("cvar null");

	return trap_cvar ( var );
}

char *cvar_string( const char *var )
{
	static char		string[MAX_STRINGS][1024];
	static int		index = 0;

	index %= MAX_STRINGS;

	trap_cvar_string( var, string[index], sizeof( string[0] ) );

	return string[index++];
}

void cvar_set( const char *var, const char *val )
{
	if ( strnull( var ) || val == NULL )
		G_Error("cvar_set null");

	trap_cvar_set( var, val );
}

void cvar_fset( const char *var, float val )
{
	if ( strnull( var ) )
		G_Error("cvar_fset null");

	trap_cvar_set_float(var, val);
}

int getteams(char teams[MAX_CLIENTS][MAX_TEAM_NAME])
{
	char *team;
	int i, j;

	// clear array
	for ( i = 0; i < MAX_CLIENTS; i++ )
		teams[i][0] = 0;

	for ( i = 1; i <= MAX_CLIENTS; i++ ) {
		if ( g_edicts[i].ct != ctPlayer )
			continue; // collect only players teams

		team = getteam( &(g_edicts[i]) );

		if ( strnull( team ) )
			continue; // empty team

		for ( j = 0; j < MAX_CLIENTS; j++ ) {
			if ( strnull( teams[j] ) ) {
				strlcat(teams[j], team, MAX_TEAM_NAME); // add new team to array
				break;
			}

			if ( streq( teams[j], team ) )
				break; // team already in array
		}
	}

	for ( j = 0; j < MAX_CLIENTS; j++ )
		if ( strnull( teams[j] ) )
			break;

	return j;
}

// i'm tired of this shit, so implement this
// return team of edict, edict may has "player" or "ghost" classname
char *getteam( gedict_t * ed )
{
	static char		string[MAX_STRINGS][128];
	static int		index = 0;
	char 			*team=NULL;
	int 			num = (int)(ed - g_edicts);

	index %= MAX_STRINGS;

	if ( num >= 1 && num <= MAX_CLIENTS )
		team = ezinfokey(ed, "team");
	else if ( streq(ed->s.v.classname, "ghost") )
		team = ezinfokey(world, va("%d", (int)ed->k_teamnum));
	else {
//		G_Error("getteam: wrong classname %s", ed->s.v.classname);
		team = "";
	}

	string[index][0] = 0;
	strlcat( string[index], team, sizeof( string[0] ) );

	return string[index++];
}

char *getname( gedict_t * ed )
{
	static char		string[MAX_STRINGS][1024];
	static int		index = 0;
	char 			*name=NULL;
	int 			num = (int)(ed - g_edicts);

	index %= MAX_STRINGS;

	if ( num >= 1 && num <= MAX_CLIENTS )
		name = ed->s.v.netname;
	else if ( streq(ed->s.v.classname, "ghost") )
		name = ezinfokey(world, va("%d", (int)ed->cnt2));
	else {
		name = "";
//		G_Error("getname: wrong classname %s", ed->s.v.classname);
	}

	string[index][0] = 0;
	strlcat( string[index], name, sizeof( string[0] ) );

	return string[index++];
}

// return "his" or "her" depend on gender of player
char *g_his( gedict_t * ed )
{
	static char		string[MAX_STRINGS][5];
	static int		index = 0;
	char 			*sex="his";

	index %= MAX_STRINGS;

//	if ( ed->ct != ctPlayer && ed->ct != ctSpec )
//		G_Error("g_his: not client, classname %s", ed->s.v.classname);
	if( streq( ezinfokey( ed, "gender" ), "f" ) )
		sex = "her";

	string[index][0] = 0;
	strlcat( string[index], sex, sizeof( string[0] ) );

	return string[index++];
}

// return "he" or "she" depend on gender of player
char *g_he( gedict_t * ed )
{
	static char		string[MAX_STRINGS][5];
	static int		index = 0;
	char 			*sex="he";

	index %= MAX_STRINGS;

//	if ( ed->ct != ctPlayer && ed->ct != ctSpec )
//		G_Error("g_his: not client, classname %s", ed->s.v.classname);
	if( streq( ezinfokey( ed, "gender" ), "f" ) )
		sex = "she";

	string[index][0] = 0;
	strlcat( string[index], sex, sizeof( string[0] ) );

	return string[index++];
}

// return "himself" or "herself" depend on gender of player
char *g_himself( gedict_t * ed )
{
	static char		string[MAX_STRINGS][9];
	static int		index = 0;
	char 			*sex="himself";

	index %= MAX_STRINGS;

//	if ( ed->ct != ctPlayer && ed->ct != ctSpec )
//		G_Error("g_himself: not client, classname %s", ed->s.v.classname);
	if( streq( ezinfokey( ed, "gender" ), "f" ) )
		sex = "herself";

	string[index][0] = 0;
	strlcat( string[index], sex, sizeof( string[0] ) );

	return string[index++];
}

gedict_t *find_client( gedict_t *start )
{
	for ( ; (start = trap_nextclient(start)); )
		if ( start->ct == ctPlayer || start->ct == ctSpec )
			return start;

	return NULL;
}

gedict_t *find_plr( gedict_t *start )
{
	for ( ; (start = trap_nextclient(start)); )
		if ( start->ct == ctPlayer )
			return start;

	return NULL;
}

gedict_t *find_spc( gedict_t *start )
{
	for ( ; (start = trap_nextclient(start)); )
		if ( start->ct == ctSpec )
			return start;

	return NULL;
}

// this help me walk from both players and ghosts, made code more simple
// int from = 0;
// gedict_t *p = world ;
// while ( p = find_plrghst(p, &from) ) {
// ... some code ...
// }


gedict_t *find_plrghst( gedict_t * start, int *from )
{
	gedict_t *next = ( *from ? find(start, FOFCLSN, "ghost") : find_plr( start ) );

	if ( !next && !*from ) {
		*from = 1;
		next = find(world, FOFCLSN, "ghost");
	}

	return next;
}

// this help me walk from both players and specs, made code more simple

gedict_t *find_plrspc( gedict_t * start, int *from )
{
	gedict_t *next = ( *from ? find_spc( start ) : find_plr( start ) );

	if ( !next && !*from ) {
		*from = 1;
		next = find_spc( world );
	}

	return next;
}

gedict_t *player_by_id( int id )
{
	gedict_t *p;

	if ( id < 1 )
		return NULL;

	for ( p = world; (p = find_plr( p )); ) {
		if ( id == GetUserID( p ) )
			return p;
	}

	return NULL;
}

gedict_t *player_by_name( const char *name )
{
	gedict_t *p;

	if ( strnull( name ) )
		return NULL;

	for ( p = world; (p = find_plr( p )); ) {
		if ( streq(p->s.v.netname, name) )
			return p;
	}

	return NULL;
}

gedict_t *player_by_IDorName( const char *IDname )
{
	gedict_t *p = player_by_id( atoi( IDname ) );

	return (p ? p : player_by_name( IDname ) );
}

gedict_t *spec_by_id( int id )
{
	gedict_t *p;

	if ( id < 1 )
		return NULL;

	for ( p = world; (p = find_spc( p )); ) {
		if ( id == GetUserID( p ) )
			return p;
	}

	return NULL;
}

gedict_t *spec_by_name( const char *name )
{
	gedict_t *p;

	if ( strnull( name ) )
		return NULL;

	for ( p = world; (p = find_spc( p )); ) {
		if ( streq(p->s.v.netname, name) )
			return p;
	}

	return NULL;
}

gedict_t *spec_by_IDorName( const char *IDname )
{
	gedict_t *p = spec_by_id( atoi( IDname ) );

	return (p ? p : spec_by_name( IDname ) );
}

gedict_t *SpecPlayer_by_IDorName( const char *IDname )
{
	gedict_t *p = player_by_IDorName( IDname );

	return (p ? p : spec_by_IDorName( IDname ));
}

gedict_t *SpecPlayer_by_id( int id )
{
	gedict_t *p = spec_by_id( id );

	return (p ? p : player_by_id( id ) );
}

gedict_t *not_connected_by_id( int id )
{
	char *statk;
	gedict_t *p;

	for( p = g_edicts + 1; p <= g_edicts + MAX_CLIENTS; p++ )
		if ( (   streq(statk = ezinfokey(p, "*state"), "preconnected")
			  || streq(statk, "connected")
			 ) && iKey(p, "*userid") == id  // can't use GetUserID here
		   )
			return p;

	return NULL;
}

gedict_t *not_connected_by_name( const char *name )
{
	char *statk;
	gedict_t *p;

	for( p = g_edicts + 1; p <= g_edicts + MAX_CLIENTS; p++ )
		if ( (   streq(statk = ezinfokey(p, "*state"), "preconnected")
			  || streq(statk, "connected")
			 ) && streq(p->s.v.netname, name)
		   )
			return p;

	return NULL;
}

gedict_t *not_connected_by_IDorName( const char *IDname )
{
	gedict_t *p = not_connected_by_id( atoi( IDname ) );

	return (p ? p : not_connected_by_name( IDname ) );
}

char *armor_type( int items )
{
	static char		string[MAX_STRINGS][4];
	static int		index = 0;
	char 			*at;

	index %= MAX_STRINGS;

	if(	     items & IT_ARMOR1 )
		at = "ga";
	else if( items & IT_ARMOR2 )
		at = "ya";
	else if( items & IT_ARMOR3 )
		at = "ra";
	else
		at = "0";

	string[index][0] = 0;
	strlcat( string[index], at, sizeof( string[0] ) );

	return string[index++];
}

qboolean isghost( gedict_t *ed )
{
	return (streq(ed->s.v.classname, "ghost") ? true : false);
}
// gametype >>>
qboolean isDuel( )
{
	return (k_mode == gtDuel ? true : false);
}

qboolean isTeam( )
{
	return (k_mode == gtTeam ? true : false);
}

int tp_num()
{
	return ( (isTeam() || isCTF()) ? teamplay : 0);
}

qboolean isFFA( )
{
	return (k_mode == gtFFA ? true : false);
}

qboolean isCTF( )
{
#ifdef CTF_RELOADMAP
	return k_ctf; // once setup at map load
#else
	return (k_mode == gtCTF ? true : false);
#endif
}

qboolean isUnknown( )
{
#ifdef CTF_RELOADMAP
	if ( cvar("k_mode") == gtCTF )
		return false; // zzzz, hack, let FixRules work less or more correctly

	return ((!isDuel() && !isTeam() && !isFFA() && !isCTF()) ? true : false);
#else
	return ((!isDuel() && !isTeam() && !isFFA() && !isCTF()) ? true : false);
#endif
}

// <<< gametype

int GetUserID( gedict_t *p )
{
	if ( !p || (p->ct != ctPlayer && p->ct != ctSpec) )
		return 0;

	return iKey(p, "*userid");
}

// get name of player whom spectator 'p' tracking
// if something wrong returned value is ""
char *TrackWhom( gedict_t *p )
{
	static char		string[MAX_STRINGS][32];
	static int		index = 0;
	char 			*name;
	gedict_t 		*goal = NULL;

	index %= MAX_STRINGS;

	if (  p && p->ct == ctSpec
			&& (goal = PROG_TO_EDICT( p->s.v.goalentity )) != world
			&& goal->ct == ctPlayer
       )
		name = getname(goal);
    else
		name = "";

	string[index][0] = 0;
	strlcat( string[index], name, sizeof( string[0] ) );

	return string[index++];
}

int GetHandicap( gedict_t *p )
{
	int hdc = p->ps.handicap < 1 ? 100 : min( 100, p->ps.handicap );

	if ( cvar( "k_lock_hdp" ) )
		return 100;
	else
		return hdc;
}

qboolean SetHandicap( gedict_t *p, int nhdc )
{
	int hdc = GetHandicap( p );

	nhdc = nhdc < 1 ? 100 : min( 100, nhdc );	

	if ( match_in_progress )
		return false;

	if ( cvar( "k_lock_hdp" ) ){
		G_sprint(self, 2, "%s changes are not allowed\n", redtext("handicap"));
		return false;	
	}

	if ( nhdc != hdc ){
		p->ps.handicap = nhdc;
		if ( nhdc == 100 )
			G_bprint(2, "%s turns %s off\n", p->s.v.netname, redtext("handicap"));
		else
			G_bprint(2, "%s uses %s %d%%\n", p->s.v.netname, redtext("handicap"), nhdc);
		return true;
	}

	return false;
}

void changelevel( const char *name )
{
 	if ( strnull( name ) )
		G_Error("changelevel: null");

	trap_changelevel(name);
}

int Get_Powerups ()
{
	static float k_pow_check = 0;
	static int   k_pow = 0;

	int k_pow_new         = cvar( "k_pow" ); // sure - here we not using Get_Powerups %)
	int k_pow_min_players = bound(0, cvar( "k_pow_min_players"), 999);
	int k_pow_check_time  = bound(0, cvar( "k_pow_check_time"), 999);

	k_pow_check_time = !k_pow_check_time ? 10 : k_pow_check_time; // default is 10

	if ( !k_pow_new || !k_matchLess || !k_pow_min_players )
		return k_pow = k_pow_new; // no k_pow_min_players if server in normal match mode
								  // no powerups if k_pow == 0
								  // return current value of key 'k_pow' if k_pow_min_players == 0

	if ( k_pow_check > g_globalvars.time )
		return k_pow; // too soon to re-check, so return result of last check

	// ok, time to re-check if powerups is still actual, or we have lack of players

	// because not all players may connected yet, some work around :|
	if ( framecount == 1 ) {
		k_pow_check = g_globalvars.time + 3; // fast re-check. probably time when all players reconnect
		k_pow_new = max(cvar("_k_players"), CountPlayers()) < k_pow_min_players ? 0 : k_pow_new;
		k_pow = cvar( "_k_pow_last" ); // restore k_pow from last level
	}else {
		k_pow_check = g_globalvars.time + k_pow_check_time;
		k_pow_new = CountPlayers() < k_pow_min_players ? 0 : k_pow_new;
	}

	if ( k_pow != k_pow_new ) {
		char *pwr;

		switch ( k_pow_new ) {
			case 0:  pwr = redtext("Off"); break;
			case 1:  pwr = redtext("On");  break;
			case 2:  pwr = redtext("On (Jam)"); break;
			default: pwr = redtext("Unkn"); break;
		}
		
		G_bprint(2, "Server decides to turn %s %s\n", redtext("powerups"), pwr);
	}

	return (k_pow = k_pow_new);
}

char *count_s( int cnt )
{
	return (cnt == 1 ? "" : "s");
}

char *Enables( float f )
{
	return ( f ? "enables" : "disables" );
}

char *Enabled( float f )
{
	return ( f ? "enabled" : "disabled" );
}

char *Allows( float f )
{
	return ( f ? "allows" : "disallows" );
}

char *Allowed( float f )
{
	return ( f ? "allowed" : "disallowed" );
}

char *OnOff( float f )
{
	return ( f ? "on" : "off" );
}

// { some scores stuff

// for team games
int k_scores1 = 0;
int k_scores2 = 0;

// for ffa and duel
gedict_t *ed_scores1 = NULL;
gedict_t *ed_scores2 = NULL;

void ReScores()
{
	gedict_t *p;
	int from;
	char *team1, *team2;

	// DO this by checking if k_nochange is 0. 
	// which is set in ClientObituary in client.c
	if( k_nochange )
		return;

	// ok - scores potentially changed, recalculate

	k_nochange = 1;

	k_scores1 = 0;
	k_scores2 = 0;

	if ( k_showscores ) {
		team1 = cvar_string( "_k_team1" );

		for( from = 0, p = world; (p = find_plrghst( p, &from )); ) {
			team2 = getteam(p);

			if( streq( team1, team2 ) )
				k_scores1 += p->s.v.frags;
			else
				k_scores2 += p->s.v.frags;
		}
	}

	ed_scores1 = NULL;
	ed_scores2 = NULL;
	
	if ( ( isDuel() || isFFA() ) && CountPlayers() > 1 ) {
		// no ghost serving
		for ( p = world; (p = find_plr( p )); ) {
			if ( !ed_scores1 ) { // set some first player as best player
				ed_scores1 = p;
				continue;
			}

			if ( ed_scores1->s.v.frags < p->s.v.frags ) { // seems first player is must be second and player 'p' is must be first
				ed_scores2 = ed_scores1;
				ed_scores1 = p;
				continue;
			}

			if ( !ed_scores2 || ed_scores2->s.v.frags < p->s.v.frags ) { // seemd player 'p' must be second
				ed_scores2 = p;
				continue;
			}
		}

		if ( !ed_scores1 || !ed_scores2 ) {
			ed_scores1 = NULL;
			ed_scores2 = NULL;
		}
	}
}

int get_scores1()
{
	ReScores();

	return k_scores1;
}

int get_scores2()
{
	ReScores();

	return k_scores2;
}

gedict_t *get_ed_scores1()
{
	ReScores();

	return ed_scores1;
}

gedict_t *get_ed_scores2()
{
	ReScores();

	return ed_scores2;
}

// }

// { version stuff

// stolen from MVDSV, k? :]
// ok :-D //VVD

static char *date = __DATE__;
static char *mon[12] =
    { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static char mond[12] =
    { 31,    28,    31,    30,    31,    30,    31,    31,    30,    31,    30,    31 };

// returns days since Feb 12 2006
int build_number ()
{
	int m = 0;
	int d = 0;
	int y = 0;
	static int b = 0;

	if (b != 0)
		return b;

	for (m = 0; m < 11; m++)
	{
		if (    ( date[0+0] && mon[m][0] && date[0+0] == mon[m][0] )
			 && ( date[0+1] && mon[m][1] && date[0+1] == mon[m][1] )
			 && ( date[0+2] && mon[m][2] && date[0+2] == mon[m][2] )
		   )
			break;
		d += mond[m];
	}

	d += atoi( &date[4] ) - 1;
	y = atoi( &date[7] ) - 1900;
	b = d + (int)((y - 1) * 365.25);

	if (((y % 4) == 0) && m > 1)
		b += 1;

	b -= 38393; // Feb 12 2006

	return b;
}

// }

// { // autotrack stuff

gedict_t *ed_best1 = NULL;
gedict_t *ed_best2 = NULL;
gedict_t *ed_bestPow = NULL;

void CalculateBestPlayers()
{
	gedict_t *p;
	int best, best1, best2;

	// ok - best povs potentially changed, recalculate

	best1 = 0;
	best2 = 0;
	ed_best1 = NULL;
	ed_best2 = NULL;
	ed_bestPow = NULL;

	// autotrack stuff
	// no ghost serving
	for ( p = world; (p = find_plr( p )); ) {

		if ( ISDEAD(p) )
			continue;

/*
	Pentagram of Protection     3000
	Quad Damage                 2000
	any other Powerup           2000
	Rocket Launcher with ammo   1000
	Lightning Gun with ammo      500
	Grenade Launcher with ammo   200
	Super Nailgun with ammo      100
	Super Shotgun with ammo       50
*/
		best = 0;
		best += ( p->invincible_finished >= g_globalvars.time )   ? 3000 : 0; // pent
		best += ( p->super_damage_finished >= g_globalvars.time ) ? 2000 : 0; // quad
		best += (   p->invisible_finished >= g_globalvars.time
				 || p->radsuit_finished >= g_globalvars.time  )   ? 2000 : 0; // ring or suit
		best += ( ((int)p->s.v.items & IT_ROCKET_LAUNCHER)
				 				   && p->s.v.ammo_rockets > 0 )   ? 1000 : 0; // rl with ammo
		best += ( ((int)p->s.v.items & IT_LIGHTNING)
				 				   && p->s.v.ammo_cells > 0   )   ?  500 : 0; // lg with ammo
		best += ( ((int)p->s.v.items & IT_GRENADE_LAUNCHER)
				 				   && p->s.v.ammo_rockets > 0 )   ?  200 : 0; // gl with ammo
		best += ( ((int)p->s.v.items & IT_SUPER_NAILGUN)
				 				   && p->s.v.ammo_nails > 0   )   ?  100 : 0; // sng with ammo
		best += ( ((int)p->s.v.items & IT_SUPER_SHOTGUN)
				 				   && p->s.v.ammo_shells > 0  )   ?   50 : 0; // ssg with ammo
		best += p->s.v.frags;
	
		if ( !ed_best1 ) { // select some first player as best
			ed_best1 = p;
			best1 = best;
			continue;
		}

		if ( best1 < best ) { // seems first player is must be second and player 'p' is must be first
			ed_best2 = ed_best1;
			ed_best1 = p;
			best2 = best1;
			best1 = best;
			continue;
		}

		if ( !ed_best2 || best2 < best ) { // seems player 'p' must be second
			ed_best2 = p;
			best2 = best;
			continue;
		}
	}

	// auto_pow stuff
	// no ghost serving
	for ( p = world; (p = find_plr( p )); ) {

		if ( ISDEAD(p) )
			continue;

/*
	Pentagram of Protection    4000
	Quad Damage                2000
	Ring                       1000
	Suit						500
*/
		best = 0;
		best += ( p->invincible_finished >= g_globalvars.time )   ? 4000 : 0; // pent
		best += ( p->super_damage_finished >= g_globalvars.time ) ? 2000 : 0; // quad
		best += ( p->invisible_finished >= g_globalvars.time )    ? 1000 : 0; // ring
		best += ( p->radsuit_finished >= g_globalvars.time )      ?  500 : 0; // suit
		best += p->s.v.frags;
	
		if ( !ed_bestPow || best1 < best ) {
			ed_bestPow = p;
			best1 = best;
			continue;
		}
	}
}

gedict_t *get_ed_best1()
{
	return ed_best1;
}

gedict_t *get_ed_best2()
{
	return ed_best2;
}

gedict_t *get_ed_bestPow()
{
	return ed_bestPow;
}

// }

void show_sv_version()
{
	char *tm;

	if ( !strnull( ezinfokey( self, "*is" ) ) ) // show this only ones at connect time
		return;

	if ( !strnull( tm = ezinfokey(world, "date_str") ) )
		G_sprint(self, 2, "Date: %s\n", tm);
	G_sprint(self, 2, "Mod: %s v.%s build:%05d\n", MOD_NAME, MOD_VERSION, build_number());
	G_sprint(self, 2, "Server: %s\n", cvar_string( "version" ));

	if ( (int)cvar( "sv_specprint" ) & SPECPRINT_SPRINT )
		G_sprint(self, PRINT_CHAT, "\x87\x87\x87WARNING: spectators may see team messages (mm2) on this server!\n");
}


char *str_noweapon(int k_disallow_weapons)
{
	static char	string[MAX_STRINGS][128];
	static int		index = 0;

	index %= MAX_STRINGS;

	string[index][0] = 0;

	if (k_disallow_weapons & IT_AXE)
		strlcat(string[index], " axe", sizeof( string[0] ));
	if (k_disallow_weapons & IT_SHOTGUN)
		strlcat(string[index], " sg", sizeof( string[0] ));
	if (k_disallow_weapons & IT_SUPER_SHOTGUN)
		strlcat(string[index], " ssg", sizeof( string[0] ));
	if (k_disallow_weapons & IT_NAILGUN)
		strlcat(string[index], " ng", sizeof( string[0] ));
	if (k_disallow_weapons & IT_SUPER_NAILGUN)
		strlcat(string[index], " sng", sizeof( string[0] ));
	if (k_disallow_weapons & IT_GRENADE_LAUNCHER)
		strlcat(string[index], " gl", sizeof( string[0] ));
	if (k_disallow_weapons & IT_ROCKET_LAUNCHER)
		strlcat(string[index], " rl", sizeof( string[0] ));
	if (k_disallow_weapons & IT_LIGHTNING)
		strlcat(string[index], " lg", sizeof( string[0] ));

	return string[index++];
}

void cvar_toggle_msg( gedict_t *p, char *cvarName, char *msg )
{
    int i;

	if ( strnull( cvarName ) )
		return;

    i = !cvar( cvarName );

	if ( !strnull( msg ) )
    	G_bprint(2, "%s %s %s\n", p->s.v.netname, Enables( i ), msg);

	trap_cvar_set_float( cvarName, (float)i );
}

// generally - this check if we can read file - but used for exec command
qboolean can_exec( char *name )
{
	fileHandle_t handle;

	if ( trap_FS_OpenFile( name, &handle, FS_READ_BIN ) >= 0 ) {
		trap_FS_CloseFile( handle );
		return true;
	}

	return false;
}

// { ghosts stuff

void ghostClearScores( gedict_t *g )
{
	int to = MSG_ALL;
	int cl_slot = g->ghost_slot;

	if ( strneq( g->s.v.classname, "ghost" ) )
		return;

	if ( cl_slot < 1 || cl_slot > MAX_CLIENTS )
		return;

    if ( !strnull( g_edicts[ cl_slot ].s.v.netname ) )
			return; // slot is busy - does't clear

	g_edicts[ cl_slot ].ghost_slot = 0; // mark it as free
	cl_slot--;

	WriteByte(to, SVC_UPDATEUSERINFO); // update userinfo
	WriteByte(to, cl_slot);            // client number
	WriteLong(to, 0);                  // client userid
	WriteString(to, "\\name\\");
}

void ghost2scores( gedict_t *g )
{
	int to = MSG_ALL;
	int cl_slot;

	if ( strneq( g->s.v.classname, "ghost" ) )
		return;

 	cl_slot = g->ghost_slot; // try restore

	if ( cl_slot < 1 || cl_slot > MAX_CLIENTS || !strnull( g_edicts[ cl_slot ].s.v.netname ) )
		cl_slot	= 0; // slot was occupied or wrong

 	// check is restore possible
	if ( cl_slot < 1 ) {
		for( cl_slot = 1; cl_slot <= MAX_CLIENTS; cl_slot++ ) {
			if ( g_edicts[ cl_slot ].ghost_slot )
				continue; // some ghost is already uses this slot

        	if ( strnull( g_edicts[ cl_slot ].s.v.netname ) )
				break;
		}
	}

	if ( cl_slot > MAX_CLIENTS )
		return; // no free slot for ghost

	g_edicts[ cl_slot ].ghost_slot = cl_slot; // mark it as not free
	g->ghost_slot = cl_slot; // save slot - so we can clear/restore it in future

	cl_slot--;

	WriteByte(to, SVC_UPDATEUSERINFO); // update userinfo
	WriteByte(to, cl_slot);            // client number
	WriteLong(to, 0);                  // client userid
	WriteString(to, va("\\name\\\x83 %s\\team\\%s\\topcolor\\%d\\bottomcolor\\%d",
			 		getname( g ), getteam( g ),
			 		bound(0, ((g->ghost_clr >> 8) & 0xF), 13),
			 		bound(0, (g->ghost_clr & 0xF), 13)));

	WriteByte(to, SVC_UPDATEFRAGS); // update frags
	WriteByte(to, cl_slot);
	WriteShort(to, g->s.v.frags);

	WriteByte(to, SVC_UPDATEENTERTIME);				// update time
	WriteByte(to, cl_slot);  						// client number
// FIXME: !!! qqshka: - must be WriteFloat but API have not it - so use WriteLong - dunno is this ok
	WriteLong(to, PASSFLOAT(g_globalvars.time - g->ghost_dt)); // client enter time - here time since player was dropped

	WriteByte(to, SVC_UPDATEPING);      // update ping
	WriteByte(to, cl_slot);      		// client number
	WriteShort(to, 39);  				// client ping
}

void update_ghosts ()
{
	gedict_t *p;
	int from;

	for( from = 1, p = world; (p = find_plrghst( p, &from )); )
		ghost2scores( p );
}

// } ghost stuff

// { events

void on_connect()
{
	char *newteam;

	if ( !(iKey(self, "ev") & EV_ON_CONNECT) ) // client doesn't want on_connect
		return;

	if ( self->ct == ctPlayer ) {
		if ( isFFA() )
			stuffcmd(self, "on_connect_ffa\n");
		else if ( isCTF() )
			stuffcmd(self, "on_connect_ctf\n");
		else
			stuffcmd(self, "on_connect\n");

		if ( isCTF() && ( streq(newteam = getteam(self), "red") || streq(newteam, "blue") ) )
			stuffcmd(self, "auto%s\n", newteam); 
	}
	else {
		if ( isFFA() )
			stuffcmd(self, "on_observe_ffa\n");
		else if ( isCTF() )
			stuffcmd(self, "on_observe_ctf\n");
		else
			stuffcmd(self, "on_observe\n");
	}
}

void on_enter()
{
	if ( iKey(self, "kf") & KF_ON_ENTER ) // client doesn't want on_enter 
		return;

	if ( self->ct == ctPlayer ) {
		if ( isFFA() )
			stuffcmd(self, "on_enter_ffa\n");
		else if ( isCTF() )
			stuffcmd(self, "on_enter_ctf\n");
		else
			stuffcmd(self, "on_enter\n");
	}
	else {
		if ( isFFA() )
			stuffcmd(self, "on_spec_enter_ffa\n");
		else if ( isCTF() )
			stuffcmd(self, "on_spec_enter_ctf\n");
		else
			stuffcmd(self, "on_spec_enter\n");
	}
}

void on_match_start( gedict_t *p )
{
	if ( !(iKey(p, "ev") & EV_ON_MATCH_START) )
		return;

	if ( p->ct == ctPlayer ) {
		stuffcmd(p, "on_matchstart\n");
	}
	else {
		stuffcmd(p, "on_spec_matchstart\n");
	}
}

void on_match_end( gedict_t *p )
{
	if ( !(iKey(p, "ev") & EV_ON_MATCH_END) )
		return;

	if ( p->ct == ctPlayer ) {
		stuffcmd(p, "on_matchend\n");
	}
	else {
		stuffcmd(p, "on_spec_matchend\n");
	}
}

void on_match_break( gedict_t *p )
{
	if ( !(iKey(p, "ev") & EV_ON_MATCH_BREAK) )
		return;

	if ( p->ct == ctPlayer ) {
		stuffcmd(p, "on_matchbreak\n");
	}
	else {
		stuffcmd(p, "on_spec_matchbreak\n");
	}
}

void on_admin( gedict_t *p )
{
	if ( !(iKey(p, "ev") & EV_ON_ADMIN) )
		return;

	stuffcmd(p, "on_admin\n");
}

void on_unadmin( gedict_t *p )
{
	if ( !(iKey(p, "ev") & EV_ON_UNADMIN) )
		return;

	stuffcmd(p, "on_unadmin\n");
}

void ev_print( gedict_t *p, int new_ev, int old_ev, int bit, char *msg)
{
	int on;

	if ( ( on = (new_ev & bit) ) != (old_ev & bit) )
		G_sprint( p, 2, "%s%s\n", msg, OnOff( on ));
}

void info_ev_update ( gedict_t *p, char *from, char *to )
{
	int new_ev = atoi( to );
	int old_ev = atoi( from );

	ev_print( p, new_ev, old_ev, EV_ON_CONNECT,     "[on_connect] event: ");
	ev_print( p, new_ev, old_ev, EV_ON_MATCH_START, "[on_matchstart] event: ");
	ev_print( p, new_ev, old_ev, EV_ON_MATCH_END,   "[on_matchend] event: ");
	ev_print( p, new_ev, old_ev, EV_ON_MATCH_BREAK, "[on_matchbreak] event: ");
	ev_print( p, new_ev, old_ev, EV_ON_ADMIN,       "[on_admin] event: ");
	ev_print( p, new_ev, old_ev, EV_ON_UNADMIN,     "[on_unadmin] event: ");
}

void info_kf_update ( gedict_t *p, char *from, char *to )
{
	int new_ev = atoi( to );
	int old_ev = atoi( from );

	ev_print( p,  new_ev,  old_ev, KF_KTSOUNDS, "KTSounds: ");
	ev_print( p,  new_ev,  old_ev, KF_SCREEN,   "auto screenshot: ");
	ev_print( p, ~new_ev, ~old_ev, KF_ON_ENTER, "calling on_enter user alias: "); // heh ~ is ok
	ev_print( p,  new_ev,  old_ev, KF_SPEED,    "showing speed in prewar: ");
}

// }

void cl_refresh_plus_scores (gedict_t *p)
{
	gedict_t *swp;

	if ( (p->ct == ctPlayer || p->ct == ctSpec) && p->sc_stats ) {
			swp = self; // save self
			self = p;

			Sc_Stats( 2 ); // force refresh

			self = swp; // restore self
	}
}

void refresh_plus_scores ()
{
	gedict_t *p;

	for( p = world; (p = find_client( p )); )
		cl_refresh_plus_scores( p );
}

int only_digits(const char *s)
{
	if ( !s || *s == '\0' )
		return (0);

	while ( *s != '\0' )
	{
		if (!isdigit(*s))
			return (0);
		s++;
	}
	return (1);
}

// params_str( 0, -1 ) return all params
char *params_str( int from, int to )
{
	static char		string[MAX_STRINGS][1024];
	static int		index = 0;

	char arg_x[1024];
	int i, argc = trap_CmdArgc();

	from = max(0, from);
	to   = ( to < 0 ? argc-1: min(argc-1, to) );

	if ( !argc || from >= argc || from > to )
		return "";

	index %= MAX_STRINGS;

	for ( string[index][0] = 0, i = from; i <= to; i++ ) {
		trap_CmdArgv( i, arg_x, sizeof( arg_x ) );

		if ( i != from )
			strlcat( string[index], " ", sizeof( string[0] ) );
		strlcat( string[index], arg_x, sizeof( string[0] ) );
	}

	return string[index++];
}

char *SD_type_str()
{
	switch ( (int)k_sudden_death ) {
		case         0:   return "none";
		case SD_NORMAL:   return "Sudden death";
		case SD_TIEBREAK: return "tie-break";
		default:		  return "unknown";
	}
}

char *respawn_model_name( int mdl_num )
{
	switch ( mdl_num ) {
		case 0:  return "Normal QW respawns";
		case 1:  return "KT SpawnSafety";
		case 2:  return "Kombat Teams respawns";
		case 3:  return "KTX respawns";
		default: return "!Unknown!";
	}
}

