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

Mat3 Body::GetInverseInertiaTensorBodySpace() const {
// NOTE - inverting matrix, then multiplying by inv mass, is the same as multiplying by mass, then inverting
// ( bc of commutable scalar multiplication by mass/invmass, mass gets inverted in the process of inverting the matrix )
    const Mat3 tensorGeom        = m_shape->InertiaTensorGeometric();
    const Mat3 invTensorGeom     = tensorGeom.Inverse();
    const Mat3 invInertialTensor = invTensorGeom * m_invMass;
    return invInertialTensor;
}

Mat3 Body::GetInverseInertiaTensorWorldSpace() const {
    const Mat3 invInertialTensor = GetInverseInertiaTensorBodySpace();

    // @TODO, why sandwich product? basically queueing up ( encoding ) the following transformation sequence?
    // [ change-of-basis to world space >> transform by inv inertial tensor >> change-of-basis back to local ]
    const Mat3 orientation = m_orientation.ToMat3();
    Mat3 invITensorWorld   = orientation * invInertialTensor * orientation.Transpose();
    return invITensorWorld;
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

void Body::ApplyImpulseAngular( const Vec3 & impulse ) {
    // 0 would imply infinitely massive object unaffected by impulses
    if ( m_invMass = 0.f ) {
        return;
    }

    // Angular momentum   = Inertia tensor     * Angular velocity         = radius CROSS momentum
    // D Angular momentum = Inertia tensor     * D Angular velocity       = radius CROSS impulse
    // THEREFORE : 
    // D angular velocity = inv Inertia tensor * ( radius CROSS impulse )
    m_angularVelocity += GetInverseInertiaTensorWorldSpace() * impulse;

    const float maxAngularSpeed = 30.f;
    if ( m_angularVelocity.GetLengthSqr() > maxAngularSpeed * maxAngularSpeed ) {
        m_angularVelocity.Normalize();
        m_angularVelocity *= maxAngularSpeed;
    }
}
