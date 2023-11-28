//
//  Contact.cpp
//
#include "Contact.h"

/*
====================================================
ResolveContact
====================================================
*/
void ResolveContact( const contact_t & contact ) {
	Body * const bodyA = contact.bodyA;
	Body * const bodyB = contact.bodyB;
	const Vec3 ptOnA   = contact.ptOnA_WorldSpace;
	const Vec3 ptOnB   = contact.ptOnB_WorldSpace;
	const float sysElasticity = bodyA->m_elasticity * bodyB->m_elasticity;
	const float invMassA = contact.bodyA->m_invMass;
	const float invMassB = contact.bodyB->m_invMass;
	const Mat3 invWorldInertiaA = bodyA->GetInverseInertiaTensorWorldSpace();
	const Mat3 invWorldInertiaB = bodyB->GetInverseInertiaTensorWorldSpace();
	const Vec3 normal  = contact.normal;
	const Vec3 radiusA = ptOnA - bodyA->GetCenterOfMassWorldSpace();
	const Vec3 radiusB = ptOnB - bodyB->GetCenterOfMassWorldSpace();

	// get a vector orthogonal to axis of rotation and radius, whose magnitude is the angular impulse 
	// ( basically, the second basis vector of the 2d subspace, or "disc" of rotation )
	const Vec3 angImpulseA	  = ( invWorldInertiaA * radiusA.Cross( normal ) ).Cross( radiusA );
	const Vec3 angImpulseB	  = ( invWorldInertiaB * radiusB.Cross( normal ) ).Cross( radiusB );
	// now how much of this angular impulse is actually acting in the direction of the impulse normal?
	const float angularFactor = ( angImpulseA + angImpulseB ).Dot( normal );

	// world space velocity including both motion and rotation
	const Vec3 velA = bodyA->m_linearVelocity + bodyA->m_angularVelocity.Cross( radiusA );
	const Vec3 velB = bodyB->m_linearVelocity + bodyB->m_angularVelocity.Cross( radiusB );

	// Calculate and apply collision impulse
	const Vec3 velARelToB  = velA - velB;
	const float impulseMag = ( 1.f + sysElasticity ) * velARelToB.Dot( normal ) / ( invMassA + invMassB + angularFactor );
	const Vec3 impulseVec  = normal * impulseMag;
	bodyA->ApplyImpulse( ptOnA, impulseVec * -1.f );
	bodyB->ApplyImpulse( ptOnB, impulseVec *  1.f );

	// Calculate impulse caused by friction
	const float frictionA = bodyA->m_friction;	   // 0 ~ 1
	const float frictionB = bodyB->m_friction;	   // 0 ~ 1
	const float frictionT = frictionA * frictionB; // 0 ~ 1

	// Calculate velocity components normal and tangential ( perpendicular ) to collision
	const Vec3 velNormToCollision = normal * velARelToB.Dot( normal );
	const Vec3 velTangToCollision = velARelToB - velNormToCollision;
	
	// Tangential velocity direction
	Vec3 velTangDir = velTangToCollision;
	velTangDir.Normalize();

	const Vec3 inertiaA    = ( invWorldInertiaA * radiusA.Cross( velTangDir ).Cross( radiusA ) );
	const Vec3 inertiaB    = ( invWorldInertiaB * radiusB.Cross( velTangDir ).Cross( radiusB ) );
	const float invInertia = ( inertiaA + inertiaB ).Dot( velTangDir );

	// Calculate tangential impulse for friction
	const float reducedMass    = 1.f / ( invMassA + invMassB + invInertia );
	const Vec3 impulseFriction = velTangDir * reducedMass * frictionT;

	bodyA->ApplyImpulse( ptOnA, impulseFriction * -1.f );
	bodyB->ApplyImpulse( ptOnB, impulseFriction *  1.f );

	// only do the fake separation of colliding bodies w valid time of impact
	if ( 0.f == contact.timeOfImpact ) {
		// separate the colliding bodies such that their shared center of mass doesnt change
		// ( collective c. of mass is like a barycentric avg of their positions, weighted by their respective masses )
		const Vec3 separationDist = ptOnB - ptOnA;
		const float adjustPercA   = invMassA / ( invMassA + invMassB );
		const float adjustPercB   = invMassB / ( invMassA + invMassB );
		bodyA->m_position += separationDist * adjustPercA;
		bodyB->m_position -= separationDist * adjustPercB;
	}
}