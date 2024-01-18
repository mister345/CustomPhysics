#include "Config.h"
#include <cstdio>
#include <cstdarg>

FILE * g_debugFile = nullptr;

void openDebugLog() {
	if ( g_debugFile == nullptr ) {
		g_debugFile = fopen( "bones.json", "a" );
		fprintf( g_debugFile, "{ OnFoundBoneCB : [\n" );
	}
}
void closeDebugLog() {
	fprintf( g_debugFile, "] }\n\n" );
	fclose( g_debugFile );
	g_debugFile = nullptr;
}

void writeToDebugLog( const char * s, ... ) {
	va_list args;
	va_start( args, s );
	vfprintf( g_debugFile, s, args );
	va_end( args );
}