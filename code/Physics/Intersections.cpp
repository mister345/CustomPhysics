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
Intersect - Ray Sphere
====================================================
*/
bool RaySphere( const Vec3 & rayStart, const Vec3 & rayDir, const Vec3 & sphereCenter, const float sphereRadii, float & t1, float & t2 ) {
	const Vec3 pathToSphere		  = sphereCenter - rayStart;

	const float rayDirMagSq		  = rayDir.Dot( rayDir );				// a
	const float rayParallelToPath = pathToSphere.Dot( rayDir );			// b
	// after accounting for the sum of both radii, 
	// we see the distance for that outermost edge, from our raycast
	// starting point. we need to solve for the scalar ( time, traversal ),
	// that will get us from start point to that distance
	const float distToSphereSurfaceSq = 
		pathToSphere.Dot( pathToSphere ) - sphereRadii * sphereRadii;	// c

	const float discriminant = rayParallelToPath * rayParallelToPath - rayDirMagSq * distToSphereSurfaceSq;
	const float invA		 = 1.f / rayDirMagSq;

	if ( discriminant < 0 ) {
		// no solution exists
		return false;
	}

	const float sqrtDiscriminant = sqrtf( discriminant );
	t1 = invA * ( rayParallelToPath - sqrtDiscriminant );
	t2 = invA * ( rayParallelToPath + sqrtDiscriminant );

	return true;
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






















