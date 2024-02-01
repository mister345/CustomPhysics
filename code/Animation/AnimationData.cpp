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
#include <thread>
#include <cmath>
#include <chrono>
#include "../Config.h"

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
void AnimationClip::Interpolate( float t, std::vector<BoneTransform> & boneTransforms, bool isGlobal ) const {
//	bool shouldPrint = true;
	bool shouldPrint = false;
	for ( int i = 0; i < BoneAnimations.size(); i++ ) {
		if ( isGlobal ) {
			BoneAnimationsGlobal[ i ].Interpolate( t, boneTransforms[ i ], shouldPrint ? i : -1 );
		} else {
			BoneAnimations[ i ].Interpolate( t, boneTransforms[ i ], shouldPrint ? i : -1 );
		}
		shouldPrint = false;
	}
}

// interpolate bone animations & populate a list of all bone transforms ( aka, the pose ) at this given time
void AnimationClip::Interpolate( float t, std::vector<fbxsdk::FbxAMatrix > & boneTransforms, bool isGlobal ) const {
//	bool shouldPrint = true;
	bool shouldPrint = false;
	for ( int i = 0; i < BoneAnimations.size(); i++ ) {
		if ( isGlobal ) {
			BoneAnimationsGlobal[ i ].Interpolate( t, boneTransforms[ i ], shouldPrint ? i : -1 );
		} else {
			BoneAnimations[ i ].Interpolate( t, boneTransforms[ i ], shouldPrint ? i : -1 );
		}
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

//////////////////////////////////////////////////////////////////////////////////////////////
// https://www.gamedev.net/articles/programming/graphics/how-to-work-with-fbx-sdk-r3582/
//////////////////////////////////////////////////////////////////////////////////////////////
void populateTPoseCB( void * user, fbxsdk::FbxNode * meshNode ) {
	SkinnedData * me = reinterpret_cast< SkinnedData * >( user );
	fbxsdk::FbxMesh * mesh = reinterpret_cast< FbxMesh * >( meshNode->GetNodeAttribute() );
	assert( meshNode->GetNodeAttribute()->GetAttributeType() == fbxsdk::FbxNodeAttribute::EType::eMesh );

	fbxsdk::FbxSkin * deformer = reinterpret_cast< fbxsdk::FbxSkin * >( mesh->GetDeformer( 0, fbxsdk::FbxDeformer::eSkin ) );
	const std::unordered_map< std::string, int > & boneNameToIdx = me->boneNameToIdx;

	for ( int clusterIdx = 0; clusterIdx < deformer->GetClusterCount(); ++clusterIdx ) {
		fbxsdk::FbxCluster * cluster = deformer->GetCluster( clusterIdx );

		const std::string boneName = cluster->GetLink()->GetName();
		if ( boneNameToIdx.find( boneName ) == boneNameToIdx.end() ) {
			assert( !"Bone did not exist in the bone map!" );
			continue;
		}

		const FbxVector4 lT = meshNode->GetGeometricTranslation( FbxNode::eSourcePivot );
		const FbxVector4 lR = meshNode->GetGeometricRotation( FbxNode::eSourcePivot );
		const FbxVector4 lS = meshNode->GetGeometricScaling( FbxNode::eSourcePivot );
		FbxAMatrix geometryTransform = FbxAMatrix( lT, lR, lS ); // Additional deformation to this vertex ( why??? ), always identity

		FbxAMatrix meshTransform;		// The transformation of the mesh at binding time
		FbxAMatrix boneTransformTPose;	// The transformation of the cluster(joint) at binding time from joint space to world space
		cluster->GetTransformMatrix( meshTransform );
		cluster->GetTransformLinkMatrix( boneTransformTPose );

		// calculate final transform of this bone, in world space, in t-pose
		FbxAMatrix invBindPose = boneTransformTPose.Inverse() * meshTransform * geometryTransform;
		FbxAMatrix bindPose    = boneTransformTPose			  * meshTransform * geometryTransform;

		me->FbxInvBindPoseMatrices[ boneNameToIdx.at( boneName ) ] = invBindPose;
		me->InvBindPoseMatrices[ boneNameToIdx.at( boneName ) ]	   = BoneTransform( &invBindPose.GetQ(), &invBindPose.GetT() );
		me->BindPoseMatrices[ boneNameToIdx.at( boneName ) ]       = BoneTransform( &bindPose.GetQ(), &bindPose.GetT() );
	}
}

void SkinnedData::PreProcessSceneData( fbxsdk::FbxScene * scene ) {
	auto start = std::chrono::high_resolution_clock::now();

	fbxsdk::FbxNode * pRootNode = scene->GetRootNode();
	for ( int i = 0; i < scene->GetSrcObjectCount< FbxAnimStack >(); i++ ) {
		fbxsdk::FbxAnimStack * curStack = scene->GetSrcObject< FbxAnimStack >( i );
		animations.insert( { curStack->GetName(), AnimationClip() } );
	}

	// single threaded
	FbxUtil::ProcessNodes_R( pRootNode, { &FbxNodeParsers::PopulateBoneAnimsCB, &FbxNodeParsers::PopulateVertsDataCB }, this );

	FbxInvBindPoseMatrices.assign( BoneCount(), fbxsdk::FbxAMatrix() );
	InvBindPoseMatrices.assign( BoneCount(), BoneTransform() );
	BindPoseMatrices.assign( BoneCount(), BoneTransform() );

	printf( "Preprocessed anim data in %i ms!\n", 
			std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::high_resolution_clock::now() - start ).count() 
	);
}

void SkinnedData::HarvestSceneData( fbxsdk::FbxScene * pScene ) {
	auto start = std::chrono::high_resolution_clock::now();

	fbxsdk::FbxNode * pRootNode = pScene->GetRootNode();
	FbxUtil::ProcessNodes_Q( pRootNode, { nullptr, &populateTPoseCB }, this );
//	FbxUtil::ProcessNodes_R( pRootNode, { nullptr, &populateTPoseCB }, this );

	printf( "Harvested anim data on %i threads in %i ms!\n",
			std::min( std::thread::hardware_concurrency(), NUM_THREADS_LOAD ),
			std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::high_resolution_clock::now() - start ).count()
	);
}

