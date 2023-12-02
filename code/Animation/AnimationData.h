#pragma once
#include <map>
#include <string>
#include "../Physics/Body.h"
#include "Bone.h"


struct Keyframe {
	float timePos = 0.f;
	BoneTransform transform;
};

struct BoneAnimation {
	float GetStartTime() const;
	float GetEndTime() const;
	void Interpolate( float t, BoneTransform & outTransform ) const;
	std::vector< Keyframe > keyframes;
};

struct AnimationClip {
	// earliest start time of all bones in the clip
	float GetClipStartTime() const;
	// latest end time of all bones in the clip
	float GetClipEndTime() const;
	// loop over each bone and interpolate animation
	void Interpolate( float t, std::vector< BoneTransform > & boneTransforms ) const;

	// animation for each bone
	std::vector< BoneAnimation > BoneAnimations;
};

class SkinnedData {
	friend class AnimationInstance;

public:
	inline uint32_t BoneCount() const { return BoneHierarchy.size(); };
	void Initialize();
	void Set( const std::vector< int > & boneHierarchy,
			  std::vector< BoneTransform > & boneOffsets,
			  std::map< std::string, AnimationClip > & animations );

	// engine that drives the pose based on the anim data of this class -
	// note that this is const - it HAS NO STATE!
	void GetFinalTransforms( const std::string & clipName, 
							 float timePos, std::vector< BoneTransform > & outFinalTransforms ) const;

private:
	// stored in parent - child order, as a flattened tree
	std::vector< int > BoneHierarchy = {
		-1, 0, 0, 2, 3, 4, 5, 6, 8
	};
	std::map< std::string, AnimationClip > mAnimations;
};