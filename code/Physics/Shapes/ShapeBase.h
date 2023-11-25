//
//	ShapeBase.h
//
#pragma once
#include "../../Math/Vector.h"
#include "../../Math/Quat.h"
#include "../../Math/Matrix.h"
#include "../../Math/Bounds.h"
#include <vector>

/*
====================================================
Shape
====================================================
*/
class Shape {
public:
	enum shapeType_t {
		SHAPE_SPHERE,
		SHAPE_BOX,
		SHAPE_CONVEX,
	};

	virtual shapeType_t GetType() const = 0;
	virtual Mat3 InertiaTensorGeometric() const = 0;
	virtual Vec3 GetCenterOfMass() const { return m_centerOfMass; }

protected:
	Vec3 m_centerOfMass;
};
