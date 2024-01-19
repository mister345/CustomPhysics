#pragma once

namespace fbxsdk {
	class FbxImporter;
	class FbxScene;
	class FbxNode;
	class FbxQuaternion;
	class FbxVector4;
	class FbxAMatrix;
	class FbxAnimStack;
	class FbxAnimLayer;
	class FbxAnimCurve;
	class FbxSkin;
	class FbxCluster;
}

namespace FbxUtil {
	extern float g_scale;

	typedef void ( *onFoundNode_fn ) ( void * userData, fbxsdk::FbxNode * boneNode );
	typedef void ( *onFoundTPose_fn )( fbxsdk::FbxNode * n, void * me );
	typedef void ( *onLoadedCallback_fn )( bool status, fbxsdk::FbxImporter * pImporter, fbxsdk::FbxScene * scene, void * userData );

	struct callbackAPI_t {
		onFoundNode_fn onFoundBone;
		onFoundNode_fn onFoundMesh;
	};

	void HarvestSceneData( fbxsdk::FbxScene * pScene, const callbackAPI_t & callback, void * caller );
	void HarvestTPose( fbxsdk::FbxScene * pScene, const onFoundTPose_fn & callback, void * caller );
	bool LoadFBXFile( const char * filename, onLoadedCallback_fn onLoaded, void * userData, bool bConvert, float scale = 1.f );
} // namespace fbx util