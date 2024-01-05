#include <cassert>
#include "AnimationData.h"
#include "FbxNodeParsers.h"
#include "fbxInclude.h"
#include "../Renderer/model.h"

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

		SkinnedData * me	   = reinterpret_cast< SkinnedData * >( user );
		fbxsdk::FbxMesh * mesh = reinterpret_cast< FbxMesh * >( meshNode->GetNodeAttribute() );
		const int numVerts	   = mesh->GetControlPointsCount();
		printf( "found mesh named:%s\n with %i vertices.\n", meshNode->GetName(), numVerts );

		// Check for UV set
		fbxsdk::FbxStringList uvSetNameList;
		mesh->GetUVSetNames( uvSetNameList );
		const char * uvSetName = uvSetNameList.GetCount() > 0 ? uvSetNameList[ 0 ] : nullptr;
		if ( !uvSetName ) {
			assert( !"No UV set found in the mesh.\n" );
		}

		// @TODO - we need to add a receptacle for this data somewhere first!
		me->numVerts = numVerts;
		me->renderedVerts = reinterpret_cast< vert_t * >( malloc( sizeof( vert_t ) * me->numVerts ) );
		vert_t * pVert = me->renderedVerts;

		for ( int i = 0; i < numVerts; i++ ) {
			vert_t & outVert = *pVert;
			pVert++;

			fbxsdk::FbxVector4 fbxVert = mesh->GetControlPointAt( i );
			outVert.xyz[ 0 ] = static_cast< float >( fbxVert[ 0 ] );
			outVert.xyz[ 1 ] = static_cast< float >( fbxVert[ 1 ] );
			outVert.xyz[ 2 ] = static_cast< float >( fbxVert[ 2 ] );

			fbxsdk::FbxVector2 uv;
			bool unmappedUV;
			if ( mesh->GetPolygonVertexUV( 0, i, uvSetName, uv, unmappedUV ) ) {
				outVert.st[ 0 ] = static_cast< float >( uv[ 0 ] );
				outVert.st[ 1 ] = static_cast< float >( uv[ 1 ] );
			}

			//////////////////////////////////////////////////////////
			// @TODO - idxes
			int numIdxes = 10; // @TODO - ARBITRARY!
			assert( !"THIS IS JUST TEMP AND COMPLETELY WRONG @TODO - FIX IT!!!!" );
			me->numIdxes = numIdxes;
			me->idxes = reinterpret_cast< int * >( malloc( sizeof( int ) * numIdxes ) );
			int * pIdx = me->idxes;

			// @TODO - note this fbxsdk sample mesh might not have valid uvs, just init to 0 instead!
			printf( "Vertex %i: %3.2f, %3.2f, %3.2f, uv:%3.2f,%3.2f\n", 
					i, outVert.xyz[ 0 ], outVert.xyz[ 1 ], outVert.xyz[ 2 ], 0.f/*outVert.st[ 0 ]*/, 0.f/*outVert.st[ 1 ]*/ );
		}
	}

} // namespace FbxNodeParsers
