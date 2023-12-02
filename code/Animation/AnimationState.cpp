#include "AnimationState.h"

namespace SingleBoneTest {
	constexpr float TO_RAD = 3.14159265359f / 180.f;
	constexpr const char * ANIM_NAME = "SingleBone";
	const std::vector< int > HIERARCHY = { -1 };
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

void PopulateTestAnimData( SkinnedData & skinnedData ) {
	AnimationClip clip{};
	clip.BoneAnimations.push_back( MakeBoneAnim() );
	std::map< std::string, AnimationClip > SingleBoneAnim = { std::make_pair( SingleBoneTest::ANIM_NAME, clip ) };

	skinnedData.Set( HIERARCHY, OFFSETS, SingleBoneAnim );
}
} // namespace SingleBoneTest

void AnimationInstance::Initialize( Body * bodySrc, const Vec3 & startPos_WS ) {
	SingleBoneTest::PopulateTestAnimData( animData );
	const BoneAnimation & singleBoneAnim = GetSingleBoneTestAnim();

	// rendering / physics
	bodyToAnimate = bodySrc;
	bodyToAnimate->m_position = startPos_WS;
	bodyToAnimate->m_orientation = singleBoneAnim.keyframes[ 0 ].transform.rotation;
	bodyToAnimate->m_linearVelocity.Zero();
	bodyToAnimate->m_invMass = 0.f;	// no grav
	bodyToAnimate->m_elasticity = 1.f;
	bodyToAnimate->m_friction = 0.f;
	bodyToAnimate->m_shape = new ShapeSphere( 2.f );
}

void AnimationInstance::Update( float deltaT ) {
	// @TODO - generalize!
	// currently hardcoded to only animate the single bone from our test
	const BoneAnimation & singleBoneAnim = GetSingleBoneTestAnim();

	animTimePos += deltaT;
	if ( animTimePos * ANIM_MULTIPLIER >= singleBoneAnim.GetEndTime() ) {
		animTimePos = 0.f;
	}
	BoneTransform bTransform{};
	singleBoneAnim.Interpolate( animTimePos * ANIM_MULTIPLIER, bTransform );

	bodyToAnimate->m_orientation = bTransform.rotation;
	bodyToAnimate->m_position = worldPos + bTransform.translation;
}

const BoneAnimation & AnimationInstance::GetSingleBoneTestAnim() const {
	return animData.mAnimations.at( SingleBoneTest::ANIM_NAME ).BoneAnimations[ 0 ];
}
