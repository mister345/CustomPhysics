#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include "Bone.h"
#include "../Physics/Body.h"

struct vert_t;

namespace fbxsdk {
class FbxScene;
class FbxNode;
class FbxQuaternion;
class FbxVector4;
class FbxAnimStack;
class FbxAnimLayer;
class FbxAnimCurve;
}

namespace AnimationAssets {
	enum eSkeleton : uint8_t;
}

////////////////////////////////////////////////////////////////////////////////
// ANIMATION CLIP
////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
// SKINNED DATA
////////////////////////////////////////////////////////////////////////////////
// database of keyframes & bone offsets + logic to evaluate a pose @ time t - HAS NO STATE ( except for while loading )
class SkinnedData {
	friend class AnimationInstance;

public:
	SkinnedData( AnimationAssets::eSkeleton whichSkeleton ) : 
		skeletonType( whichSkeleton ) {}
	~SkinnedData() {
		if ( renderedVerts != nullptr ) {
			free( renderedVerts );
			renderedVerts = nullptr;
		}
		if ( idxes != nullptr ) {
			free( idxes );
			idxes = nullptr;
		}
	}

	inline uint32_t BoneCount() const { return boneNameToIdx.size(); };
	void BoneSpaceToModelSpace( int inBoneIdx, std::vector< BoneTransform > & inOutBoneTransforms ) const;

// INITIALIZATION / LOADING
	void Set( const std::vector< BoneInfo_t > & boneHierarchy,
			  std::vector< BoneTransform > & boneOffsets,
			  std::map< std::string, AnimationClip > & animations );

	/*	https://www.gamedev.net/articles/programming/graphics/how-to-work-with-fbx-sdk-r3582/
	https://stackoverflow.com/questions/45690006/fbx-sdk-skeletal-animations
NOTE - 2 ways to get frames from fbx file
	1. Store key frames only ( raw animation data from within the fbx file )
	2. Sample frames from the fbx file at a fixed rate - loses raw animation data,
		bc its resampling the original keyframes according to some arbitrary logic.
	-> we use way #2 because much more straightforward ( not easy to match curves to bones )
	*/
	void Set( fbxsdk::FbxScene * scene );

	void GetFinalTransforms_v2( const std::string & cName, float time, std::vector<BoneTransform> & outFinalTransforms ) const;

// PLAYBACK
	void GetFinalTransforms( const std::string & cName, float time, std::vector<BoneTransform> & outFinalTransforms ) const;

public:
	int numVerts = 0;
	vertSkinned_t * renderedVerts = nullptr;
	int numIdxes = 0;
	int * idxes = nullptr;

	AnimationAssets::eSkeleton skeletonType;
 	std::unordered_map< std::string, int > boneNameToIdx;
 	std::unordered_map< int, std::string > boneIdxToName;
	std::vector< BoneInfo_t > BoneHierarchy; // flattened tree, parent-child order, idx=bone, element@idx=that bone's parent
	std::map< std::string, AnimationClip > animations;

// INVERSE bind pose matrices of each bone, in MODEL SPACE - they undo the Bind Pose transformations of these bones, 
// so that they can be REPLACED with the ANIMATED Pose ( interpolated bone transforms @ curr time ) 
// https://i0.wp.com/animcoding.com/wp-content/uploads/2021/05/zelda-apply-bind-pose.gif?resize=365%2C519&ssl=1
	std::vector< BoneTransform > InvBindPoseMatrices;

	// for debug purposes only ( setting a skeleton to t-pose )
	std::vector< BoneTransform > BindPoseMatrices;

	fbxsdk::FbxScene * fbxScene = nullptr;
};