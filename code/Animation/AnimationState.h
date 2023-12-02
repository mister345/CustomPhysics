#pragma once

#include "../Math/Vector.h"
#include "../Math/Quat.h"
#include "AnimationData.h"

struct AnimationInstance {
	SkinnedData animData;
	Vec3 worldPos = Vec3( 0, 0, 15 );
	float animTimePos = 0.f;
	Body * bodyToAnimate = nullptr;
	float ANIM_MULTIPLIER = 10.f;

	void Initialize( Body * bodySrc, const Vec3 & startPos_WS );
	void Update( float deltaT );

	const BoneAnimation & GetSingleBoneTestAnim() const;
};

