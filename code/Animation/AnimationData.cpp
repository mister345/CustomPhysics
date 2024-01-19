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
#include <cmath>
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

//typedef void( *onFoundTPose_fn )( fbxsdk::FbxNode * n, void * me );
//void tPoseCB( fbxsdk::FbxNode * n, void * me ) {
//	SkinnedData * skinnedData = ( SkinnedData * )me;
//	const char * name = n->GetName();
//	if ( skinnedData->boneNameToIdx.find( name ) == skinnedData->boneNameToIdx.end() ) {
//		return;
//	}
//
//	// experiment 0 - this is just completely broken, looks like it's in the wrong space
//	// NOTE - it looks like the rotation has been zero'd out in this pose!
//	fbxsdk::FbxVector4 eulerRot = n->LclRotation.Get();
//	fbxsdk::FbxVector4 trans    = n->LclTranslation.Get();
//
//	double rollRad  = FBXSDK_PI_DIV_180 * ( eulerRot[ 0 ] ); // X
//	double pitchRad = FBXSDK_PI_DIV_180 * ( eulerRot[ 1 ] ); // Y
//	double yawRad   = FBXSDK_PI_DIV_180 * ( eulerRot[ 2 ] ); // Z
//	fbxsdk::FbxQuaternion quat;
//	quat.ComposeSphericalXYZ( fbxsdk::FbxVector4( rollRad, pitchRad, yawRad ) );
//	quat.Normalize();
//
//	fbxsdk::FbxVector4 scaling = n->LclScaling.Get();
//
//	// experiment 1 - note, this doesnt get the t-pose, it gets the anim pose at current anim stack
////	fbxsdk::FbxAMatrix transfrom = n->EvaluateGlobalTransform( fbxsdk::FBXSDK_TIME_INFINITE );
//	//fbxsdk::FbxVector4 trans   = transfrom.GetT();
//	//fbxsdk::FbxQuaternion quat = transfrom.GetQ();
//
//	skinnedData->InvBindPoseMatrices[ skinnedData->boneNameToIdx[ name ] ] = { &quat, &trans };
//
//	// WRONG - these values are already poluted w transforms other than bind poses!
//	//skinnedData->BindPoseMatrices[ skinnedData->boneNameToIdx[ name ] ] =
//	//	skinnedData->InvBindPoseMatrices[ skinnedData->boneNameToIdx[ name ] ].Inverse();
//
//	//{
//	//	writeToDebugLog( CLUSTERS,
//	//					 "\t{\"%i\":\"%s\",\"transform\":{\"euler\":"
//	//					 "[%.0f,%.0f,%.0f],\"quat\":[%.4f,%.4f,%.4f,%.4f],"
//	//					 "\"pos\":[%.0f,%.0f,%.0f],\"scale\":[%.0f,%.0f,%.0f]}},\n",
//	//					 boneIdx, name,
//	//					 eulerRot[ 0 ], eulerRot[ 1 ], eulerRot[ 2 ],
//	//					 quaternion[ 0 ], quaternion[ 1 ], quaternion[ 2 ], quaternion[ 3 ],
//	//					 translation[ 0 ], translation[ 1 ], translation[ 2 ],
//	//					 scaling[ 0 ], scaling[ 1 ], scaling[ 2 ] 
//	//	);
//	//}
//}

//////////////////////////////////////////////////////////////////////////////////////////////
// https://www.gamedev.net/articles/programming/graphics/how-to-work-with-fbx-sdk-r3582/
//////////////////////////////////////////////////////////////////////////////////////////////
void tPoseCB_v2( void * user, fbxsdk::FbxNode * meshNode ) {
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

		//GetBindPose( meshNode, cluster, outBindPose );
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
		FbxAMatrix bindPose    = boneTransformTPose * meshTransform * geometryTransform;

		me->InvBindPoseMatrices[ boneNameToIdx.at( boneName ) ] = BoneTransform( &invBindPose.GetQ(), &invBindPose.GetT() );
		me->BindPoseMatrices[ boneNameToIdx.at( boneName ) ]    = BoneTransform( &bindPose.GetQ(), &bindPose.GetT() );
	}
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

	// get bone and vert data
	openDebugLog( BONES );
	FbxUtil::HarvestSceneData( fbxScene, { &FbxNodeParsers::OnFoundBoneCB, &FbxNodeParsers::OnFoundMeshCB }, this );
	closeDebugLog( BONES );

	openDebugLog( BINDPOSES_BEFORE );
	for ( int i = 0; i < BindPoseMatrices.size(); i++ ) {
		BoneTransform bt = BindPoseMatrices[ i ];
		Quat & q = bt.rotation;
		writeToDebugLog( BINDPOSES_BEFORE,
						 "\t{\"%i\":\"%s\",\"transform\":{\"quat\":[%.4f,%.4f,%.4f,%.4f],"
						 "\"pos\":[%.0f,%.0f,%.0f],\"scale\":[%.0f,%.0f,%.0f]}},\n",
						 i, boneIdxToName[ i ].c_str(),
						 bt.rotation.x, bt.rotation.y, bt.rotation.z, bt.rotation.w,
						 bt.translation[ 0 ], bt.translation[ 1 ], bt.translation[ 2 ],
						 1.f, 1.f, 1.f
		);
	}
	closeDebugLog( BINDPOSES_BEFORE );

	// @TODO - either these bind pose matrices are in local space, or something else is wrong
	openDebugLog( CLUSTERS );

//	FbxUtil::HarvestTPose( fbxScene, &tPoseCB, this );
	// try a different tpose func
	FbxUtil::HarvestSceneData( fbxScene, { nullptr, &tPoseCB_v2 }, this );

	closeDebugLog( CLUSTERS );

	openDebugLog( BINDPOSES_AFTER );
	for ( int i = 0; i < BindPoseMatrices.size(); i++ ) {
		BoneTransform bt = BindPoseMatrices[ i ];
		Quat & q		 = bt.rotation;
		writeToDebugLog( BINDPOSES_AFTER,
						 "\t{\"%i\":\"%s\",\"transform\":{\"quat\":[%.4f,%.4f,%.4f,%.4f],"
						 "\"pos\":[%.0f,%.0f,%.0f],\"scale\":[%.0f,%.0f,%.0f]}},\n",
						 i, boneIdxToName[ i ].c_str(),
						 bt.rotation.x, bt.rotation.y, bt.rotation.z, bt.rotation.w,
						 bt.translation[ 0 ], bt.translation[ 1 ], bt.translation[ 2 ],
						 1.f, 1.f, 1.f
		);
	}
	closeDebugLog( BINDPOSES_AFTER );
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