//
//  Contact.cpp
//
#include "Contact.h"

/*
====================================================
ResolveContact
====================================================
*/
void ResolveContact( contact_t & contact ) {
	contact.bodyA->m_linearVelocity.Zero();
	contact.bodyB->m_linearVelocity.Zero();

	// separate the colliding bodies such that their shared center of mass doesnt change
	// collective center of mass is basically a barycentric avg of their positions,
	// each weighted by their respective weights
	const float massA		= 1.f / contact.bodyA->m_invMass;
	const float massB		= 1.f / contact.bodyB->m_invMass;
	const Vec3 centerOfMass = ( contact.bodyA->GetCenterOfMassModelSpace() * massA +
								contact.bodyB->GetCenterOfMassModelSpace() * massB )
							  / massA + massB;

	const Vec3 separationDist =  contact.ptOnB_WorldSpace - contact.ptOnA_WorldSpace;
	const float totalInvMass  =  contact.bodyA->m_invMass + contact.bodyB->m_invMass;
	const float adjustPercA   =  contact.bodyA->m_invMass / totalInvMass;
	const float adjustPercB   =  contact.bodyB->m_invMass / totalInvMass;
	contact.bodyA->m_position += separationDist * adjustPercA;
	contact.bodyB->m_position += separationDist * adjustPercB;
}