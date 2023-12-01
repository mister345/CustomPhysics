#include <vector>
#include "../Math/Vector.h"
#include "../Math/Quat.h"
#include "Animation.h"

// config
constexpr float ANIM_MULTIPLIER = 10.f;

float BoneAnimation::GetStartTime() const {
	return keyframes.front().timePos;
}

float BoneAnimation::GetEndTime() const {
	return keyframes.back().timePos;
}

Quat Lerp( const Quat & from, const Quat & to, float t ) {
	Vec4 a( from.x, from.y, from.z, from.w );
	Vec4 b(   to.x,   to.y,   to.z,   to.w );

	const bool isLongWayAround = a.Dot( b ) < 0;
	if ( isLongWayAround ) {
		a *= -1;
	}
	Vec4 lerped = a * ( 1 - t ) + b * t;

	Quat out( lerped.x, lerped.y, lerped.z, lerped.w );
	out.Normalize();
	return out;
}

Quat Slerp( const Quat & from, const Quat & to, float t ) {
	Vec4 a( from.x, from.y, from.z, from.w );
	Vec4 b( to.x, to.y, to.z, to.w );

	float cosPhi = a.Dot( b );
	if ( cosPhi > ( 1.f - 0.001 ) ) {
		// small angle so just lerp
		return Lerp( from, to, t );
	}

	float phi    = acos( cosPhi );
	float sinPhi = sin( phi );

	Vec4 result = a * ( sin( phi * ( 1.f - t ) ) / sinPhi ) + b * ( sin( phi * t ) / sinPhi );
	Quat resultQ( result.x, result.y, result.z, result.w );
//	resultQ.Normalize();
	return resultQ;
}

void BoneAnimation::Interpolate( float t, Quat & outRotation, Vec3 & outTranslation ) const {
	// return first keyframe
	if ( t <= keyframes.front().timePos ) {
		outRotation    = keyframes.front().transform.rotation;
		outTranslation = keyframes.front().transform.translation;
	} else if ( t >= keyframes.back().timePos ) {
		outRotation = keyframes.back().transform.rotation;
		outTranslation = keyframes.back().transform.translation;
	} else {
		for ( size_t i = 0; i < keyframes.size() - 1; i++ ) {
			const Keyframe & start = keyframes[ i ];
			const Keyframe & end   = keyframes[ i + 1 ];
			if ( t >= start.timePos && t <= end.timePos ) {
				// we have found the corresponding range for the keyframe, so interpolate between begin and end

				// lerp translation
				const float range    = end.timePos - start.timePos;
				const float progress = ( t - start.timePos ) / range;
				outTranslation		 = start.transform.translation + ( end.transform.translation - start.transform.translation ) * progress;

				// slerp quat
//				outRotation = Lerp( start.rotation, end.rotation, progress );
				outRotation = Slerp( start.transform.rotation, end.transform.rotation, progress );

				printf( "cur time: %10.2f\ncur prog: %2.2f\ncur keyframes: %zu:%zu", t, progress, i, i + 1 );

				break;
			}
		}
	}
}

Vec3 SkeletalMesh::meshWorldPos = Vec3( 0, 0, 15 );

void SkeletalMesh::Initialize( const Vec3 startPos_WS, Body * bodyToAnimateSource ) {
	bodyToAnimate = bodyToAnimateSource;

	// bone animation
	constexpr float TO_RAD = 3.14159265359f / 180.f;

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

	// rendering / physics
	bodyToAnimate->m_position = startPos_WS;
	bodyToAnimate->m_orientation = anim.keyframes[ 0 ].transform.rotation;
	bodyToAnimate->m_linearVelocity.Zero();
	bodyToAnimate->m_invMass = 0.f;	// no grav
	bodyToAnimate->m_elasticity = 1.f;
	bodyToAnimate->m_friction = 0.f;
	bodyToAnimate->m_shape = new ShapeSphere( 2.f );
}

void SkeletalMesh::Update( float deltaT ) {
	animTimePos += deltaT;
	if ( animTimePos * ANIM_MULTIPLIER >= anim.GetEndTime() ) {
		animTimePos = 0.f;
	}
	Vec3 posOffset;
	Quat rotOffset;
	anim.Interpolate( animTimePos * ANIM_MULTIPLIER, rotOffset, posOffset );

	bodyToAnimate->m_orientation = rotOffset;
	bodyToAnimate->m_position = meshWorldPos + posOffset;
}
