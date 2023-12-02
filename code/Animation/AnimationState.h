#pragma once

#include "../Math/Vector.h"
#include "../Math/Quat.h"
#include "AnimationData.h"

namespace SingleBoneTest {
	constexpr float TO_RAD = 3.14159265359f / 180.f;
	constexpr const char * ANIM_NAME = "SingleBone";

	extern const std::vector< int > HIERARCHY;
	extern std::vector< BoneTransform > OFFSETS;

	BoneAnimation MakeBoneAnim();
	void PopulateTestAnimData( SkinnedData & skinnedData, const int desiredBoneCount );
} // namespace SingleBoneTest

struct AnimationInstance {
	SkinnedData animData;

	Vec3 worldPos		   = Vec3( 0, 0, 15 );
	Body * bodiesToAnimate = nullptr;
	const char * curClip   = SingleBoneTest::ANIM_NAME;
	
	float animTimePos      = 0.f;
	float speedMultiplier  = 10.f;

	void Initialize( Body * bodies, unsigned numBodies, const Vec3 & startPos_WS, const char * clipToPlay );
	void Update( float deltaT );

	// test funcs
	const BoneAnimation * TryGetSingleBoneTestAnim() const;
};

