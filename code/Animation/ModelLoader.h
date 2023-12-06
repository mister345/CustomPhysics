#pragma once

#include <fbxsdk.h>

typedef void ( *onLoadedCallback_t )( bool status, fbxsdk::FbxScene * scene, void * userData );

namespace FbxUtil {
	typedef void ( *onFoundBoneNode_fn ) ( 
		void * userData,
		fbxsdk::FbxAMatrix & localTransform,
		fbxsdk::FbxVector4 & translation_LS,
		fbxsdk::FbxVector4 & rotation_LS
	);

	bool IsBone( fbxsdk::FbxNode * node );
	void PrintNode( fbxsdk::FbxNode * pNode, onFoundBoneNode_fn onFoundBone );
	void HarvestSceneData( fbxsdk::FbxScene * pScene, void * dataToPopulate, onFoundBoneNode_fn onFoundBone );
	bool LoadFBXFile( const char * filename, onLoadedCallback_t onLoaded, void * userData );
} // namepsace fbx util