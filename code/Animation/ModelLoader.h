#pragma once

#include <fbxsdk.h>

typedef void ( *onLoadedCallback_t )( bool status, fbxsdk::FbxScene * scene, void * userData );

bool LoadFBXFile( const char * filename, onLoadedCallback_t onLoaded, void * userData );
void PrintScene( fbxsdk::FbxScene * pScene );