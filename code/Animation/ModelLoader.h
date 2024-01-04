#pragma once

namespace fbxsdk {
	class FbxImporter;
	class FbxScene;
	class FbxNode;
	class FbxQuaternion;
	class FbxVector4;
	class FbxAnimStack;
	class FbxAnimLayer;
	class FbxAnimCurve;
}

namespace FbxUtil {
	extern float g_scale;

	typedef void ( *onFoundNode_fn ) ( void * userData, fbxsdk::FbxNode * boneNode );
	typedef void ( *onLoadedCallback_fn )( bool status, fbxsdk::FbxImporter * pImporter, fbxsdk::FbxScene * scene, void * userData );

	struct callbackAPI_t {
		onFoundNode_fn onFoundBone;
		onFoundNode_fn onFoundMesh;
	};

	void PrintSceneAnimData( fbxsdk::FbxImporter * pImporter );
	void ProcessNode( fbxsdk::FbxNode * pNode, const callbackAPI_t & cb, void * dataRecipient );
	void HarvestSceneData( fbxsdk::FbxScene * pScene, bool bConvert, const callbackAPI_t & callback, void * caller );
	bool LoadFBXFile( const char * filename, onLoadedCallback_fn onLoaded, void * userData, float scale = 1.f );
	int CountBonesInSkeleton( fbxsdk::FbxNode * rootNode );
} // namepsace fbx util