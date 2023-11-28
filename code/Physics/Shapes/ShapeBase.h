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
	virtual Mat3 InertiaTensorGeometric() const = 0;

	virtual Bounds GetBounds( const Vec3 & pos, const Quat & orient ) const = 0;
	virtual Bounds GetBounds() const = 0;

	virtual Vec3 GetCenterOfMass() const { return m_centerOfMass; }

	enum shapeType_t {
		SHAPE_SPHERE,
	};

	virtual shapeType_t GetType() const = 0;


protected:
	Vec3 m_centerOfMass;
};
