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

		me->numVerts	  = mesh->GetControlPointsCount();
		me->numIdxes	  = mesh->GetPolygonVertexCount();
		me->renderedVerts = reinterpret_cast< vertSkinned_t * >( malloc( sizeof( vertSkinned_t ) * me->numVerts ) );
		me->idxes		  = reinterpret_cast< int * >( malloc( sizeof( int ) * me->numIdxes ) );
		printf( "found mesh named:%s with %i vertices and %i indices. Copying verts and incides...\n", meshNode->GetName(), me->numVerts, me->numIdxes );

		// Load uvs if we have them
		fbxsdk::FbxStringList uvSetNameList;
		mesh->GetUVSetNames( uvSetNameList );
		const char * uvSetName = uvSetNameList.GetCount() > 0 ? uvSetNameList[ 0 ] : nullptr;

		// Load vert, uv data into our receptacle
		for ( int i = 0; i < me->numVerts; i++ ) {
			vertSkinned_t & outVert = me->renderedVerts[ i ];

			// coordinates
			fbxsdk::FbxVector4 fbxVert = mesh->GetControlPointAt( i );
			outVert.xyz[ 0 ]		   = static_cast< float >( fbxVert[ 0 ] );
			outVert.xyz[ 1 ]		   = static_cast< float >( fbxVert[ 1 ] );
			outVert.xyz[ 2 ]		   = static_cast< float >( fbxVert[ 2 ] );

			// UVs
			bool isUnmapped;
			fbxsdk::FbxVector2 uv;
			if ( mesh->GetPolygonVertexUV( 0, i, uvSetName, uv, isUnmapped ) ) {
				outVert.st[ 0 ] = static_cast< float >( uv[ 0 ] );
				outVert.st[ 1 ] = static_cast< float >( uv[ 1 ] );
			}
			printf( "Vertex %i: %3.1f, %3.1f, %3.2f, uv:%1.1f,%1.1f\n", 
					i, outVert.xyz[ 0 ], outVert.xyz[ 1 ], outVert.xyz[ 2 ], outVert.st[ 0 ], outVert.st[ 1 ] );

			// @TODO - load the bone weights and bone indices for each of these verts
			// currently seed w random values to check if they're showing up in vtx shader in renderdoc
			outVert.boneIdxes[ 0 ] = 0;
			outVert.boneIdxes[ 1 ] = 1;
			outVert.boneIdxes[ 2 ] = 2;
			outVert.boneIdxes[ 3 ] = 3;

			outVert.boneWeights[ 0 ] = 0.15f;
			outVert.boneWeights[ 1 ] = 0.35f;
			outVert.boneWeights[ 2 ] = 0.75f;
			outVert.boneWeights[ 3 ] = 1.00f;
		}

		// Load indices
		// points to an element in the raw array of vertex data; used as a point in the whole mesh
		int vertIdx = 0;
		const int numTriangles = mesh->GetPolygonCount();
		// triangleIdx points to an element in fbx's internal array of POLYGONS( assume triangles );
		// each triangle is just a grouping of three ints, representing indices
		// each of those indices will point to an element in the raw array of vertex data for the WHOLE MESH
		for ( int triangleIdx = 0; triangleIdx < numTriangles; triangleIdx++ ) {
			const int localVertCount = mesh->GetPolygonSize( triangleIdx );
			for ( int localVertIdx = 0; localVertIdx < localVertCount; localVertIdx++ ) {
				me->idxes[ vertIdx + localVertIdx ] = mesh->GetPolygonVertex( triangleIdx, localVertIdx );
			}
			vertIdx += localVertCount;
		}
	}

} // namespace FbxNodeParsers
