#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <math.h>

/* Physics includes */
#include "tvector.h"
#include "tmatrix.h"
#include "tray.h"

/*
 * STRINGIZE macro - see Weiss, pages 108-109
 */

#define STRINGIZE( s )		#s

/*
 * This defines compiler values for FAILURE and SUCCESS, as well as
 * convenience macros for testing the return values of functions that
 * return a "boolean" type. Since C does not have true booleans, it 
 * is best to do explicit comparisons to predefined values to 
 * determine the success or failure of a function.
 *
 * These macros simplify that task, while at the same time abstracting
 * away specific details of how the return value is tested.
 */

enum { FAILURE, SUCCESS };

#define SUCCEEDS( m )		( ( m ) != FAILURE )
#define FAILS( m )		( ( m ) == FAILURE )

/*
 * Ditto for UNSET and SET, which can be used for "flag" variables
 */

enum { UNSET, SET };

#define IS_SET( m )		( ( m ) != UNSET )
#define NOT_SET( m )		( ( m ) == UNSET )

/*
 * The ERR_MSG macro prints a message on stderr describing where an
 * error occured and what its cause was (if known)
 */

#define ERR_MSG( func )						\
    { 								\
	( void )fflush( stderr );				\
	( void )fprintf( stderr, __FILE__ ":%d:" #func ": %s\n",\
		__LINE__, strerror( errno ) );			\
    }

/*
 * PRINTF_LIKE macro
 *
 * This macro is a wrapper for functions (like printf(3s))
 * that people tend to want to ignore the return value of.
 * Since printf(3s) can have a variable number of arguments, we
 * must rely on a "trick" to get the macro properly expanded.
 *
 * To use this macro, substitute PRINTF(( ... )) everywhere you
 * would normally use printf( ... ). Note the use of the double
 * parentheses (that is the "trick").
 */

#define PRINTF_LIKE( fn, args, test )	if( fn args test ) ERR_MSG( fn )

/*
 * PRINTF_LIKE functions - these require the (( ... )) "trick"
 * because they take a variable number of arguments
 */

#define PRINTF( args )		PRINTF_LIKE( printf, args, < 0 )
#define FPRINTF( args )		PRINTF_LIKE( fprintf, args, < 0 )
#define SPRINTF( args )		PRINTF_LIKE( sprintf, args, < 0 )

#define SCANF( args )		PRINTF_LIKE( scanf, args, < 0 )
#define FSCANF( args )		PRINTF_LIKE( fscanf, args, < 0 )
#define SSCANF( args )		PRINTF_LIKE( sscanf, args, < 0 )

/*
 * fflush doesn't "technically" need the (( ... )) trick, but it is
 * included for the sake of consistency with the above stdio functions
 */

#define FFLUSH( args )		PRINTF_LIKE( fflush, args, < 0 )

/*
 * The STR_LIKE macro is a wrapper for functions (like strcat(3c))
 * that people tend to want to ignore the return value of. Its main
 * purpose is to check the return value, which keeps lint happy.
 */

#define STR_LIKE( fn, s1, s2 )		if( fn( s1, s2 ) != s1 ) ERR_MSG( fn )

/*
 * The STRN_LIKE macro is a wrapper for functions (like strncat(3c))
 * that people tend to want to ignore the return value of. Its main
 * purpose is to check the return value, which keeps lint happy.
 */

#define STRN_LIKE( fn, s1, s2, n )	if( fn( s1, s2, n ) != s1 ) ERR_MSG( fn )

/*
 * STR_LIKE and STRN_LIKE functions - single parens work fine here
 */

#define STRCAT( s1, s2 )	STR_LIKE( strcat, (s1), (s2) )
#define STRCPY( s1, s2 )	STR_LIKE( strcpy, (s1), (s2) )

#define STRNCAT( s1, s2, n )	STRN_LIKE( strncat, (s1), (s2), (n) )
#define STRNCPY( s1, s2, n )	STRN_LIKE( strncpy, (s1), (s2), (n) )

/* Added Macros */
#define WINX 800
#define WINY 800

#define TITLE "TRING - A Carum Game - Main View"
#define TITLE_3D "TRING - A Carum Game - 3D View"
#define TITLE_SCORE "TRING - A Carum Game - Score"

/* Macros for display size */
#define Vx_max 400
#define Vx_min -400
#define Vy_max 400
#define Vy_min -400
#define Vz_max 800
#define Vz_min -800
#define INFINITY 10000000

/* variables for surface lighting conditions */
GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat mat_shininess[] = { 50.0 };

/* color variables */
GLfloat mat_yellow[] = {1.0, 1.0, 0.0, 0.0};
GLfloat mat_red[] = {1.0, 0.0, 0.0, 0.0};
GLfloat mat_green[] = {0.0, 1.0, 0.0, 0.0};
GLfloat mat_blue[]  = {0.0, 0.0, 1.0, 0.0};
GLfloat mat_white[]  = {1.0, 1.0, 1.0, 0.0};
GLfloat mat_black[]  = {0.0, 0.0, 0.0, 0.0};
GLfloat mat_cue[]  = {0.933, 0.93, 0.55, 0.0};

/* For cue ball hit detection */
enum {RED, YELLOW, BLUE,NONE=-1};




