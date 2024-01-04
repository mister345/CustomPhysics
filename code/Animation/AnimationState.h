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
	const char * curClipName = "NONE";

	float animTimePos      = 0.f;
	float speedMultiplier  = 2.f;
	std::map< std::string, AnimationClip >::iterator pCurAnim;

	AnimationInstance();
	~AnimationInstance();
	void Initialize( Body * bodies, unsigned numBodies, const Vec3 & startPos_WS );
	void Update( float deltaT );
	const char * CycleCurClip() {
		if ( ++pCurAnim == animData->animations.end() ) {
			pCurAnim = animData->animations.begin();
		}
		curClipName = pCurAnim->first.c_str();
		return curClipName;
	}
};