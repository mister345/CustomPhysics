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
class FbxScene;
}

namespace AnimationAssets {
	enum eWhichAnim : uint8_t;
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
	inline uint32_t BoneCount() const { 
		assert( OffsetMatrices.size() == BoneHierarchy.size() );
		return BoneHierarchy.size();
	};
	void BoneSpaceToModelSpace( int inBoneIdx, std::vector< BoneTransform > & inOutBoneTransforms ) const;
	static BoneTransform FbxToBoneTransform( fbxsdk::FbxQuaternion * q, const fbxsdk::FbxVector4 * t );

// INITIALIZATION / LOADING
	void Set( const std::vector< BoneInfo_t > & boneHierarchy,
			  std::vector< BoneTransform > & boneOffsets,
			  std::map< std::string, AnimationClip > & animations );
	void Set( fbxsdk::FbxScene * scene, const AnimationAssets::eWhichAnim whichAnim );
	void FillBoneAnimKeyframes( fbxsdk::FbxNode * node, fbxsdk::FbxAnimLayer * layer, AnimationClip & clip, int whichBoneIdx );

// PLAYBACK
	void GetFinalTransforms( const std::string & cName, float time, std::vector<BoneTransform> & outFinalTransforms ) const;

public:
	std::map< std::string, AnimationClip > animations;
 	std::unordered_map< std::string, int > BoneIdxMap;
	std::vector< BoneInfo_t > BoneHierarchy; // flattened tree, parent-child order, idx=bone, element@idx=that bone's parent
// INVERSE bind pose matrices of each bone, in MODEL SPACE - they undo the Bind Pose transformations of these bones, 
// so that they can be REPLACED with the ANIMATED Pose ( interpolated bone transforms @ curr time ) 
// https://i0.wp.com/animcoding.com/wp-content/uploads/2021/05/zelda-apply-bind-pose.gif?resize=365%2C519&ssl=1
	std::vector< BoneTransform > OffsetMatrices;
	std::vector< BoneTransform > OffsetMatrices_DIRECT_DEBUG;

// FBX RELATED
	const char * curAnimName		   = nullptr;
	fbxsdk::FbxScene * fbxScene		   = nullptr;
	fbxsdk::FbxAnimStack * animStack   = nullptr;	
	fbxsdk::FbxAnimLayer * activeLayer = nullptr; // @TODO - expand to contain ALL layers
};