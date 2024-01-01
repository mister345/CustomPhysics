#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include "Bone.h"
#include "../Physics/Body.h"

namespace fbxsdk {
class FbxScene;
class FbxQuaternion;
class FbxVector4;
class FbxNode;
class FbxAnimStack;
class FbxAnimLayer;
class FbxAnimCurve;
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

	// util
	BoneAnimation MakeBoneAnim0();
	BoneAnimation MakeBoneAnim1();

	// user interface
	void FillAnimInstanceData( SkinnedData *& skinnedData, const eWhichAnim which, fbxsdk::FbxScene * sceneData = nullptr );
} // namespace AnimationAssets

// an array of BoneAnimations corresponding to every bone in a skeleton
struct AnimationClip {
	// earliest start time of all bones in the clip
	float GetClipStartTime() const;
	// latest end time of all bones in the clip
	float GetClipEndTime() const;
	// loop over each bone and interpolate animation
	void Interpolate( float t, std::vector< BoneTransform > & boneTransforms ) const;

	AnimationClip() = default;
	AnimationClip( const int numBones ) { BoneAnimations.resize( numBones ); }

	inline bool HasValidKeyframes() const {
		return GetClipStartTime() >= 0.f;
	}

	// animation for each bone in the skeleton
	std::vector< BoneAnimation > BoneAnimations;
};

// database of keyframes & bone offsets + logic to evaluate a pose @ time t - HAS NO STATE!
class SkinnedData {
	friend struct AnimationInstance;

public:
// INITIALIZATION / LOADING DATA
	inline uint32_t BoneCount() const { 
		assert( OffsetMatrices.size() == BoneHierarchy.size() );
		return BoneHierarchy.size();
	};
	void Set( const std::vector< BoneInfo_t > & boneHierarchy,
			  std::vector< BoneTransform > & boneOffsets,
			  std::map< std::string, AnimationClip > & animations );
	void Set( fbxsdk::FbxScene * scene, const AnimationAssets::eWhichAnim whichAnim );

	static BoneTransform FbxToBoneTransform( fbxsdk::FbxQuaternion * q, const fbxsdk::FbxVector4 * t );
	std::string BoneNameFromCurve( fbxsdk::FbxAnimCurve * curve );
//	void FillBoneAnimKeyframes( fbxsdk::FbxNode * node, fbxsdk::FbxAnimLayer * layer, BoneAnimation & boneAnim );
	void FillBoneAnimKeyframes( fbxsdk::FbxNode * node, fbxsdk::FbxAnimLayer * layer, AnimationClip & clip, int whichBoneIdx );


//	RUNTIME DATA PROCESSING ( Playback )
	void GetFinalTransforms( const std::string & clipName,
							 float timePos,
							 std::vector< BoneTransform > & outFinalTransforms ) const;

public:
	std::map< std::string, AnimationClip > mAnimations;

	// flattened tree of child ( idx ) -> parent ( value of element @ idx ) relationships, in parent-child order
	std::vector< BoneInfo_t > BoneHierarchy;

	// @TODO - will have to expand this to contain ALL layers
	fbxsdk::FbxAnimStack * animStack   = nullptr;
	fbxsdk::FbxAnimLayer * activeLayer = nullptr;
	const char * curAnimName = nullptr;

// INVERSE bind pose matrices of each bone, in MODEL SPACE - they undo the Bind Pose transformations of these bones, 
// so that they can be REPLACED with the ANIMATED Pose ( interpolated bone transforms @ curr time ) 
// https://i0.wp.com/animcoding.com/wp-content/uploads/2021/05/zelda-apply-bind-pose.gif?resize=365%2C519&ssl=1
	std::vector< BoneTransform > OffsetMatrices;
	std::vector< BoneTransform > OffsetMatrices_DIRECT_DEBUG;
	std::unordered_map< std::string, int > BoneIdxMap;
};