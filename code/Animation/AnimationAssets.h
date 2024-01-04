#pragma once

#include <vector>
#include <string>

class SkinnedData;
class AnimationInstance;

namespace AnimationAssets {
	enum eSkeleton : uint8_t {
		SINGLE_BONE = 0,
		MULTI_BONE = 1,
		SKELETON_ONLY = 2,
		SKINNED_MESH = 3,
		COUNT = 4,
	};
	void FillAnimInstanceData( AnimationInstance * animInstance, eSkeleton whichSkeleton, const char * fileName, float scale );
} // namespace AnimationAssets