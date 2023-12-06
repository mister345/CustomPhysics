#pragma once

#include <map>
#include <string>
#include "../Physics/Body.h"
#include "Bone.h"

namespace fbxsdk {
class FbxScene;
}

struct Keyframe {
	float timePos = 0.f;
	BoneTransform transform;
};

struct BoneAnimation {
	float GetStartTime() const;
	float GetEndTime() const;
	void Interpolate( float t, BoneTransform & outTransform ) const;
	inline bool HasData() const { return !keyframes.empty(); }
	std::vector< Keyframe > keyframes;
};

struct AnimationClip {
	// earliest start time of all bones in the clip
	float GetClipStartTime() const;
	// latest end time of all bones in the clip
	float GetClipEndTime() const;
	// loop over each bone and interpolate animation
	void Interpolate( float t, std::vector< BoneTransform > & boneTransforms ) const;

	inline bool HasValidKeyframes() const {
		return GetClipStartTime() >= 0.f;
	}

	// animation for each bone
	std::vector< BoneAnimation > BoneAnimations;
};

class SkinnedData;
namespace AnimationAssets {
	enum eWhichAnim : uint8_t {
		SINGLE_BONE = 0,
		MULTI_BONE = 1,
		SKELETON_ONLY = 2,
		SKINNED_MESH = 3,
		COUNT = 4,
	};
	extern const char * animNames[ eWhichAnim::COUNT + 1 ];

	// util
	BoneAnimation MakeBoneAnim0();
	BoneAnimation MakeBoneAnim1();

	// user interface
	void FillAnimInstanceData( SkinnedData *& skinnedData, const eWhichAnim which, fbxsdk::FbxScene * sceneData = nullptr );
} // namespace AnimationAssets

// database of keyframes & bone offsets + logic to evaluate a pose @ time t - HAS NO STATE!
class SkinnedData {
	friend class AnimationInstance;

public:
	inline uint32_t BoneCount() const { return BoneHierarchy.size(); };

	void Set( const std::vector< int > & boneHierarchy,
			  std::vector< BoneTransform > & boneOffsets,
			  std::map< std::string, AnimationClip > & animations );

	void Set( fbxsdk::FbxScene * scene, const AnimationAssets::eWhichAnim whichAnim );

	void GetFinalTransforms( const std::string & clipName,
							 float timePos,
							 std::vector< BoneTransform > & outFinalTransforms ) const;

private:
	// stored in parent - child order, as a flattened tree
	std::vector< int > BoneHierarchy;
	std::vector< BoneTransform > RefPoseOffsets_ComponentSpace;
	std::map< std::string, AnimationClip > mAnimations;
};