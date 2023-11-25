//
//	ShapeSphere.h
//
#pragma once
#include "ShapeBase.h"

/*
====================================================
ShapeSphere
====================================================
*/
class ShapeSphere : public Shape {
public:
	explicit ShapeSphere( const float radius ) : m_radius( radius ) {
		m_centerOfMass.Zero();
	}
	shapeType_t GetType() const override { return SHAPE_SPHERE; }
	Mat3 InertiaTensorGeometric() const override;

public:
	float m_radius;
};