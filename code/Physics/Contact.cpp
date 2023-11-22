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
	const Vec3 velocityARelativeToB		 = contact.bodyA->m_linearVelocity - contact.bodyB->m_linearVelocity;
	const float velCompNormalToCollision = velocityARelativeToB.Dot( contact.normal );
	const float impulseMag				 = -2.f * velCompNormalToCollision / ( contact.bodyA->m_invMass + contact.bodyB->m_invMass );
	const Vec3  impulseVec				 = contact.normal * impulseMag;
	contact.bodyA->ApplyImpulseLinear( impulseVec *  1.f );
	contact.bodyB->ApplyImpulseLinear( impulseVec * -1.f );

	// separate the colliding bodies such that their shared center of mass doesnt change
	// ( collective c. of mass is like a barycentric avg of their positions, weighted by their respective masses )
	const float massA		= 1.f / contact.bodyA->m_invMass;
	const float massB		= 1.f / contact.bodyB->m_invMass;
	const Vec3 centerOfMass = ( contact.bodyA->GetCenterOfMassModelSpace() * massA +
								contact.bodyB->GetCenterOfMassModelSpace() * massB )
							  / massA + massB;

	const float totalInvMass  =  contact.bodyA->m_invMass + contact.bodyB->m_invMass;
	const float adjustPercA   =  contact.bodyA->m_invMass / totalInvMass;
	const float adjustPercB   =  contact.bodyB->m_invMass / totalInvMass;
	const Vec3 separationDist =  contact.ptOnB_WorldSpace - contact.ptOnA_WorldSpace;
	contact.bodyA->m_position += separationDist * adjustPercA;
	contact.bodyB->m_position += separationDist * adjustPercB;
}