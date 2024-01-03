#pragma once

#include "../Math/Vector.h"
#include "../Math/Quat.h"
#include "AnimationAssets.h"

class SkinnedData;
class Body;

////////////////////////////////////////////////////////////////////////////////
// ANIMATION INSTANCE
////////////////////////////////////////////////////////////////////////////////
struct AnimationInstance {
	SkinnedData * animData = nullptr;

	Vec3 worldPos		     = Vec3( 0, 0, 15 );
	Body * bodiesToAnimate   = nullptr;
	const char * curClipName = AnimationAssets::animNames[ AnimationAssets::COUNT ].c_str();
	
	float animTimePos      = 0.f;
	float speedMultiplier  = 2.f;

	AnimationInstance();
	~AnimationInstance();
	void Initialize( Body * bodies, unsigned numBodies, const Vec3 & startPos_WS, const char * clipToPlay );
	void Update( float deltaT );
};