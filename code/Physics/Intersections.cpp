//
//  Intersections.cpp
//
#include "Intersections.h"
#include "GJK.h"
#include "Shapes/ShapeSphere.h"
#include <assert.h>

/*
====================================================
Intersect - sphere to sphere only
====================================================
*/
bool Intersect( Body * bodyA, Body * bodyB ) {
	constexpr Shape::shapeType_t sphereType = Shape::shapeType_t::SHAPE_SPHERE;
	const ShapeSphere * sphereA = reinterpret_cast< ShapeSphere * >( bodyA->m_shape );
	const ShapeSphere * sphereB = reinterpret_cast< ShapeSphere * >( bodyB->m_shape );
	assert( sphereA->GetType() == sphereType && sphereB->GetType() == sphereType );

	const float distSquared	  = ( bodyA->m_position - bodyB->m_position ).GetLengthSqr();
	const float radiusSum     = sphereA->m_radius + sphereB->m_radius;
	const float radSumSquared = radiusSum * radiusSum;

	return distSquared <= radSumSquared;
}

/*
====================================================
Intersect
====================================================
*/
bool Intersect( Body * bodyA, Body * bodyB, contact_t & contact ) {
	// TODO: Add Code

	return false;
}

/*
====================================================
Intersect
====================================================
*/
bool Intersect( Body * bodyA, Body * bodyB, const float dt, contact_t & contact ) {
	// TODO: Add Code

	return false;
}






















