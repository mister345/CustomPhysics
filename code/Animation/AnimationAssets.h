#pragma once

#include <vector>
#include <string>

namespace fbxsdk {
	class FbxScene;
}
class SkinnedData;

namespace AnimationAssets {
	enum eWhichAnim : uint8_t {
		SINGLE_BONE = 0,
		MULTI_BONE = 1,
		SKELETON_ONLY = 2,
		SKINNED_MESH = 3,
		COUNT = 4,
	};
	extern std::vector< std::string > animNames;

	void FillAnimInstanceData( SkinnedData *& skinnedData, const eWhichAnim which, fbxsdk::FbxScene * sceneData = nullptr );
} // namespace AnimationAssets