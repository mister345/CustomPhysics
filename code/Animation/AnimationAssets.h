#pragma once

#include <vector>
#include <string>

namespace fbxsdk {
	class FbxScene;
}
class SkinnedData;

namespace AnimationAssets {
	enum eSkeleton : uint8_t {
		SINGLE_BONE = 0,
		MULTI_BONE = 1,
		SKELETON_ONLY = 2,
		SKINNED_MESH = 3,
		COUNT = 4,
	};
	void FillAnimInstanceData( SkinnedData * skinnedData, const eSkeleton which, fbxsdk::FbxScene * sceneData = nullptr );
} // namespace AnimationAssets