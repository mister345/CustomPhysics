#pragma once

#include "../Math/Vector.h"
#include "../Math/Quat.h"
#include "../Physics/Body.h"
#include "AnimationAssets.h"

class SkinnedData;

////////////////////////////////////////////////////////////////////////////////
// ANIMATION INSTANCE
////////////////////////////////////////////////////////////////////////////////
enum eAnimMode : uint8_t {
	TPOSE = 0,
	LOCAL = 1,
	GLOBAL = 2,
	MODE_COUNT = 3
};

struct AnimationInstance {
	float animTimePos = 0.f;
	float speedMultiplier = 2.f;
	SkinnedData * animData = nullptr;
	std::map< std::string, AnimationClip >::iterator pCurAnim;

	Vec3 worldPos = Vec3( 0, 0, 15 );
	std::vector< Body > bodiesToAnimate;

	bool isInstanced = true;
	bool startInTPose = true;
//	eAnimMode animMode = startInTPose ? TPOSE : LOCAL;
	eAnimMode animMode = GLOBAL;

	AnimationInstance( const Vec3 & worldPos, AnimationAssets::eSkeleton whichSkeleton );
	~AnimationInstance();
	void Update( float deltaT );
	const char * GetCurClipName();
	const char * CycleCurClip();
	void CycleAnimMode() { 
		animMode = static_cast< eAnimMode >( ( animMode + 1 ) % MODE_COUNT );
	}
};