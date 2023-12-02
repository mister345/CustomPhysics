#include "AnimationState.h"

// TODO
// const std::vector< int > HIERARCHY = { -1, 0, 1, 2 };
//	std::vector< BoneTransform > OFFSETS = { BoneTransform::Identity(), BoneTransform::Identity(), BoneTransform::Identity(), BoneTransform::Identity() };

namespace SingleBoneTest {
	const std::vector< int > HIERARCHY   = { -1 };
	std::vector< BoneTransform > OFFSETS = { BoneTransform::Identity() };

BoneAnimation MakeBoneAnim() {
	BoneAnimation anim;
	anim.keyframes.resize( 5 );

	anim.keyframes[ 0 ].timePos = 0.0f;
	anim.keyframes[ 0 ].transform.translation = Vec3( -1.0f, 0.0f, 0.0f );
	anim.keyframes[ 0 ].transform.rotation = Quat( Vec3( 0, 1, 0 ), 30 * TO_RAD );

	anim.keyframes[ 1 ].timePos = 1.0f;
	anim.keyframes[ 1 ].transform.translation = Vec3( 0.0f, 2.0f, 3.0f );
	anim.keyframes[ 1 ].transform.rotation = Quat( Vec3( 1, 1, 2 ), 45 * TO_RAD );

	anim.keyframes[ 2 ].timePos = 1.0f;
	anim.keyframes[ 2 ].transform.translation = Vec3( 3.0f, 0.0f, 0.0f );
	anim.keyframes[ 2 ].transform.rotation = Quat( Vec3( 0, 1, 0 ), -30 * TO_RAD );

	anim.keyframes[ 3 ].timePos = 3.0f;
	anim.keyframes[ 3 ].transform.translation = Vec3( 0.0f, 1.0f, -3.0f );
	anim.keyframes[ 3 ].transform.rotation = Quat( Vec3( 0, 1, 0 ), 70 * TO_RAD );

	anim.keyframes[ 4 ].timePos = 4.0f;
	anim.keyframes[ 4 ].transform.translation = Vec3( -2.5f, 0.0f, 0.0f );
	anim.keyframes[ 4 ].transform.rotation = Quat( Vec3( 0, 1, 0 ), 30 * TO_RAD );

	return anim;
}

void PopulateTestAnimData( SkinnedData & skinnedData, const int desiredBoneCount ) {
	AnimationClip clip{};
	for ( int i = 0; i < desiredBoneCount; i++ ) {
		clip.BoneAnimations.push_back( MakeBoneAnim() );
	}

	std::map< std::string, AnimationClip > SingleBoneAnim = { { SingleBoneTest::ANIM_NAME, clip } };
	skinnedData.Set( HIERARCHY, OFFSETS, SingleBoneAnim );
}
} // namespace SingleBoneTest


void AnimationInstance::Initialize( Body * bodies, unsigned numBodies, const Vec3 & startPos_WS, const char * clipToPlay ) {
	curClip = clipToPlay;

	const BoneAnimation * testAnim = TryGetSingleBoneTestAnim();
	if ( testAnim == nullptr ) {
		assert( "TEST ANIM WAS NULL!" );
	}
	bodiesToAnimate = bodies;
	const BoneAnimation & singleBoneAnim = *testAnim;

	// for now, this will default to only animating our single bone bc thats all we have!
	for ( int i = 0; i < numBodies; i++ ) {
		Body * bodyToAnimate = bodiesToAnimate + i;
		bodyToAnimate->m_position = startPos_WS;
		bodyToAnimate->m_orientation = singleBoneAnim.keyframes[ 0 ].transform.rotation;
		bodyToAnimate->m_linearVelocity.Zero();
		bodyToAnimate->m_invMass = 0.f;	// no grav
		bodyToAnimate->m_elasticity = 1.f;
		bodyToAnimate->m_friction = 0.f;
		bodyToAnimate->m_shape = new ShapeSphere( 1.f );
	}
}

void AnimationInstance::Update( float deltaT ) {
	// find our time bounds for looping
	const BoneAnimation * testAnim = TryGetSingleBoneTestAnim();
	if ( testAnim == nullptr ) {
		assert( "TEST ANIM WAS NULL!" );
	}
	const BoneAnimation & singleBoneAnim = *testAnim;
	const float loopBoundary = singleBoneAnim.GetEndTime();

	// increment time and wrap around
	animTimePos += deltaT;
	if ( animTimePos * speedMultiplier >= loopBoundary ) {
		animTimePos = 0.f;
	}
	const float timePos = animTimePos * speedMultiplier;

	// ask AnimData to interpolate each bone transform across its keyframes, based on 
	// our given time point, and concatenate them down the skeletal hiearchy...
	// to get the final COMPONENT SPACE transforms of our bones
	std::vector< BoneTransform > boneTransforms( animData.BoneCount() );
	animData.GetFinalTransforms( curClip, timePos, boneTransforms );

	// ( with a single bone and no hierarchy, we are basically doing this: )
	// singleBoneAnim.Interpolate( timePos, boneTransforms[ 0 ] );

	// apply each bone transform to each body ( 1 to 1 )
	for ( int i = 0; i < animData.BoneCount(); i++ ) {
		Body * bodyToAnimate			= bodiesToAnimate + i;
		const BoneTransform & transform = boneTransforms[ i ];
		bodyToAnimate->m_orientation = transform.rotation;
		bodyToAnimate->m_position = worldPos + transform.translation;
	}
}

const BoneAnimation * AnimationInstance::TryGetSingleBoneTestAnim() const {
	return &animData.mAnimations.at( SingleBoneTest::ANIM_NAME ).BoneAnimations[ 0 ];
}
