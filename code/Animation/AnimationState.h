#pragma once

#include "../Math/Vector.h"
#include "../Math/Quat.h"
#include "../Physics/Body.h"
#include "AnimationAssets.h"

class SkinnedData;

////////////////////////////////////////////////////////////////////////////////
// ANIMATION INSTANCE
////////////////////////////////////////////////////////////////////////////////
struct AnimationInstance {
	float animTimePos = 0.f;
	float speedMultiplier = 2.f;
	SkinnedData * animData = nullptr;
	std::map< std::string, AnimationClip >::iterator pCurAnim;

	Vec3 worldPos = Vec3( 0, 0, 15 );
	std::vector< Body > bodiesToAnimate;

	bool isInstanced = true;

	AnimationInstance( const Vec3 & worldPos );
	~AnimationInstance();
	void Update( float deltaT );
	const char * GetCurClipName();
	const char * CycleCurClip();
};