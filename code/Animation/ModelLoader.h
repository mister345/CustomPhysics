#pragma once

#include <fbxsdk.h>

typedef void ( *onLoadedCallback_t )( bool status, fbxsdk::FbxScene * scene, void * userData );

namespace FbxUtil {
	typedef void ( *onFoundBoneNode_fn ) ( void * userData, fbxsdk::FbxNode * boneNode );

	void ProcessNode( fbxsdk::FbxNode * pNode, onFoundBoneNode_fn onFoundBone, void * dataRecipient );
	void HarvestSceneData( fbxsdk::FbxScene * pScene, bool bConvert, onFoundBoneNode_fn onFoundBone, void * caller );
	bool LoadFBXFile( const char * filename, onLoadedCallback_t onLoaded, void * userData );
} // namepsace fbx util