void SkinnedData::Set( fbxsdk::FbxScene * scene ) {
	if ( scene == nullptr ) {
		assert( !"ERROR - trying to load data from an empty FbxScene!" );
		return;
	}
	fbxScene = scene;


	// Single threaded
	PreProcessSceneData( fbxScene );

	// Multithreaded
	HarvestSceneData( fbxScene );
}

void SkinnedData::GetFinalTransformsGlobal( const std::string & cName, float time, std::vector<fbxsdk::FbxAMatrix> & outFinalTransforms ) const {
	const int boneCount		   = BoneCount();
	const AnimationClip & clip = animations.at( cName );

	clip.Interpolate( time, outFinalTransforms, true );

	for ( int i = 0; i < boneCount; i++ ) {
		if ( skeletonType == AnimationAssets::eSkeleton::SKINNED_MESH ) {
			outFinalTransforms[ i ] = outFinalTransforms[ i ] * FbxInvBindPoseMatrices[ i ];
		}
	}
}

void SkinnedData::GetFinalTransformsGlobal( const std::string & cName, float time, std::vector<BoneTransform> & outFinalTransforms ) const {
	const int boneCount		   = BoneCount();
	const AnimationClip & clip = animations.at( cName );

	for ( int i = 0; i < boneCount; i++ ) {
		if ( skeletonType == AnimationAssets::eSkeleton::SKINNED_MESH ) {
			outFinalTransforms[ i ] = InvBindPoseMatrices[ i ];
		} else {
			outFinalTransforms[ i ] = BoneTransform(); // identity
		}
	}

	std::vector< BoneTransform > interpd( BoneCount(), BoneTransform() );
	clip.Interpolate( time, interpd, true );
	for ( int i = 0; i < boneCount; i++ ) {
		outFinalTransforms[ i ] *= interpd[ i ];
	}
}

void SkinnedData::GetFinalTransformsLocal( const std::string & cName, float time, std::vector<BoneTransform> & outFinalTransforms ) const {
	const int boneCount		   = BoneCount();
	const AnimationClip & clip = animations.at( cName );

	// get interpolated transform for every bone at THIS TIME
	std::vector< BoneTransform > interpolatedBoneSpaceTransforms( boneCount );
	clip.Interpolate( time, interpolatedBoneSpaceTransforms, false );

	// bring all the interpolated bones into model space ( accumulate from root to leaf )
	for ( int i = 1; i < boneCount; i++ ) {
		BoneSpaceToModelSpace( i, interpolatedBoneSpaceTransforms );
	}

	outFinalTransforms = std::vector< BoneTransform >( boneCount, BoneTransform::Identity() );

	for ( int i = 0; i < boneCount; i++ ) {
		outFinalTransforms[ i ] *= interpolatedBoneSpaceTransforms[ i ];
	}

	if ( skeletonType == AnimationAssets::eSkeleton::SKINNED_MESH ) {
		for ( int i = 0; i < boneCount; i++ ) {
			outFinalTransforms[ i ] *= InvBindPoseMatrices[ i ];
		}
	}
}