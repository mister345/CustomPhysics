#include <cassert>
#include <cmath>
#include "fbxInclude.h"
#include "AnimationData.h"
#include "FbxNodeParsers.h"
#include "ModelLoader.h"
#include "../Renderer/model.h"
#include "../Config.h"

namespace FbxNodeParsers {
	void PopulateBoneAnimsCB( void * user, fbxsdk::FbxNode * boneNode ) {
		assert( boneNode->GetNodeAttribute()->GetAttributeType() == fbxsdk::FbxNodeAttribute::EType::eSkeleton );

		SkinnedData * me = reinterpret_cast< SkinnedData * >( user );

		// try register a new bone. if it's already in our map, our job here is done
		if ( !me->boneNameToIdx.insert( { boneNode->GetName(), me->BoneCount() } ).second ) {
			return;
		}
		me->boneIdxToName.insert( { me->BoneCount() - 1, boneNode->GetName() } );

		// populate the bone hierarchy, telling bone @ which idx is parent to which
		const char * parentName = boneNode->GetParent()->GetName();
		const bool bFoundParent = me->boneNameToIdx.find( parentName ) != me->boneNameToIdx.end();
		me->BoneHierarchy.push_back( bFoundParent ? me->boneNameToIdx[ parentName ] : -1 );

		// Get all animations for this bone
		for ( int i = 0; i < me->fbxScene->GetSrcObjectCount< FbxAnimStack >(); i++ ) {
			fbxsdk::FbxAnimStack * stack = me->fbxScene->GetSrcObject< FbxAnimStack >( i );
			me->fbxScene->SetCurrentAnimationStack( stack );
			AnimationClip & clip = me->animations[ stack->GetName() ];
			clip.BoneAnimations.emplace_back( me->fbxScene, boneNode, false );
			clip.BoneAnimationsGlobal.emplace_back( me->fbxScene, boneNode, true );
		}
	}

	/*
		Ported from fbxsdk - ViewScene demo
	*/
	void ComputeClusterDeformation( fbxsdk::FbxCluster * pCluster,
									fbxsdk::FbxAMatrix & outVertexTransformMatrix,
									fbxsdk::FbxTime pTime ) {
		FbxAMatrix lReferenceGlobalInitPosition;
		FbxAMatrix lClusterGlobalInitPosition;
		FbxAMatrix lClusterGlobalCurrentPosition;

		pCluster->GetTransformMatrix( lReferenceGlobalInitPosition );
		pCluster->GetTransformLinkMatrix( lClusterGlobalInitPosition );
		lClusterGlobalCurrentPosition = pCluster->GetLink()->EvaluateGlobalTransform( pTime );	// same as above; works
		outVertexTransformMatrix = lClusterGlobalCurrentPosition * lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;
	}

	void validateBoneWeight( int & idx, double & weight ) {
		if ( std::fpclassify( weight ) == FP_SUBNORMAL ) {
			idx = -1;
			weight = 0.f;
			printf( "\nbone weight was denormal! invalidating...\n" );
		}
	}

	bool TryPopulateBoneWeights( vertSkinned_t & outVert, int vertIdx, fbxsdk::FbxSkin * skinDeformer, SkinnedData * me ) {
		constexpr int MAX_BONES_PER_VERT = 4;

		if ( skinDeformer == nullptr ) {
			return false;
		}

		for ( int i = 0; i < MAX_BONES_PER_VERT; ++i ) {
			outVert.boneWeights[ i ] = 0.0f;
			outVert.boneIdxes[ i ] = -1;
		}
		int clusterCount = skinDeformer->GetClusterCount();

		for ( int clusterIdx = 0; clusterIdx < clusterCount; ++clusterIdx ) {
			fbxsdk::FbxCluster * cluster = skinDeformer->GetCluster( clusterIdx );
			// links are basically the real bones
			if ( !cluster->GetLink() ) {
				continue;
			}

			// Get the indices and weights
			int * ctrlPtIdxes	   = cluster->GetControlPointIndices();
			const int numCtrlPts   = cluster->GetControlPointIndicesCount();
			double * inVertWeights = cluster->GetControlPointWeights();

			// note - fbxsdk inverts bone-vert relationship; each cluster ( bone ) has a list of VERTS that it affects
			// ( but in the shader, we need each VERT to have a list of BONES that affect IT
			for ( int ctrlPtIdx = 0; ctrlPtIdx < numCtrlPts; ++ctrlPtIdx ) {
				if ( ctrlPtIdxes[ ctrlPtIdx ] == vertIdx ) {
					for ( int boneWtIdx = 0; boneWtIdx < MAX_BONES_PER_VERT; ++boneWtIdx ) {
						if ( outVert.boneWeights[ boneWtIdx ] == 0.0f ) {

							// @WARNING - cluster idx is NOT the same as the bone idx!
							const char * boneName = cluster->GetLink()->GetName();
							int realBoneIdx		  = me->boneNameToIdx[ boneName ];
							double inWt			  = inVertWeights[ ctrlPtIdx ];
							validateBoneWeight( realBoneIdx, inWt );

							outVert.boneIdxes[ boneWtIdx ]   = realBoneIdx;
							outVert.boneWeights[ boneWtIdx ] = static_cast< float >( inWt );
							break;
						}
					}
				}
			}
		}
		return true;
	}

	void PopulateVertsDataCB( void * user, fbxsdk::FbxNode * meshNode ) {
		assert( meshNode->GetNodeAttribute()->GetAttributeType() == fbxsdk::FbxNodeAttribute::EType::eMesh );

		SkinnedData * me	   = reinterpret_cast< SkinnedData * >( user );
		fbxsdk::FbxMesh * mesh = reinterpret_cast< FbxMesh * >( meshNode->GetNodeAttribute() );

		me->numVerts	  = mesh->GetControlPointsCount();
		me->numIdxes	  = mesh->GetPolygonVertexCount();
		me->renderedVerts = reinterpret_cast< vertSkinned_t * >( malloc( sizeof( vertSkinned_t ) * me->numVerts ) );
		me->idxes		  = reinterpret_cast< int * >( malloc( sizeof( int ) * me->numIdxes ) );
		printf( "found mesh named:%s with %i vertices and %i indices. Copying verts and incides...\n", 
				meshNode->GetName(), me->numVerts, me->numIdxes );

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
			//printf( "Vertex %i: %3.1f, %3.1f, %3.2f, uv:%1.1f,%1.1f\n", 
			//		i, outVert.xyz[ 0 ], outVert.xyz[ 1 ], outVert.xyz[ 2 ], outVert.st[ 0 ], outVert.st[ 1 ] );

			// @TODO - load the bone weights and bone indices for each of these verts
			// currently seed w random values to check if they're showing up in vtx shader in renderdoc
			const bool populated = TryPopulateBoneWeights( 
				outVert, 
				i, 
				reinterpret_cast< fbxsdk::FbxSkin * >( mesh->GetDeformer( 0, fbxsdk::FbxDeformer::eSkin ) ),
				me
			);
			assert( populated );
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
