#include <vector>
#include "../Math/Vector.h"
#include "../Math/Quat.h"
#include "AnimationData.h"

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

void BoneAnimation::Interpolate( float t, BoneTransform & outTransform ) const {
	// return first keyframe
	if ( t <= keyframes.front().timePos ) {
		outTransform.rotation = keyframes.front().transform.rotation;
		outTransform.translation = keyframes.front().transform.translation;
	} else if ( t >= keyframes.back().timePos ) {
		outTransform.rotation = keyframes.back().transform.rotation;
		outTransform.translation = keyframes.back().transform.translation;
	} else {
		for ( size_t i = 0; i < keyframes.size() - 1; i++ ) {
			const Keyframe & start = keyframes[ i ];
			const Keyframe & end   = keyframes[ i + 1 ];
			if ( t >= start.timePos && t <= end.timePos ) {
				// we have found the corresponding range for the keyframe, so interpolate between begin and end

				// lerp translation
				const float range		 = end.timePos - start.timePos;
				const float progress	 = ( t - start.timePos ) / range;
				outTransform.translation = start.transform.translation + ( end.transform.translation - start.transform.translation ) * progress;

				// slerp quat
				outTransform.rotation = Slerp( start.transform.rotation, end.transform.rotation, progress );
				printf( "cur time: %10.2f\ncur prog: %2.2f\ncur keyframes: %zu:%zu", t, progress, i, i + 1 );
				break;
			}
		}
	}
}

void SkinnedData::Initialize() {
}

float AnimationClip::GetClipStartTime() const {
	float minTime = std::numeric_limits< float >::max();
	for ( const BoneAnimation & anim : BoneAnimations ) {
		minTime = std::min( anim.GetStartTime(), minTime );
	}
	return minTime;
}

float AnimationClip::GetClipEndTime() const {
	float maxTime = std::numeric_limits< float >::min();
	for ( const BoneAnimation & anim : BoneAnimations ) {
		maxTime = std::max( anim.GetEndTime(), maxTime );
	}
	return maxTime;
}

// interpolate all individual bone animations and populate a list of the exact transform of each bone
// ( aka, the pose ), at this given time
void AnimationClip::Interpolate( float t, std::vector<BoneTransform> & boneTransforms ) const {
	for ( int i = 0; i < BoneAnimations.size(); i++ ) {
		BoneAnimations[ i ].Interpolate( t, boneTransforms[ i ] );
	}
}

void SkinnedData::Set( 
	const std::vector<int> & boneHierarchy, 
	std::vector<BoneTransform> & boneOffsets, 
	std::map<std::string, AnimationClip> & animations ) {

	BoneHierarchy.assign( boneHierarchy.begin(), boneHierarchy.end() );
	boneOffsets.assign( boneOffsets.begin(), boneOffsets.end() );
	mAnimations.insert( animations.begin(), animations.end() );
}

void SkinnedData::GetFinalTransforms( 
	const std::string & clipName, 
	float timePos, 
	std::vector<BoneTransform> & outFinalTransforms ) const {

	AnimationClip clip = mAnimations.at( clipName );	
	// we will compose all the final transforms in place

	// get all the individual bone transforms( still relative to their parents, as defined in the fbx file ),
	// but interpolated in the TIME DOMAIN, across their keyframes at this exact time point
	std::vector< BoneTransform > interpolatedBoneSpaceTransforms( BoneCount() );
	clip.Interpolate( timePos, interpolatedBoneSpaceTransforms );

	// now concatenate all the transforms leading up to each leaf, to get the full transforms that take us
	// from component space ( skeletal root ), to each respective bone
	// this will be the final weight represented by each bone, that will deform each vertex in the skinning stage
	// ( maybe be blended with multiple bones as a matrix palette )
	std::vector< BoneTransform > concatenatedTransforms( BoneCount() );
	// prepopulate tree root node because it has no parent
	concatenatedTransforms[ 0 ] = interpolatedBoneSpaceTransforms[ 0 ];
	for ( int i = 1; i < BoneCount(); i++ ) {
		const int parentIdx = BoneHierarchy[ i ];
		const BoneTransform parentTransform = interpolatedBoneSpaceTransforms[ parentIdx ];
		concatenatedTransforms[ i ] *= parentTransform;
	}
}