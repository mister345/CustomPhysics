//
//	ShapeLoadedMesh.h
//
#pragma once
#include "ShapeBase.h"

/*
====================================================
ShapeLoadedMesh
====================================================
*/
class ShapeLoadedMesh : public Shape {
public:
	explicit ShapeLoadedMesh( const float radius = 10.f ) : m_radius( radius ) {
		m_centerOfMass.Zero();
	}
	Mat3 InertiaTensorGeometric() const override;

	Bounds GetBounds( const Vec3 & pos, const Quat & orient ) const override;
	Bounds GetBounds() const override;

	shapeType_t GetType() const override { return SHAPE_LOADED_MESH; }

public:
	// @TODO - we just copied implementation from sphere for now to get this working,
	// since we wont even be using physics for this shape yet...
	// after I add a convex body shape to the code base, I will just copy the implementation
	// to some approximation of that
	float m_radius;
};