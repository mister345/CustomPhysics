#pragma once

#include "../Math/Vector.h"
#include "../Math/Quat.h"
#include "AnimationData.h"

namespace AnimationAssets {
	enum eWhichAnim : uint8_t {
		SINGLE_BONE = 0,
		MULTI_BONE = 1,
		REAL_MESH = 2,
		SKINNED_MESH = 3,
		COUNT = 4,
	};
	extern const char * animNames[ eWhichAnim::COUNT + 1 ];

	// util
	BoneAnimation MakeBoneAnim0();
	BoneAnimation MakeBoneAnim1();
	
	// user interface
	void MakeAnimInstanceData( SkinnedData & skinnedData, const eWhichAnim which );
} // namespace SingleBoneTest

struct AnimationInstance {
	SkinnedData animData;

	Vec3 worldPos		     = Vec3( 0, 0, 15 );
	Body * bodiesToAnimate   = nullptr;
	const char * curClipName = AnimationAssets::animNames[ AnimationAssets::SINGLE_BONE ];
	
	float animTimePos      = 0.f;
	float speedMultiplier  = 1.f;

	void Initialize( Body * bodies, unsigned numBodies, const Vec3 & startPos_WS, const char * clipToPlay );
	void Update( float deltaT );
};

