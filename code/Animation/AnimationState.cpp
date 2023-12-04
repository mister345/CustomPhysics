#include "AnimationState.h"

namespace AnimationAssets {

constexpr float TO_RAD = 3.14159265359f / 180.f;
const char * animNames[ eWhichAnim::COUNT + 1 ] = {
	"SingleBone", 
	"MultiBone",
	"RealMesh",
	"SkinnedMesh",
	"Unknown",
};

void MakeAnimInstanceData( SkinnedData & skinnedData, const eWhichAnim whichAnim ) {
	int boneCount;
	std::vector< int > hierarchy;
	std::vector< BoneTransform > boneOffsets;
	AnimationClip clip;

	switch ( whichAnim ) {
		case SINGLE_BONE: {
			hierarchy.assign( { -1 } );
			boneOffsets.assign( { BoneTransform::Identity() } );
			clip.BoneAnimations.assign( { MakeBoneAnim1(), } );
			break;
		}
		case MULTI_BONE: {
			hierarchy.assign( { -1, 0, 1, 2, 3, 4, 5, 6, 7 } );
			boneOffsets.assign( { { { 0, 0, 0, 1 }, { 0, 0  * 2.f, 0 }, true, },
								  { { 0, 0, 0, 1 }, { 0, 2  * 2.f, 0 }, false, },
								  { { 0, 0, 0, 1 }, { 0, 4  * 2.f, 0 }, false, },
								  { { 0, 0, 0, 1 }, { 0, 6  * 2.f, 0 }, false, },
								  { { 0, 0, 0, 1 }, { 0, 8  * 2.f, 0 }, false, },
								  { { 0, 0, 0, 1 }, { 0, 10 * 2.f, 0 }, false, },
								  { { 0, 0, 0, 1 }, { 0, 12 * 2.f, 0 }, false, },
								  { { 0, 0, 0, 1 }, { 0, 14 * 2.f, 0 }, false, },
								  { { 0, 0, 0, 1 }, { 0, 16 * 2.f, 0 }, false, } } );

			BoneAnimation boneAnim = MakeBoneAnim0();
			clip.BoneAnimations.assign( {  { { } },
										  boneAnim,
										  boneAnim,
										  boneAnim,
										  boneAnim,
										  boneAnim,
										  boneAnim,
										  boneAnim,
										  boneAnim } );
			break;
		}
		case REAL_MESH:
		case SKINNED_MESH:
		default: {
			hierarchy.assign( {} );
			boneOffsets.assign( {} );
			clip.BoneAnimations.assign( {} );
			break;
		}
	}
	std::map< std::string, AnimationClip > animMap = { { animNames[ whichAnim ], clip } };
	skinnedData.Set( hierarchy, boneOffsets, animMap );
}

BoneAnimation MakeBoneAnim0() {
	BoneAnimation anim;
	anim.keyframes.resize( 5 );

	anim.keyframes[ 0 ].timePos = 0.0f;
	anim.keyframes[ 0 ].transform.translation = Vec3( -5.0f, 0.0f, 0.0f );
	anim.keyframes[ 0 ].transform.rotation = { 0, 0, 0, 1 };

	anim.keyframes[ 1 ].timePos = 1.0f;
	anim.keyframes[ 1 ].transform.translation = Vec3( 0.0f, 0.0f, 0.0f );
	anim.keyframes[ 1 ].transform.rotation = { 0, 0, 0, 1 };

	anim.keyframes[ 2 ].timePos = 1.0f;
	anim.keyframes[ 2 ].transform.translation = Vec3( 5.0f, 0.0f, 0.0f );
	anim.keyframes[ 2 ].transform.rotation = { 0, 0, 0, 1 };

	anim.keyframes[ 3 ].timePos = 3.0f;
	anim.keyframes[ 3 ].transform.translation = Vec3( 0.0f, 0.0f, 0.0f );
	anim.keyframes[ 3 ].transform.rotation = { 0, 0, 0, 1 };

	anim.keyframes[ 4 ].timePos = 4.0f;
	anim.keyframes[ 4 ].transform.translation = Vec3( -5.f, 0.0f, 0.0f );
	anim.keyframes[ 4 ].transform.rotation = { 0, 0, 0, 1 };

	return anim;
}

BoneAnimation MakeBoneAnim1() {
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
} // namespace SingleBoneTest


void AnimationInstance::Initialize( Body * bodies, unsigned numBodies, const Vec3 & startPos_WS, const char * clipToPlay ) {
	curClipName				   = clipToPlay;
	bodiesToAnimate			   = bodies;
	worldPos				   = startPos_WS;
	const AnimationClip & clip = animData.mAnimations.at( curClipName );

	std::vector< BoneTransform > initialTransforms;
	animData.GetFinalTransforms( curClipName, 0, initialTransforms );

	for ( int i = 0; i < numBodies; i++ ) {
		Body * bodyToAnimate = bodiesToAnimate + i;
		bodyToAnimate->m_position = worldPos + initialTransforms[ i ].translation;
		bodyToAnimate->m_orientation = initialTransforms[ i ].rotation; // @TODO - add world rotation member
		bodyToAnimate->m_linearVelocity.Zero();
		bodyToAnimate->m_invMass = 0.f;	// no grav
		bodyToAnimate->m_elasticity = 1.f;
		bodyToAnimate->m_friction = 0.f;
		bodyToAnimate->m_shape = new ShapeSphere( 1.f );
	}
}

void AnimationInstance::Update( float deltaT ) {
	// find our time bounds for looping
	const AnimationClip curClip = animData.mAnimations.at( curClipName );
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
	std::vector< BoneTransform > boneTransforms( animData.BoneCount() );
	animData.GetFinalTransforms( curClipName, timePos, boneTransforms );

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