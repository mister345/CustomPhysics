#pragma once

#include <fbxsdk.h>

bool LoadFBXFile( const char * filename, void ( *onLoadedCallback )( bool status, FbxScene * scene ) );
void PrintScene( FbxScene * pScene );