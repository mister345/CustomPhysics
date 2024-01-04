#include <cassert>
#include "AnimationData.h"
#include "FbxNodeParsers.h"
#include "fbxInclude.h"

namespace FbxNodeParsers {
	void OnFoundBoneCB( void * user, fbxsdk::FbxNode * boneNode ) {
		assert( boneNode->GetNodeAttribute()->GetAttributeType() == fbxsdk::FbxNodeAttribute::EType::eSkeleton );

		SkinnedData * me = reinterpret_cast< SkinnedData * >( user );
		const char * boneName = boneNode->GetName();

		/////////////////////////////////////////////////////////////////
		// Populate OffsetMatrices array Bone Map, and Bone Hierarchy
		/////////////////////////////////////////////////////////////////
		// @BUG - this is NOT getting the bind pose; it's getting the pose @ time 0 of top anim in stack!
		fbxsdk::FbxAMatrix localTransform = boneNode->EvaluateLocalTransform( FBXSDK_TIME_INFINITE ); // infinite gets default w/o any anims
		fbxsdk::FbxVector4 translation = localTransform.GetT();
		fbxsdk::FbxQuaternion rotation = localTransform.GetQ();
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

	void OnFoundMeshCB( void * user, fbxsdk::FbxNode * meshNode ) {
		assert( meshNode->GetNodeAttribute()->GetAttributeType() == fbxsdk::FbxNodeAttribute::EType::eMesh );

		SkinnedData * me = reinterpret_cast< SkinnedData * >( user );
		const char * meshName = meshNode->GetName();

	}

} // namespace FbxNodeParsers
