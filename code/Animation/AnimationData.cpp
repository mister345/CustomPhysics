#include <cassert>
#include <algorithm>
#include <vector>
#include "../Math/Vector.h"
#include "../Math/Quat.h"
#include "ModelLoader.h"
#include "AnimationData.h"
#include "AnimationState.h"

#include "../../libs/FBX/2020.3.4/include/fbxsdk/scene/animation/fbxanimstack.h"
#include "../../libs/FBX/2020.3.4/include/fbxsdk/scene/animation/fbxanimcurve.h"
#include "../../libs/FBX/2020.3.4/include/fbxsdk/scene/animation/fbxanimevaluator.h"

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
void SkinnedData::Set( 
	const std::vector<BoneInfo_t> & boneHierarchy,
	std::vector<BoneTransform> & boneOffsets, 
	std::map<std::string, AnimationClip> & animations ) {

	// make sure we have no negative keyframes
	for ( auto & iter = animations.begin(); iter != animations.end(); iter++ ) {
		assert( iter->second.HasValidKeyframes() );
	}

	BoneHierarchy.assign( boneHierarchy.begin(), boneHierarchy.end() );
	OffsetMatrices.assign( boneOffsets.begin(), boneOffsets.end() );
	animations.insert( animations.begin(), animations.end() );
}

void SkinnedData::OnFoundBoneCB( void * user, fbxsdk::FbxNode * boneNode ) {
	assert( boneNode->GetNodeAttribute()->GetAttributeType() == fbxsdk::FbxNodeAttribute::EType::eSkeleton );

	SkinnedData * me	  = reinterpret_cast< SkinnedData * >( user );
	const char * boneName = boneNode->GetName();

	/////////////////////////////////////////////////////////////////
	// Populate OffsetMatrices array Bone Map, and Bone Hierarchy
	/////////////////////////////////////////////////////////////////
	// @BUG - this is NOT getting the bind pose; it's getting the pose @ time 0 of top anim in stack!
	fbxsdk::FbxAMatrix localTransform = boneNode->EvaluateLocalTransform( FBXSDK_TIME_INFINITE ); // infinite gets default w/o any anims
	fbxsdk::FbxVector4 translation    = localTransform.GetT();
	fbxsdk::FbxQuaternion rotation    = localTransform.GetQ();
	me->OffsetMatrices.emplace_back( &rotation, &translation );

	const int boneIdx = me->OffsetMatrices.size() - 1;
	if ( !me->BoneIdxMap.insert( { boneName, boneIdx } ).second ) {
		return;
	}

	const char * parentName = boneNode->GetParent()->GetName();
	const bool bFoundParent = me->BoneIdxMap.find( parentName ) != me->BoneIdxMap.end();
	me->BoneHierarchy.push_back( bFoundParent ? me->BoneIdxMap[ parentName ] : -1 );

	//////////////////////////////////////
	// Get all animations for this bone
	//////////////////////////////////////
	for ( int i = 0; i < me->fbxScene->GetSrcObjectCount< FbxAnimStack >(); i++ ) {
		fbxsdk::FbxAnimStack * stack = me->fbxScene->GetSrcObject< FbxAnimStack >( i );
		me->fbxScene->SetCurrentAnimationStack( stack );
		AnimationClip & clip = me->animations[ stack->GetName() ];
		clip.BoneAnimations.emplace_back( me->fbxScene, boneNode );
	}
}

void SkinnedData::BoneSpaceToModelSpace( int boneIdx, std::vector< BoneTransform > & inOutBoneTransforms ) const {
	const int parentIdx = BoneHierarchy[ boneIdx ].GetParent();
	if ( parentIdx < 0 ) {
		return;
	}

	BoneTransform & outBoneTransform = inOutBoneTransforms[ boneIdx ];
	outBoneTransform = inOutBoneTransforms[ parentIdx ] * outBoneTransform;
}

/*	https://www.gamedev.net/articles/programming/graphics/how-to-work-with-fbx-sdk-r3582/
	https://stackoverflow.com/questions/45690006/fbx-sdk-skeletal-animations
NOTE - 2 ways to get frames from fbx file
	1. Store key frames only ( raw animation data from within the fbx file )
	2. Sample frames from the fbx file at a fixed rate - loses raw animation data, 
		bc its resampling the original keyframes according to some arbitrary logic. 
	-> we use way #2 because much more straightforward ( not easy to match curves to bones )		
*/
void SkinnedData::Set( fbxsdk::FbxScene * scene, const AnimationAssets::eWhichAnim whichAnim ) {
	// set active layer first so that the per-node callbacks can access it later
	if ( scene == nullptr ) {
		assert( !"ERROR - trying to load data from an empty FbxScene!" );
		return;
	}

	fbxScene  = scene;
	animStack = fbxScene->GetCurrentAnimationStack();
	activeLayer				   = animStack->GetMember<fbxsdk::FbxAnimLayer>();
	const std::string animName = animStack->GetName();
	AnimationAssets::animNames[ AnimationAssets::eWhichAnim::SKELETON_ONLY ] = animName;
	AnimationAssets::animNames[ AnimationAssets::eWhichAnim::SKINNED_MESH ]  = animName;
	assert( activeLayer != nullptr && !animName.empty() );

	const int boneCount = FbxUtil::CountBonesInSkeleton( fbxScene->GetRootNode() );
	curAnimName = animName.c_str();

	for ( int i = 0; i < scene->GetSrcObjectCount< FbxAnimStack >(); i++ ) {
		fbxsdk::FbxAnimStack * curStack = scene->GetSrcObject< FbxAnimStack >( i );
		animations.insert( { curStack->GetName(), AnimationClip() } );
	}
	FbxUtil::callbackAPI_t cb{ &OnFoundBoneCB, nullptr };
	FbxUtil::HarvestSceneData( fbxScene, false, cb, this );

	// accumulate local bone transforms to bring into component space
	for ( int i = 1; i < boneCount; i++ ) {
		BoneSpaceToModelSpace( i, OffsetMatrices );
	}

	OffsetMatrices_DIRECT_DEBUG.assign( OffsetMatrices.begin(), OffsetMatrices.end() );
	std::transform( OffsetMatrices.begin(), OffsetMatrices.end(), OffsetMatrices.begin(),
					[]( BoneTransform & bTransform ) { return bTransform.Inverse(); } );
}

void SkinnedData::GetFinalTransforms( const std::string & cName, float time, std::vector<BoneTransform> & outFinalTransforms ) const {
	const int boneCount = BoneCount();
	AnimationClip clip  = animations.at( cName );

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