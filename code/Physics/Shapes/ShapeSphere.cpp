//
//  ShapeSphere.cpp
//
#include "ShapeSphere.h"

/*
========================================================================================================

ShapeSphere

========================================================================================================
*/

/*
====================================================
ShapeSphere::InertiaTensor
====================================================
*/
Mat3 ShapeSphere::InertiaTensorGeometric() const {
	// NOTE - intertia tensor requires mass, but mass is a property of Body, which is downstream from Shape.
	// So calculate the partial tensor matrix without, and let Body apply the mass
	const float diagValNoMass = 2.f * m_radius * m_radius / 5.f;

	Mat3 tensorWithoutMass;
	tensorWithoutMass.Zero();
	tensorWithoutMass.rows[ 0 ][ 0 ] = diagValNoMass;
	tensorWithoutMass.rows[ 1 ][ 1 ] = diagValNoMass;
	tensorWithoutMass.rows[ 2 ][ 2 ] = diagValNoMass;
	
	return tensorWithoutMass;
}

// world space
Bounds ShapeSphere::GetBounds( const Vec3 & pos, const Quat & orient ) const {
	Bounds bounds;
	bounds.mins = Vec3( -m_radius ) + pos;
	bounds.maxs = Vec3(  m_radius ) + pos;
	return bounds;
}

// local space
Bounds ShapeSphere::GetBounds() const {
	Bounds bounds;
	bounds.mins = Vec3( -m_radius );
	bounds.maxs = Vec3(  m_radius );
	return bounds;
}
