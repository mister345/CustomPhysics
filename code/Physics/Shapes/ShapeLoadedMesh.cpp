//
//  ShapeLoadedMesh.cpp
//
#include "ShapeLoadedMesh.h"
#include "../../Animation/Bone.h"
#include "../../Animation/fbxInclude.h"

/*
========================================================================================================

ShapeLoadedMesh

========================================================================================================
*/

/*
====================================================
ShapeLoadedMesh::InertiaTensor
====================================================
*/
Mat3 ShapeLoadedMesh::InertiaTensorGeometric() const {
	// NOTE - intertia tensor requires mass, but mass is a property of Body, which is downstream from Shape.
	// So calculate the partial tensor matrix without, and let Body apply the mass
	const float diagValNoMass = 2.f * m_radius * m_radius / 5.f;

	Mat3 tensorWithoutMass;
	tensorWithoutMass.Zero();
	tensorWithoutMass.rows[ 0 ][ 0 ] = diagValNoMass;
	tensorWithoutMass.rows[ 1 ][ 1 ] = diagValNoMass;
	tensorWithoutMass.rows[ 2 ][ 2 ] = diagValNoMass;

	return tensorWithoutMass;
}

// world space
Bounds ShapeLoadedMesh::GetBounds( const Vec3 & pos, const Quat & orient ) const {
	Bounds bounds;
	bounds.mins = Vec3( -m_radius ) + pos;
	bounds.maxs = Vec3( m_radius ) + pos;
	return bounds;
}

// local space
Bounds ShapeLoadedMesh::GetBounds() const {
	Bounds bounds;
	bounds.mins = Vec3( -m_radius );
	bounds.maxs = Vec3( m_radius );
	return bounds;
}

extern bool * g_isPaused;
void ShapeLoadedMesh::PopulateMatrixPalette( void * bTransforms ) {
	//std::vector< BoneTransform > & boneTransforms = *reinterpret_cast< std::vector< BoneTransform > * >( bTransforms );

	// V1
	//for ( int i = 0; i < boneTransforms.size(); i++ ) {
	//	BoneTransform & t = boneTransforms[ i ];
	//	const Mat3 rotMat = t.rotation.ToMat3();
	//	//matrixPalette[ i ] = {
	//	//	Vec4{ rotMat.rows[ 0 ][ 0 ], rotMat.rows[ 0 ][ 1 ], rotMat.rows[ 0 ][ 2 ], t.translation.x },
	//	//	Vec4{ rotMat.rows[ 1 ][ 0 ], rotMat.rows[ 1 ][ 1 ], rotMat.rows[ 1 ][ 2 ], t.translation.y },
	//	//	Vec4{ rotMat.rows[ 2 ][ 0 ], rotMat.rows[ 2 ][ 1 ], rotMat.rows[ 2 ][ 2 ], t.translation.z },
	//	//	Vec4{ 0, 0, 0, 1 }
	//	//};
	// 
	// // V1A - broken
	//	matrixPalette[ i ] = {
	//		Vec4{ rotMat.rows[ 0 ][ 0 ], rotMat.rows[ 0 ][ 1 ], rotMat.rows[ 0 ][ 2 ], 0 },
	//		Vec4{ rotMat.rows[ 1 ][ 0 ], rotMat.rows[ 1 ][ 1 ], rotMat.rows[ 1 ][ 2 ], 1 },
	//		Vec4{ rotMat.rows[ 2 ][ 0 ], rotMat.rows[ 2 ][ 1 ], rotMat.rows[ 2 ][ 2 ], 2 },
	//		Vec4{ t.translation.x, t.translation.y, t.translation.z, 1 }
	//	};
	//}

	// V2 - broken
	//for ( int i = 0; i < boneTransforms.size(); i++ ) {
	//	BoneTransform & t = boneTransforms[ i ];
	//	const Mat3 rotMat = t.rotation.ToMat3();
	//	matrixPalette[ i ] = {
	//		// Transpose the rotation matrix by switching rows and columns
	//		Vec4{ rotMat.rows[ 0 ][ 0 ], rotMat.rows[ 1 ][ 0 ], rotMat.rows[ 2 ][ 0 ], 0 },
	//		Vec4{ rotMat.rows[ 0 ][ 1 ], rotMat.rows[ 1 ][ 1 ], rotMat.rows[ 2 ][ 1 ], 0 },
	//		Vec4{ rotMat.rows[ 0 ][ 2 ], rotMat.rows[ 1 ][ 2 ], rotMat.rows[ 2 ][ 2 ], 0 },
	//		Vec4{ t.translation.x, t.translation.y, t.translation.z, 1 }
	//	};
	//}

	// V3
	std::vector< fbxsdk::FbxAMatrix > & boneTransforms = 
		*reinterpret_cast< std::vector< fbxsdk::FbxAMatrix > * >( bTransforms );

	for ( int paletteIdx = 0; paletteIdx < boneTransforms.size(); paletteIdx++ ) {
		fbxsdk::FbxAMatrix & inFbxMat = boneTransforms[ paletteIdx ];
		Mat4 & outMat4				  = matrixPalette[ paletteIdx ];

		//outMat4 = {
		//	Vec4{ ( float )inFbxMat.mData[ 0 ][ 0 ], ( float )inFbxMat.mData[ 0 ][ 1 ], ( float )inFbxMat.mData[ 0 ][ 2 ], ( float )inFbxMat.mData[ 0 ][ 3 ] },
		//	Vec4{ ( float )inFbxMat.mData[ 1 ][ 0 ], ( float )inFbxMat.mData[ 1 ][ 1 ], ( float )inFbxMat.mData[ 1 ][ 2 ], ( float )inFbxMat.mData[ 1 ][ 3 ] },
		//	Vec4{ ( float )inFbxMat.mData[ 2 ][ 0 ], ( float )inFbxMat.mData[ 2 ][ 1 ], ( float )inFbxMat.mData[ 2 ][ 2 ], ( float )inFbxMat.mData[ 2 ][ 3 ] },
		//	Vec4{ ( float )inFbxMat.mData[ 3 ][ 0 ], ( float )inFbxMat.mData[ 3 ][ 1 ], ( float )inFbxMat.mData[ 3 ][ 2 ], ( float )inFbxMat.mData[ 3 ][ 3 ] },
		//};

		// Transpose the elements because FbxAMatrix is column-major and Mat4 is row-major
		//for ( int i = 0; i < 4; i++ ) {
		//	for ( int j = 0; j < 4; j++ ) {
		//		outMat4.rows[ i ][ j ] = inFbxMat[ j ][ i ];
		//	}
		//}
		for ( int i = 0; i < 4; i++ ) {
			for ( int j = 0; j < 4; j++ ) {
				outMat4.rows[ i ][ j ] = inFbxMat[ j ][ i ];
			}
		}
	}

	//if ( !*g_isPaused ) {
	//	static int hitCount = 0;
	//	printf( "~~ \tpopulating matrix palette for the %ith time: boneTransforms:%llu\tmatPaletteSz:%llu \n", 
	//			hitCount++, 
	//			boneTransforms.size(),
	//			matrixPalette.size()
	//	);
	//}
}
