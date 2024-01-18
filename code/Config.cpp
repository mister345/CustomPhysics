#include "Config.h"
#include <cstdio>
#include <cstdarg>

FILE * g_debugFiles[ 2 ] = {};

void openDebugLog( int whichFile ) {
	if ( g_debugFiles[ whichFile ] == nullptr ) {

		char fname[ 256 ];
		sprintf( fname, debugFNames[ whichFile ] );
		int offset = strlen( fname );
		sprintf( fname + offset, ".json" );

		g_debugFiles[ whichFile ] = fopen( fname, "a" );
		fprintf( g_debugFiles[ whichFile ], "{ \"%s\" : [\n", debugFNames[ whichFile ] );
	}
}
void closeDebugLog( int whichFile ) {
	fprintf( g_debugFiles[ whichFile ], "] },\n\n" );
	fclose( g_debugFiles[ whichFile ] );
	g_debugFiles[ whichFile ] = nullptr;
}

void writeToDebugLog( int whichFile, const char * s, ... ) {
	va_list args;
	va_start( args, s );
	vfprintf( g_debugFiles[ whichFile ], s, args );
	va_end( args );
}