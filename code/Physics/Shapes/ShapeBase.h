//
//	ShapeBase.h
//
#pragma once
#include "../../Math/Vector.h"
#include "../../Math/Quat.h"
#include "../../Math/Matrix.h"
#include "../../Math/Bounds.h"
#include <vector>

enum shapeColor_t {
	DEFAULT = 0,
	RED = 1,
	BLUE = 2,
};

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
		SHAPE_LOADED_MESH,
	};

	virtual shapeType_t GetType() const = 0;

	shapeColor_t color = DEFAULT;

protected:
	Vec3 m_centerOfMass;
};
