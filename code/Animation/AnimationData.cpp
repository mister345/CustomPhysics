#include <vector>
#include "ModelLoader.h"
#include "../Math/Vector.h"
#include "../Math/Quat.h"
#include "AnimationData.h"
#include "AnimationState.h"

namespace AnimationAssets {
	constexpr float TO_RAD = 3.14159265359f / 180.f;
	const char * animNames[ eWhichAnim::COUNT + 1 ] = {
		"SingleBone",
		"MultiBone",
		"SkeletonOnly",
		"SkinnedMesh",
		"Unknown",
	};

	void FillAnimInstanceData( SkinnedData *& skinnedData, eWhichAnim whichAnim, fbxsdk::FbxScene * sceneData ) {
		switch ( whichAnim ) {
			case SINGLE_BONE: {
				int boneCount;
				std::vector< int > hierarchy;
				std::vector< BoneTransform > boneOffsets;
				AnimationClip clip;

				hierarchy.assign( { -1 } );
				boneOffsets.assign( { BoneTransform::Identity() } );
				clip.BoneAnimations.assign( { MakeBoneAnim1(), } );

				std::map< std::string, AnimationClip > animMap = { { animNames[ whichAnim ], clip } };
				skinnedData->Set( hierarchy, boneOffsets, animMap );
				break;
			}
			case MULTI_BONE: {
				int boneCount;
				std::vector< int > hierarchy;
				std::vector< BoneTransform > boneOffsets;
				AnimationClip clip;

				hierarchy.assign( { -1, 0, 1, 2, 3, 4, 5, 6, 7 } );
				boneOffsets.assign( { { { 0, 0, 0, 1 }, { 0, 0 * 2.f, 0 }, true, },
									  { { 0, 0, 0, 1 }, { 0, 2 * 2.f, 0 }, false, },
									  { { 0, 0, 0, 1 }, { 0, 4 * 2.f, 0 }, false, },
									  { { 0, 0, 0, 1 }, { 0, 6 * 2.f, 0 }, false, },
									  { { 0, 0, 0, 1 }, { 0, 8 * 2.f, 0 }, false, },
									  { { 0, 0, 0, 1 }, { 0, 10 * 2.f, 0 }, false, },
									  { { 0, 0, 0, 1 }, { 0, 12 * 2.f, 0 }, false, },
									  { { 0, 0, 0, 1 }, { 0, 14 * 2.f, 0 }, false, },
									  { { 0, 0, 0, 1 }, { 0, 16 * 2.f, 0 }, false, } } );

				BoneAnimation boneAnim = MakeBoneAnim0();
				clip.BoneAnimations.assign( { { { } },
											  boneAnim,
											  boneAnim,
											  boneAnim,
											  boneAnim,
											  boneAnim,
											  boneAnim,
											  boneAnim,
											  boneAnim } );

				std::map< std::string, AnimationClip > animMap = { { animNames[ whichAnim ], clip } };
				skinnedData->Set( hierarchy, boneOffsets, animMap );
				break;
			}
			case SKELETON_ONLY:
			case SKINNED_MESH:
			default: {
				skinnedData->Set( sceneData, whichAnim );
				break;
			}
		}
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
} // namespace AnimationAssets

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
		// where does teh given time t fall within our list of keyframes?
		for ( size_t i = 0; i < keyframes.size() - 1; i++ ) {
			const Keyframe & start = keyframes[ i ];
			const Keyframe & end   = keyframes[ i + 1 ];
			if ( t >= start.timePos && t <= end.timePos ) {
				// we have found the two keyframes that t lies between, 
				// so interpolate it within the range of start ~ end

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

float AnimationClip::GetClipStartTime() const {
	float minTime = std::numeric_limits< float >::max();
	for ( const BoneAnimation & anim : BoneAnimations ) {
		if ( anim.HasData() ) {
			minTime = std::min( anim.GetStartTime(), minTime );
		}
	}
	return minTime;
}

float AnimationClip::GetClipEndTime() const {
	float maxTime = std::numeric_limits< float >::min();
	for ( const BoneAnimation & anim : BoneAnimations ) {
		if ( anim.HasData() ) {
			maxTime = std::max( anim.GetEndTime(), maxTime );
		}
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

	// make sure we have no negative keyframes
	for ( auto & iter = animations.begin(); iter != animations.end(); iter++ ) {
		assert( iter->second.HasValidKeyframes() );
	}

	BoneHierarchy.assign( boneHierarchy.begin(), boneHierarchy.end() );
	RefPoseOffsets_ComponentSpace.assign( boneOffsets.begin(), boneOffsets.end() );
	mAnimations.insert( animations.begin(), animations.end() );
}

void SkinnedData::Set( fbxsdk::FbxScene * scene, const AnimationAssets::eWhichAnim whichAnim ) {
	// @TODO - move into to animation assets namespace	
	struct local_t {
		static BoneTransform ToBoneTransform( fbxsdk::FbxVector4 & translation_LS, fbxsdk::FbxVector4 & rotation_LS ) {
			//BoneTransform temp;
			//temp.isIdentity = false;
			//temp.rotation = Quat()
			return {};
		}
		static void ToBindPose( 
			fbxsdk::FbxAMatrix & localTransform,
			fbxsdk::FbxVector4 & translation_LS,
			fbxsdk::FbxVector4 & rotation_LS ){

			// https://www.gamedev.net/forums/topic/515878-fbx-sdk-how-to-get-bind-pose/4354881/
			/* TODO
				The bind poses are stored as inverse world space transform matrices.
				Invert them, and you'll have world space TM's.
				Multiply by the parents inverse TM's and you'll get the local space matrices.
				Extract the translation direct from the matrix.
				Extract the scale by getting the length of each axis. You can skip this if you aren't animating scale.
				Convert the matrix to a quaternion, and you'll get the combined PreRotate * Rotate * PostRotate.
				Pre and Post multiply the inverse Pre/Post rotate quats you get from the FBX sdk and you'll be able to extract the actual rotation values.
				( mayube not necessary if u use the alt function, not go thru matrices )
			*/
		}
	};

	FbxUtil::HarvestSceneData( 
		scene, 
		this,  
		[]( void * userData,
			fbxsdk::FbxAMatrix & localTransform,
			fbxsdk::FbxVector4 & translation_LS,
			fbxsdk::FbxVector4 & rotation_LS ) {

		SkinnedData * me = reinterpret_cast< SkinnedData * >( userData );

		local_t::ToBindPose( localTransform, translation_LS, rotation_LS );

		// @TODO - push_back on the BoneOffsets_ComponentSpace, etc, etc
		/*	( smtg like this )
			for( ToBoneTransform( 
									
		*/

		printf( "TODO - now that we found our bone, populate the data!" );
	});
}

void SkinnedData::GetFinalTransforms( 
	const std::string & clipName, 
	float timePos, 
	std::vector<BoneTransform> & outFinalTransforms ) const {

	// pre-multiply the ref pose offsets 
	// ( T-Pose positions of bones, relative to which these bone animations are defined )
	// @TODO - do these also need to be multiplied in accumulated fashion like hte other transforms,
	outFinalTransforms.assign( RefPoseOffsets_ComponentSpace.begin(), RefPoseOffsets_ComponentSpace.end() );

	// get all the individual bone transforms( still relative to their parents, as defined in the fbx file ),
	// but interpolated in the TIME DOMAIN, across their keyframes at this exact time point
	// @TODO - cache and reuse
	AnimationClip clip = mAnimations.at( clipName );	
	std::vector< BoneTransform > interpolatedBoneSpaceTransforms( BoneCount() );
	clip.Interpolate( timePos, interpolatedBoneSpaceTransforms );

	/*
		now concatenate all the transforms leading up to each leaf, to get the full transforms that take us
		from component space ( skeletal root ), to each respective bone
		this will be the final weight represented by each bone, that will deform each vertex in the skinning stage
		( maybe be blended with multiple bones as a matrix palette )
	*/
	
	// prepopulate tree root node because it has no parent
	// NOTE - root will probably have NO animation so this value should always be identity
	
	// concatenate the rest, from root to leaf
	// NOTE - if we want a flat hiearchy, it is the CONTENT CREATOR's job to add an identity root bone
	// for consistency, we will still "concatenate" all the bones w that identity, and the hierarchy will look like this:
	// const std::vector< int > HIERARCHY = { -1, 0, 0, 0, 0, 0, ... }; // ( could be used as a partical shader )

	// When you accumulate local bone transforms from teh ROOT to the leaf,
	// you are bringing that bone from local space into component space

	for ( int i = 1; i < BoneCount(); i++ ) {
		const int parentIdx = BoneHierarchy[ i ];
		const BoneTransform parentSpaceTransform = interpolatedBoneSpaceTransforms[ parentIdx ];

		// first, get into bone space ( parent space ) by accumulated transform from parent
		// now apply the current interpolated transform of THIS bone, in that space
		interpolatedBoneSpaceTransforms[ i ] = parentSpaceTransform * interpolatedBoneSpaceTransforms[ i ];
	}

	// now that the bones are in component space
	for ( int i = 0; i < BoneCount(); i++ ) {
		outFinalTransforms[ i ] *= interpolatedBoneSpaceTransforms[ i ];
	}
}