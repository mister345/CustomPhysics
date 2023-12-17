#include "AnimationData.h"
#include "AnimationState.h"

AnimationInstance::AnimationInstance() {
	animData = new SkinnedData();
}

AnimationInstance::~AnimationInstance() {
	delete animData;
	animData = nullptr;
	bodiesToAnimate = nullptr;
}

void AnimationInstance::Initialize( Body * bodies, unsigned numBodies, const Vec3 & startPos_WS, const char * clipToPlay ) {
	curClipName				   = clipToPlay;
	bodiesToAnimate			   = bodies;
	worldPos				   = startPos_WS;
	std::vector< BoneTransform > initialTransforms;

	if ( animData->mAnimations.empty() ) {
		// if no anims, just T-pose
		initialTransforms.assign( animData->RefPoseOffsets_ComponentSpace.begin(), animData->RefPoseOffsets_ComponentSpace.end() );
	} else {
		animData->GetFinalTransforms( curClipName, 0, initialTransforms );
	}
	for ( int i = 0; i < numBodies; i++ ) {
		Body * bodyToAnimate = bodiesToAnimate + i;
		bodyToAnimate->m_position = worldPos + initialTransforms[ i ].translation;
		bodyToAnimate->m_orientation = initialTransforms[ i ].rotation; // @TODO - add world rotation member
		bodyToAnimate->m_linearVelocity.Zero();
		bodyToAnimate->m_invMass = 0.f;	// no grav
		bodyToAnimate->m_elasticity = 1.f;
		bodyToAnimate->m_friction = 0.f;
		bodyToAnimate->m_shape = new ShapeSphere( 0.45f );
	}
}

void AnimationInstance::Update( float deltaT ) {
	if ( animData->mAnimations.empty() || bodiesToAnimate == nullptr ) {
		// no anim data, so dont update the pose ( just stay in t-pose )
		return;
	}

	// find our time bounds for looping
	const AnimationClip curClip = animData->mAnimations.at( curClipName );
	const float loopBoundary	= curClip.GetClipEndTime();

	// increment time and wrap around
	animTimePos += deltaT;
	if ( animTimePos * speedMultiplier >= loopBoundary ) {
		animTimePos = 0.f;
	}
	const float timePos = animTimePos * speedMultiplier;

	// ask AnimData to interpolate each bone transform across its keyframes, based on 
	// our given time point, and concatenate them down the skeletal hiearchy...
	// to get the final COMPONENT SPACE transforms of our bones
	std::vector< BoneTransform > boneTransforms( animData->BoneCount() );
	animData->GetFinalTransforms( curClipName, timePos, boneTransforms );

	// ( with a single bone and no hierarchy, we are basically doing this: )
	// singleBoneAnim.Interpolate( timePos, boneTransforms[ 0 ] );

	// apply each bone transform to each body ( 1 to 1 )
	for ( int i = 0; i < animData->BoneCount(); i++ ) {
		Body * bodyToAnimate			= bodiesToAnimate + i;
		const BoneTransform & transform = boneTransforms[ i ];
		bodyToAnimate->m_orientation = transform.rotation;
		bodyToAnimate->m_position = worldPos + transform.translation;
	}
}