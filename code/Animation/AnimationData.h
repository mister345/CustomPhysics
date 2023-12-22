#pragma once

#include <map>
#include <string>
#include "../Physics/Body.h"
#include "Bone.h"

namespace fbxsdk {
class FbxScene;
class FbxQuaternion;
class FbxVector4;
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

	// animation for each bone in the skeleton
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
	friend struct AnimationInstance;

public:
	inline uint32_t BoneCount() const { return OffsetMatrices.size(); };

	static BoneTransform FbxToBoneTransform( fbxsdk::FbxQuaternion * q, const fbxsdk::FbxVector4 * t );

	void Set( const std::vector< BoneInfo_t > & boneHierarchy,
			  std::vector< BoneTransform > & boneOffsets,
			  std::map< std::string, AnimationClip > & animations );

	void Set( fbxsdk::FbxScene * scene, const AnimationAssets::eWhichAnim whichAnim );

	void GetFinalTransforms( const std::string & clipName,
							 float timePos,
							 std::vector< BoneTransform > & outFinalTransforms ) const;

public:
	std::map< std::string, AnimationClip > mAnimations;

	// stored in parent - child order, as a flattened tree
	// NOTE - if we want a flat hierarchy, it is the CONTENT CREATOR's job to add an identity root bone
	// for consistency, we will still "concatenate" all the bones w that identity, and the hierarchy will look like this:
	// const std::vector< int > HIERARCHY = { -1, 0, 0, 0, 0, 0, ... }; // ( could be used as a particle shader )
//	std::vector< int > BoneHierarchy;

	std::vector< BoneInfo_t > BoneHierarchy;

// https://www.gamedevs.org/uploads/skinned-mesh-and-character-animation-with-directx9.pdf
// Each bone in the skeleton has a corresponding offset matrix.
// An offset matrix transforms vertices, in the bind pose, from bind space 
// to the space of the respective bone ( WHEN THAT BONE IS IN BIND-POSE )

// NOTE - these offset matrices ( HORRIBLE fucking name ), are the INVERSE bind pose matrices of each bone,
// in MODEL SPACE ( see gif - https://i0.wp.com/animcoding.com/wp-content/uploads/2021/05/zelda-apply-bind-pose.gif?resize=365%2C519&ssl=1 )
// - they undo the Bind Pose transformations of these bones, so that they can be REPLACED with the ANIMATED Pose transformations instead
	std::vector< BoneTransform > OffsetMatrices;

	// @TODO - add this exclusively for debug
	//std::vector< BoneTransform > RefPoseTransforms;
};