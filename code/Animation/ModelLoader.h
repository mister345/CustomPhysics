#pragma once

/*
https://help.autodesk.com/view/FBX/2018/ENU/?guid=FBX_Developer_Help_getting_started_installing_and_configuring_configuring_the_fbx_sdk_for_wind_html
*/
#define FBXSDK_SHARED

#include <fbxsdk.h>

namespace FbxUtil {
	extern float g_scale;

	typedef void ( *onFoundBoneNode_fn ) ( void * userData, fbxsdk::FbxNode * boneNode );
	typedef void ( *onFoundAnimNode_fn ) ( void * userData, fbxsdk::FbxNode * animNode );
	typedef void ( *onLoadedCallback_fn )( bool status, fbxsdk::FbxImporter * pImporter, fbxsdk::FbxScene * scene, void * userData );

	struct callbackAPI_t {
		onFoundBoneNode_fn onFoundBone;
		onFoundAnimNode_fn onFoundAnimCurve;
	};

	void PrintSceneAnimData( fbxsdk::FbxImporter * pImporter );
	void ProcessNode( fbxsdk::FbxNode * pNode, const callbackAPI_t & cb, void * dataRecipient );
	void HarvestSceneData( fbxsdk::FbxScene * pScene, bool bConvert, const callbackAPI_t & callback, void * caller );
	bool LoadFBXFile( const char * filename, onLoadedCallback_fn onLoaded, void * userData, float scale = 1.f );
} // namepsace fbx util