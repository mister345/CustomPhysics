//
//  Intersections.cpp
//
#include "Intersections.h"
#include "GJK.h"
#include "Shapes/ShapeSphere.h"
#include <assert.h>

namespace {
static constexpr Shape::shapeType_t sphereType = Shape::shapeType_t::SHAPE_SPHERE;
#define IS_SPHERES( a, b ) a->GetType() == sphereType && b->GetType() == sphereType
}

/*
====================================================
Intersect - sphere to sphere only
====================================================
*/
bool Intersect( Body * bodyA, Body * bodyB ) {
	const ShapeSphere * sphereA = reinterpret_cast< ShapeSphere * >( bodyA->m_shape );
	const ShapeSphere * sphereB = reinterpret_cast< ShapeSphere * >( bodyB->m_shape );
	assert( IS_SPHERES( sphereA, sphereB ) );

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
bool Intersect( Body * bodyA, Body * bodyB, contact_t & outContact ) {
	const ShapeSphere * sphereA = reinterpret_cast< ShapeSphere * >( bodyA->m_shape );
	const ShapeSphere * sphereB = reinterpret_cast< ShapeSphere * >( bodyB->m_shape );
	assert( IS_SPHERES( sphereA, sphereB ) );

	// populate contact with relevant data
	outContact.bodyA  = bodyA;
	outContact.bodyB  = bodyB;
	const Vec3 pathAB = bodyB->m_position - bodyA->m_position;
	outContact.normal = pathAB; 
	outContact.normal.Normalize();
	outContact.ptOnA_WorldSpace = bodyA->m_position + outContact.normal * sphereA->m_radius;
	outContact.ptOnB_WorldSpace = bodyB->m_position - outContact.normal * sphereB->m_radius;

	const float radiiSumAB = sphereA->m_radius + sphereB->m_radius;
	const float distABSq   = pathAB.GetLengthSqr();

	// do the actual collision check
	if ( distABSq <= radiiSumAB * radiiSumAB ) {
		return true;
	}
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






















