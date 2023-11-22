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
}