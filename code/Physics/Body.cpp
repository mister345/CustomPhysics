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

void Body::Update( const float dt_sec ) {
    // position
    m_position += m_linearVelocity * dt_sec;

    // orientation
    // 1. angular velocity operates relative to the center of mass, 
    //    so convert our world pos to be relative to center of mass
    //    ( use center of mass as our origin )
    const Vec3 centerOfMass = GetCenterOfMassWorldSpace();
    const Vec3 posRelToCM = m_position - centerOfMass;

    // 2. Calculate total torque
    //    -> equal to externally applied torque + precession ( internal torque about its own axis )
    //    -> ignore external torque; already applied by collision ResolveContact method ( @TODO )
    //    -> just need to calculate internal torque ( precession )

    // 3. Calculate angular velocity delta 
    //    I a =        w X I * w
    //      a = I^-1 ( w X I * w )
    const Mat3 toWorldOrientation = m_orientation.ToMat3();
    const Mat3 iTensorGeom_WS     = toWorldOrientation * m_shape->InertiaTensorGeometric() * toWorldOrientation.Transpose();
    const Vec3 deltaAngVelocity   = iTensorGeom_WS.Inverse() * ( m_angularVelocity.Cross( iTensorGeom_WS * m_angularVelocity ) );
    m_angularVelocity             += deltaAngVelocity * dt_sec;

    // @TODO ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //   we are getting the pure geometric inertia tensor here, which doesnt include mass yet...
    //   does it somehow cancel out due to cross product, or did he add the mass to the inertia tensor stored in the shape
    //   somewhere else in the code and not mention it???
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    // @TODO - review order of operations multiplying quats

    // NOTE - the quat constructor normalizes the axis for us
    // 4. Update orientation
    Vec3 deltaAngle = m_angularVelocity * dt_sec;
    Quat deltaQuat  = Quat( deltaAngle, deltaAngle.GetMagnitude() );
    m_orientation   = deltaQuat * m_orientation;
    m_orientation.Normalize();

    // 5. update position, since above rotation can also affect that
    m_position = centerOfMass + deltaQuat.RotatePoint( posRelToCM );
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
    const Mat3 iTensorGeom       = m_shape->InertiaTensorGeometric();
    const Mat3 invTensorGeom     = iTensorGeom.Inverse();
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

// apply both angular and linear impulses
void Body::ApplyImpulse( const Vec3 impulsePoint, const Vec3 & impulseLinear ) {
    ApplyImpulseLinear( impulseLinear );

    // general application, but same idea as radius if this were a sphere
    const Vec3 effectiveRadius = impulsePoint - GetCenterOfMassWorldSpace();
    const Vec3 angularImpulse  = effectiveRadius.Cross( impulseLinear );
    ApplyImpulseAngular( angularImpulse );
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
