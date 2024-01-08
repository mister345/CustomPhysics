//
//  ShapeLoadedMesh.cpp
//
#include "ShapeLoadedMesh.h"
#include "../../Animation/Bone.h"

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
	bounds.maxs = Vec3(  m_radius ) + pos;
	return bounds;
}

// local space
Bounds ShapeLoadedMesh::GetBounds() const {
	Bounds bounds;
	bounds.mins = Vec3( -m_radius );
	bounds.maxs = Vec3(  m_radius );
	return bounds;
}

void ShapeLoadedMesh::PopulateMatrixPalette( void * bTransforms ) {
	std::vector< BoneTransform > & boneTransforms = *reinterpret_cast< std::vector< BoneTransform > * >( bTransforms );

	for ( int i = 0; i < boneTransforms.size(); i++ ) {
		BoneTransform & t = boneTransforms[ i ];
		const Mat3 rotMat = t.rotation.ToMat3();
		matrixPalette.emplace_back( 
			Vec4{ rotMat.rows[ 0 ][ 0 ], rotMat.rows[ 0 ][ 1 ], rotMat.rows[ 0 ][ 2 ], t.translation.x },
			Vec4{ rotMat.rows[ 1 ][ 0 ], rotMat.rows[ 1 ][ 1 ], rotMat.rows[ 1 ][ 2 ], t.translation.y },
			Vec4{ rotMat.rows[ 2 ][ 0 ], rotMat.rows[ 2 ][ 1 ], rotMat.rows[ 2 ][ 2 ], t.translation.z },
			Vec4{ 0, 0, 0, 1 }
		);
	}
}
