#include "Config.h"
#include <cstdio>
#include <cstdarg>

#ifndef ANIM_DEBUG_LOGGING
void startDebugSession() {}
void endDebugSession() {}
void openDebugLog( int whichFile ) {}
void closeDebugLog( int whichFile ) {}
void writeToDebugLog( int whichFile, const char * s, ... ) {}
#else
char fname0[ 256 ];
char fname1[ 256 ];
char fname2[ 256 ];
char fname3[ 256 ];
char * debugFileNames[ DEBUG_FILE_COUNT ] = { fname0, fname1, fname2, fname3 };
FILE * g_debugFiles[ DEBUG_FILE_COUNT ]   = {};

void startDebugSession() {
	for ( int i = 0; i < DEBUG_FILE_COUNT; i++ ) {
		sprintf( debugFileNames[ i ], debugNames[ i ] );
		int offset = strlen( debugFileNames[ i ] );
		sprintf( debugFileNames[ i ] + offset, ".json" );

		g_debugFiles[ i ] = fopen( debugFileNames[ i ], "a" );
		fprintf( g_debugFiles[ i ], "[" );
		fclose( g_debugFiles[ i ] );
	}
}
void endDebugSession() {
	for ( int i = 0; i < DEBUG_FILE_COUNT; i++ ) {
		sprintf( debugFileNames[ i ], debugNames[ i ] );
		int offset = strlen( debugFileNames[ i ] );
		sprintf( debugFileNames[ i ] + offset, ".json" );

		g_debugFiles[ i ] = fopen( debugFileNames[ i ], "a" );
		fprintf( g_debugFiles[ i ], "]" );
		fclose( g_debugFiles[ i ] );
	}
}

void openDebugLog( int whichFile ) {
	if ( g_debugFiles[ whichFile ] == nullptr ) {
		char fname[ 256 ];
		sprintf( fname, debugNames[ whichFile ] );
		int offset = strlen( fname );
		sprintf( fname + offset, ".json" );

		g_debugFiles[ whichFile ] = fopen( fname, "a" );
		fprintf( g_debugFiles[ whichFile ], "{ \"%s\" : [\n", debugNames[ whichFile ] );
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

#endif // ANIM_DEBUG_LOGGING