#pragma once

#include <vector>
#include <string>

class SkinnedData;
class AnimationInstance;

namespace AnimationAssets {
	enum eSkeleton : uint8_t {
		SINGLE_BONE = 0,
		MULTI_BONE = 1,
		DEBUG_SKELETON = 2,
		SKINNED_MESH = 3,
		INVALID = 4,
	};
	void FillAnimInstanceData( AnimationInstance * animInstance, const char * fileName, float scale );
} // namespace AnimationAssets