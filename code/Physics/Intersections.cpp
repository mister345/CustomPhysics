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
bool RaySphere( const Vec3 & rayStart, const Vec3 & rayPath, const Vec3 & sphereCenter, const float sphereRadii, float & t1, float & t2 ) {
	const Vec3 pathToSphere		  = sphereCenter - rayStart;

	const float rayPathMagSq	  = rayPath.Dot( rayPath );				// a
	const float rayParallelToPath = pathToSphere.Dot( rayPath );		// b
	// after accounting for the sum of both radii, 
	// we see the distance for that outermost edge, from our raycast
	// starting point. we need to solve for the scalar ( time, traversal ),
	// that will get us from start point to that distance
	const float distToSphereSurfaceSq = 
		pathToSphere.Dot( pathToSphere ) - sphereRadii * sphereRadii;	// c

	const float discriminant = rayParallelToPath * rayParallelToPath - rayPathMagSq * distToSphereSurfaceSq;
	const float invA		 = 1.f / rayPathMagSq;

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
Intersect - sphere to sphere dynamic
====================================================
*/
bool SphereSphereDynamic( const ShapeSphere * shapeA, const ShapeSphere * shapeB,
						  const Vec3 & posA, const Vec3 & posB, const Vec3 & velA, const Vec3 & velB,
						  const float dt, Vec3 & ptOnA, Vec3 & ptOnB, float & toi ) {

	const Vec3 relativeVelocity = velA - velB;
	const Vec3 endPtA   = posA + relativeVelocity * dt;
	const Vec3 rayPath  = endPtA - posA;

	// two solutions
	float t0 = 0;
	float t1 = 0;
	if ( rayPath.GetLengthSqr() < 0.001f * 0.001f ) {
		// Ray is too short, just do simple intersection check
		Vec3 ab = posB - posA;
		float radii = shapeA->m_radius + shapeB->m_radius + 0.001f;
		if ( ab.GetLengthSqr() > radii * radii ) {
			return false;
		}
	} else if ( !RaySphere( posA, rayPath, posB, shapeA->m_radius + shapeB->m_radius, t0, t1) ) {
		return false;
	}

	// Scale range from 0~1 to 0~dt
	t0 *= dt;
	t1 *= dt;

	// collision was in the past, this frame will not have a collision
	if ( t1 < 0.f ) {
		return false;
	}

	// choose the earlier time of impact and clamp to 0
	toi = ( t0 < 0.f ) ? 0.f : t0;

	// if earliest collision is beyond the duration of this frame, it's not happening this frame
	if ( toi > dt ) {
		return false;
	}

	Vec3 newPosA = posA + velA * toi;
	Vec3 newPosB = posB + velB * toi;
	Vec3 ab = newPosB - newPosA;
	ab.Normalize();

	// calculate points on the surfaces of each object with respect to their radii
	ptOnA = newPosA + ab * shapeA->m_radius;
	ptOnB = newPosB - ab * shapeB->m_radius;
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
	if ( bodyA->m_shape->GetType() != Shape::SHAPE_SPHERE ||
		 bodyB->m_shape->GetType() != Shape::SHAPE_SPHERE ) {
		return false;
	}

	const ShapeSphere * sphereA = reinterpret_cast< ShapeSphere * >( bodyA->m_shape );
	const ShapeSphere * sphereB = reinterpret_cast< ShapeSphere * >( bodyB->m_shape );
	Vec3 posA = bodyA->m_position;
	Vec3 posB = bodyB->m_position;
	Vec3 velA = bodyA->m_linearVelocity;
	Vec3 velB = bodyB->m_linearVelocity;

	if ( !SphereSphereDynamic( sphereA, sphereB, posA, posB, velA, velB, dt, 
		 contact.ptOnA_WorldSpace, contact.ptOnB_WorldSpace, contact.timeOfImpact ) ) {
		return false;
	}

	// step bodies fwd in time to be at local space collision points
	bodyA->Update( contact.timeOfImpact );
	bodyB->Update( contact.timeOfImpact );

	// Convert world space contact to local space
	contact.ptOnA_LocalSpace = bodyA->WorldSpaceToBodySpace( contact.ptOnA_WorldSpace );
	contact.ptOnB_LocalSpace = bodyB->WorldSpaceToBodySpace( contact.ptOnB_WorldSpace );

	contact.normal = bodyA->m_position - bodyB->m_position;
	contact.normal.Normalize();

	// undo the step forward from above
	bodyA->Update( -contact.timeOfImpact );
	bodyB->Update( -contact.timeOfImpact );

	// Calculate separation distance
	Vec3 ab = bodyB->m_position - bodyA->m_position;
	contact.separationDistance = ab.GetMagnitude() - ( sphereA->m_radius + sphereB->m_radius );
	return true;
}






















