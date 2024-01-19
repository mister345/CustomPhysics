#pragma once

namespace fbxsdk {
	class FbxNode;
}

namespace FbxNodeParsers {
	void PopulateBoneAnimsCB( void * user, fbxsdk::FbxNode * boneNode );
	void PopulateVertsDataCB( void * user, fbxsdk::FbxNode * meshNode );
	void PopulateBindPoseData( void * user, fbxsdk::FbxNode * meshNode );
} // namespace FbxNodeParsers