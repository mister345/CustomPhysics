#pragma once

#include "../Math/Vector.h"
#include "../Math/Quat.h"

class SkinnedData;
class Body;

struct AnimationInstance {
	SkinnedData * animData = nullptr;

	Vec3 worldPos		     = Vec3( 0, 0, 15 );
	Body * bodiesToAnimate   = nullptr;
	const char * curClipName = AnimationAssets::animNames[ AnimationAssets::COUNT ];
	
	float animTimePos      = 0.f;
	float speedMultiplier  = 1.f;

	AnimationInstance();
	~AnimationInstance();
	void Initialize( Body * bodies, unsigned numBodies, const Vec3 & startPos_WS, const char * clipToPlay );
	void Update( float deltaT );
};

