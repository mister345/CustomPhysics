#include "Bone.h"
#include <inttypes.h>

// Utility functions
namespace {
	Quat Lerp( const Quat & from, const Quat & to, float t ) {
		Vec4 a( from.x, from.y, from.z, from.w );
		Vec4 b( to.x, to.y, to.z, to.w );

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
		Vec4 qA( from.x, from.y, from.z, from.w );
		Vec4 qB( to.x, to.y, to.z, to.w );

		const float cosTheta = qA.Dot( qB );
		if ( cosTheta > ( 1.f - 0.001 ) ) {
			return Lerp( from, to, t ); // small enough ang to just lerp
		}

		const float theta = acos( cosTheta );
		const float sinTheta = sin( theta );
		const float denomInverse = 1.f / sinTheta;
		const float weightA = sin( ( 1.f - t ) * theta ) * denomInverse;
		const float weightB = sin( t * theta ) * denomInverse;

		const Vec4 result = qA * weightA + qB * weightB;

		//	note - old way has more rounding errors bc not memoizing produces more operations, more opportunities for precision degradation
		//	const Vec4 resultOld = 
		//		qA * ( sin( theta * ( 1.f - t ) ) / sinTheta ) + 
		//		qB * ( sin( theta * t ) / sinTheta );

		const Quat resultQ = { result.x, result.y, result.z, result.w }; // quat ctor normalizes automatically
		return resultQ;
	}
} //anonymous namespace

////////////////////////////////////////////////////////////////////////////////
// BONE TRANSFORM
////////////////////////////////////////////////////////////////////////////////
BoneTransform BoneTransform::Identity() {
	return {
		{ 0, 0, 0, 1 },
		{ 0, 0, 0 },
		true
	};
}

void BoneTransform::operator*=( const BoneTransform & other ) {
	/*
	*	sometimes, you want a flat hierarchy of bones as in vertex animation;
	*	in that case, concatenate each transform with an identity root, ie, do nothing!
	*
	if( IsIdentity() ){
	return;
	}

	*/

	translation += rotation.RotatePoint( other.translation );
	rotation *= other.rotation;
}

BoneTransform BoneTransform::operator*( const BoneTransform & b ) const {
	/*
	*	sometimes, you want a flat hierarchy of bones as in vertex animation;
	*	in that case, concatenate each transform with an identity root, ie, do nothing!
	*
	if( IsIdentity() ){
	return *this;
	}

	*/

	BoneTransform concatenated;
	concatenated.translation = translation + rotation.RotatePoint( b.translation );
	concatenated.rotation = rotation * b.rotation;
	return concatenated;
}

////////////////////////////////////////////////////////////////////////////////
// BONE ANIMATION
////////////////////////////////////////////////////////////////////////////////
float BoneAnimation::GetStartTime() const {
	return keyframes.front().timePos;
}

float BoneAnimation::GetEndTime() const {
	return keyframes.back().timePos;
}

void BoneAnimation::Interpolate( float t, BoneTransform & outTransform, int myIdx ) const {
	if ( keyframes.empty() ) {
		puts( "Bone Animation had no keyframe! Returning..." );
		return;
	}

	// return first keyframe
	if ( t <= keyframes.front().timePos ) {
		outTransform.rotation = keyframes.front().transform.rotation;
		outTransform.translation = keyframes.front().transform.translation;
	} else if ( t >= keyframes.back().timePos ) {
		outTransform.rotation = keyframes.back().transform.rotation;
		outTransform.translation = keyframes.back().transform.translation;
	} else {
		// where does the given time t fall within our list of keyframes?
		for ( size_t i = 0; i < keyframes.size() - 1; i++ ) {
			const Keyframe & start = keyframes[ i ];
			const Keyframe & end = keyframes[ i + 1 ];
			if ( t >= start.timePos && t <= end.timePos ) {
				// we have found the two keyframes that t lies between, 
				// so interpolate it within the range of start ~ end

				// lerp translation
				const float range = end.timePos - start.timePos;
				const float progress = ( t - start.timePos ) / range;
				outTransform.translation = start.transform.translation + ( end.transform.translation - start.transform.translation ) * progress;

				// slerp quat
				outTransform.rotation = Slerp( start.transform.rotation, end.transform.rotation, progress );

				if ( myIdx != -1 ) {
					printf( "\tinput time:%1.3f\tprog btwn frames %i~%i\t:\t%2.2f\t\n", t, i, i + 1, progress );
				}
				break;
			}
		}
	}
}

