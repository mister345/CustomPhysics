#pragma once

#include <fbxsdk.h>

typedef void ( *onLoadedCallback_t )( bool status, fbxsdk::FbxScene * scene, void * userData );

namespace FbxUtil {
	bool IsBone( fbxsdk::FbxNode * node );
	void PrintNode( fbxsdk::FbxNode * pNode );
	void PrintScene( fbxsdk::FbxScene * pScene );
	bool LoadFBXFile( const char * filename, onLoadedCallback_t onLoaded, void * userData );
} // namepsace fbx util