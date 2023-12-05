#pragma once

#include <fbxsdk.h>

bool InitializeSdkObjects( FbxManager *& pManager, FbxImporter *& pImporter );
bool LoadFBXFile( const char * filename );
