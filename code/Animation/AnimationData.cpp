#include <cassert>
#include <algorithm>
#include <vector>
#include "../Math/Vector.h"
#include "../Math/Quat.h"
#include "fbxInclude.h"
#include "ModelLoader.h"
#include "AnimationData.h"
#include "AnimationState.h"
#include "FbxNodeParsers.h"

////////////////////////////////////////////////////////////////////////////////
// ANIMATION CLIP
////////////////////////////////////////////////////////////////////////////////
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

// interpolate bone animations & populate a list of all bone transforms ( aka, the pose ) at this given time
void AnimationClip::Interpolate( float t, std::vector<BoneTransform> & boneTransforms ) const {
//	bool shouldPrint = true;
	bool shouldPrint = false;
	for ( int i = 0; i < BoneAnimations.size(); i++ ) {
		BoneAnimations[ i ].Interpolate( t, boneTransforms[ i ], shouldPrint ? i : -1 );
		shouldPrint = false;
	}
}

////////////////////////////////////////////////////////////////////////////////
// SKINNED DATA
////////////////////////////////////////////////////////////////////////////////
void SkinnedData::BoneSpaceToModelSpace( int boneIdx, std::vector< BoneTransform > & inOutBoneTransforms ) const {
	const int parentIdx = BoneHierarchy[ boneIdx ].GetParent();
	if ( parentIdx < 0 ) {
		return;
	}
	BoneTransform & outBoneTransform = inOutBoneTransforms[ boneIdx ];
	outBoneTransform = inOutBoneTransforms[ parentIdx ] * outBoneTransform;
}

void SkinnedData::Set( 
	const std::vector<BoneInfo_t> & boneHierarchy,
	std::vector<BoneTransform> & boneOffsets, 
	std::map<std::string, AnimationClip> & _animations ) {

	// make sure we have no negative keyframes
	//for ( auto & iter = _animations.begin(); iter != _animations.end(); iter++ ) {
	//	assert( iter->second.HasValidKeyframes() );
	//}

	BoneHierarchy.assign( boneHierarchy.begin(), boneHierarchy.end() );
	InvBindPoseMatrices.assign( boneOffsets.begin(), boneOffsets.end() );
	animations.insert( _animations.begin(), _animations.end() );
}

void SkinnedData::Set( fbxsdk::FbxScene * scene ) {
	// set active layer first so that the per-node callbacks can access it later
	if ( scene == nullptr ) {
		assert( !"ERROR - trying to load data from an empty FbxScene!" );
		return;
	}
	fbxScene = scene;

	for ( int i = 0; i < scene->GetSrcObjectCount< FbxAnimStack >(); i++ ) {
		fbxsdk::FbxAnimStack * curStack = scene->GetSrcObject< FbxAnimStack >( i );
		animations.insert( { curStack->GetName(), AnimationClip() } );
	}
	FbxUtil::callbackAPI_t cb{ 
		&FbxNodeParsers::OnFoundBoneCB,
		&FbxNodeParsers::OnFoundMeshCB
	};
	FbxUtil::HarvestSceneData( fbxScene, cb, this );

	// accumulate local bone transforms to bring into component space
	const int boneCount = BoneCount();
	for ( int i = 1; i < boneCount; i++ ) {
		BoneSpaceToModelSpace( i, InvBindPoseMatrices );
	}

	BindPoseMatrices.assign( InvBindPoseMatrices.begin(), InvBindPoseMatrices.end() );
	std::transform( InvBindPoseMatrices.begin(), InvBindPoseMatrices.end(), InvBindPoseMatrices.begin(),
					[]( BoneTransform & bTransform ) { return bTransform.Inverse(); } );
}

void SkinnedData::GetFinalTransforms( const std::string & cName, float time, std::vector<BoneTransform> & outFinalTransforms ) const {
	const int boneCount		   = BoneCount();
	const AnimationClip & clip = animations.at( cName );

	// get interpolated transform for every bone at THIS TIME
	std::vector< BoneTransform > interpolatedBoneSpaceTransforms( boneCount );
	clip.Interpolate( time, interpolatedBoneSpaceTransforms );

	// bring all the interpolated bones into model space ( accumulate from root to leaf )
	for ( int i = 1; i < boneCount; i++ ) {
		BoneSpaceToModelSpace( i, interpolatedBoneSpaceTransforms );
	}

	// @TODO - multiply these by inverse bind pose matrix ONLY if vertex shader bound
	// for debug purposes, we just use the loaded anim poses directly as they are already in model space
	outFinalTransforms = std::vector< BoneTransform >( boneCount, BoneTransform::Identity() );
	for ( int i = 0; i < boneCount; i++ ) {
		outFinalTransforms[ i ] *= interpolatedBoneSpaceTransforms[ i ];
	}
}