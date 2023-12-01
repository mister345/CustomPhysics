#pragma once
#include "../Physics/Body.h"

struct BoneTransform {
	Quat rotation = { 0, 0, 0, 1 };
	Vec3 translation = {};
};

struct Keyframe {
	float timePos = 0.f;
	BoneTransform transform;
};

struct BoneAnimation {
	float GetStartTime() const;
	float GetEndTime() const;
	void Interpolate( float t, Quat & outRotation, Vec3 & outTranslation ) const;
	std::vector< Keyframe > keyframes;
};

struct AnimationClip {
	float GetClipStartTime() const;
	float GetClipEndTime() const;
	void Interpolate( float t, std::vector< BoneTransform > & boneTransforms ) const;
};

class SkeletalMesh {
public:
	void Initialize( const Vec3 startPos_WS, Body * bodyToAnimate );
	void Update( float deltaT );

	float animTimePos = 0.f;
	BoneAnimation anim;
	Body * bodyToAnimate = nullptr;

	static Vec3 meshWorldPos;
};

