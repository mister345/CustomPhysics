//
//  Body.cpp
//
#include "Body.h"

/*
====================================================
Body::Body
====================================================
*/
Body::Body() :
    m_position( 0.0f ),
    m_orientation( 0.0f, 0.0f, 0.0f, 1.0f ),
    m_shape( NULL ) {
}

Vec3 Body::GetCenterOfMassWorldSpace() const {
    // rotate the center of mass as defined in MODEL space, 
    // by the world orientation of the object, it may not be at the model origin
    const Vec3 centerOfMassWorldRotation = m_orientation.RotatePoint( m_shape->GetCenterOfMass() );
    // add world offset to this rotated point
    return m_position + centerOfMassWorldRotation;
}

Vec3 Body::GetCenterOfMassModelSpace() const {
    // return center of mass respect to MODEL origin, with MODEL space orientation
    return m_shape->GetCenterOfMass();
}

Vec3 Body::WorldSpaceToBodySpace( const Vec3 & worldPt ) const {
    const Vec3 pointRelToCenterOfMass_WS = worldPt - GetCenterOfMassWorldSpace();
    const Quat orientationRelToBody      = m_orientation.Inverse();
    const Vec3 pointOrientedRelToBody    = orientationRelToBody.RotatePoint( pointRelToCenterOfMass_WS );
    return pointOrientedRelToBody;
}

Vec3 Body::BodySpaceToWorldSpace( const Vec3 & localPt ) const {
    const Vec3 pointOrientedRelToWorld      = m_orientation.RotatePoint( localPt );
    const Vec3 orientedPointWithWorldOffset = pointOrientedRelToWorld + GetCenterOfMassWorldSpace();
    return orientedPointWithWorldOffset;
}

void Body::ApplyImpulseLinear( const Vec3 & impulse ) {
    // 0 would imply infinitely massive object unaffected by impulses
    if ( m_invMass == 0.f ) {
        return;
    }

    //////////////// * D = "delta"    * //////////
    //////////////// * J = "momentum" * //////////
    //                                          //
    //         momentum   = mass * velocity     //
    // J     = momentum D = mass * velocity D   //
    // J / m = velocity D                       //
    //                                          //
    //////////////////////////////////////////////
    m_linearVelocity += impulse * m_invMass;
}
