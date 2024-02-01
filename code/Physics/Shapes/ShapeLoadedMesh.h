//
//	ShapeLoadedMesh.h
//
#pragma once
#include "ShapeBase.h"
#include "../../Animation/Bone.h"
#include "../../Animation/fbxInclude.h"

struct vertSkinned_t;

/*
====================================================
ShapeLoadedMesh
====================================================
*/
class ShapeLoadedMesh : public Shape {
public:
	explicit ShapeLoadedMesh( vertSkinned_t * pVerts, int numV, int * pIdxes, int nIdxes, int nBones, const float radius = 10.f ) :
		verts( pVerts ),
		numVerts( numV ),
		idxes( pIdxes ),
		numIdxes( nIdxes ),
		m_radius( radius ) { 

		m_centerOfMass.Zero(); 

		assert( nBones <= 80 );
		matrixPalette.resize( nBones );
	}
	Mat3 InertiaTensorGeometric() const override;

	Bounds GetBounds( const Vec3 & pos, const Quat & orient ) const override;
	Bounds GetBounds() const override;

	shapeType_t GetType() const override { return SHAPE_LOADED_MESH; }

	void PopulateMatrixPalette( std::vector< fbxsdk::FbxAMatrix > & boneTransforms );
	void PopulateMatrixPalette( std::vector< BoneTransform > & boneTransforms );

	inline const std::vector< Mat4 > * GetMatrixPalette() const { return &matrixPalette;  }

public:
	// @TODO - we just copied implementation from sphere for now to get this working,
	// since we wont even be using physics for this shape yet...
	// after I add a convex body shape to the code base, I will just copy the implementation
	// to some approximation of that
	float m_radius;

	// @TODO - make these weak pointers
	int numIdxes   = 0;
	int numVerts   = 0;
	int * idxes	   = nullptr;
	vertSkinned_t * verts = nullptr;

	std::vector< Mat4 > matrixPalette;
};