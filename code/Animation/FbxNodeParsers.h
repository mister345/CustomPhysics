#pragma once

namespace fbxsdk {
	class FbxNode;
}

namespace FbxNodeParsers {
	void OnFoundBoneCB( void * user, fbxsdk::FbxNode * boneNode );
	void OnFoundMeshCB( void * user, fbxsdk::FbxNode * meshNode );
} // namespace FbxNodeParsers