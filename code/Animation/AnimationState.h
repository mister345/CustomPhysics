#pragma once

#include "../Math/Vector.h"
#include "../Math/Quat.h"
#include "AnimationAssets.h"

class SkinnedData;
class Body;
class Shape;

////////////////////////////////////////////////////////////////////////////////
// ANIMATION INSTANCE
////////////////////////////////////////////////////////////////////////////////
struct AnimationInstance {
	float animTimePos        = 0.f;
	float speedMultiplier    = 2.f;
	SkinnedData * animData   = nullptr;
	const char * curClipName = "NONE";
	std::map< std::string, AnimationClip >::iterator pCurAnim;

	Vec3 worldPos		     = Vec3( 0, 0, 15 );
	Body * bodiesToAnimate   = nullptr;

	AnimationInstance();
	~AnimationInstance();
	void Initialize( Body * bodies, unsigned numBodies, const Vec3 & startPos_WS, Shape * shapeToAnimate );
	void Update( float deltaT );
	const char * CycleCurClip() {
		if ( ++pCurAnim == animData->animations.end() ) {
			pCurAnim = animData->animations.begin();
		}
		curClipName = pCurAnim->first.c_str();
		return curClipName;
	}
};