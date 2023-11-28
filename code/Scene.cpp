//
//  Scene.cpp
//
#include "Scene.h"
#include "Physics/Contact.h"
#include "Physics/Intersections.h"
#include "Physics/Broadphase.h"

/*
========================================================================================================

Scene

========================================================================================================
*/

// CONFIG
static constexpr float GRAVITY_MAGNITUDE = 10.f;
static const Vec3 GRAV_ACCEL = { 0.f, 0.f, -GRAVITY_MAGNITUDE };

/*
====================================================
Scene::~Scene
====================================================
*/
Scene::~Scene() {
	for ( int i = 0; i < m_bodies.size(); i++ ) {
		delete m_bodies[ i ].m_shape;
	}
	m_bodies.clear();
}

/*
====================================================
Scene::Reset
====================================================
*/
void Scene::Reset() {
	for ( int i = 0; i < m_bodies.size(); i++ ) {
		delete m_bodies[ i ].m_shape;
	}
	m_bodies.clear();

	Initialize();
}

/*
====================================================
Scene::Initialize
====================================================
*/
void Scene::Initialize() {
	Body body;

	body.m_position = Vec3( -3, 0, 3 );
	body.m_orientation = Quat( 0, 0, 0, 1 );
	body.m_linearVelocity = Vec3( 1000, 0, 0 );
	body.m_invMass = 1 / 1.f;
	body.m_elasticity = 0.f;
	body.m_friction = 0.5f;
	body.m_shape = new ShapeSphere( 0.5f );
	m_bodies.push_back( body );

	body.m_position = Vec3( 0, 0, 3 );
	body.m_orientation = Quat( 0, 0, 0, 1 );
	body.m_linearVelocity = Vec3( 0, 0, 0 );
	body.m_invMass = 0.f;
	body.m_elasticity = 0.f;
	body.m_friction = 0.5f;
	body.m_shape = new ShapeSphere( 0.5f );
	m_bodies.push_back( body );

	// "ground" sphere unaffected by gravity
	body.m_position = Vec3( 0, 0, -1000 );
	body.m_orientation = Quat( 0, 0, 0, 1 );
	body.m_linearVelocity = Vec3( 0, 0, 0 );
	body.m_invMass = 0.f;
	body.m_elasticity = 1.f;
	body.m_friction = 0.f;
	body.m_shape = new ShapeSphere( 1000.0f );
	m_bodies.push_back( body );
}

int CompareContacts( const void * p1, const void * p2 ) {
	const contact_t & a = *( contact_t * )p1;
	const contact_t & b = *( contact_t * )p2;

	if ( a.timeOfImpact < b.timeOfImpact ) {
		return -1;
	}
	if ( a.timeOfImpact == b.timeOfImpact ) {
		return 0;
	}
	return 1;
}

/*
====================================================
Scene::Update
====================================================
*/
void Scene::Update( const float dt_sec ) {
	// apply gravitational acceleration to velocity
	for ( int i = 0; i < m_bodies.size(); i++ ) {
		Body * body = &m_bodies[ i ];
		// Apply gravity as an impulse
		// Impulse = total delta momentum
		// Force   = delta momentum amortized over time
		//		  => delta momentum = Force * delta time = ( mass * accel ) * delta time
		//	      => Force = mass * gravitational accel * delta time 

		// retrieve actual mass from inverse so we can use it
		const float mass	= 1.f / body->m_invMass;
		Vec3 gravityImpulse = GRAV_ACCEL * mass * dt_sec;
		body->ApplyImpulseLinear( gravityImpulse );
	}

	int numContacts = 0;
	const int maxContacts = m_bodies.size() * m_bodies.size();
	contact_t * contacts = reinterpret_cast< contact_t * >( alloca( sizeof( contact_t ) * maxContacts ) );
	for ( int i = 0; i < m_bodies.size(); i++ ) {
		for ( int j = i + 1; j < m_bodies.size(); j++ ) {
			Body * bodyA = &m_bodies[ i ];
			Body * bodyB = &m_bodies[ j ];

			// skip pairs w infinite mass
			if ( 0.f == bodyA->m_invMass && 0.f == bodyB->m_invMass ) {
				continue;
			}

			contact_t contact{};
			if ( Intersect( bodyA, bodyB, dt_sec, contact ) ) {
				contacts[ numContacts ] = contact;
				numContacts++;
			}
		}
	}

	// Sort time of impact from earliest to latest
	if ( numContacts > 1 ) {
		qsort( contacts, numContacts, sizeof( contact_t ), CompareContacts );
	}

	// resolve all the bodies that have contacted first
	float accumulatedTime = 0.f;
	for ( int i = 0; i < numContacts; i++ ) {
		contact_t & contact = contacts[ i ];
		const float dt = contact.timeOfImpact - accumulatedTime;

		Body * bodyA = contact.bodyA;
		Body * bodyB = contact.bodyB;

		// skip pairs w infinite mass
		if ( 0.f == bodyA->m_invMass && 0.f == bodyB->m_invMass ) {
			continue;
		}

		// update pos
		for ( int j = 0; j < m_bodies.size(); j++ ) {
			m_bodies[ j ].Update( dt );
		}

		ResolveContact( contact );
		accumulatedTime += dt;
	}

	const float timeRemaining = dt_sec - accumulatedTime;
	if ( timeRemaining <= 0.f ) {
		return;
	}

	// update all the bodies for the frame time remaining after all contacts were resolved
	// NOTE - if there were additional ( secondary ) contacts during this period, we just
	// ignore them, bc that would be way too expensive
	for ( int i = 0; i < m_bodies.size(); i++ ) {
		m_bodies[ i ].Update( timeRemaining );
	}
}

void Scene::UpdateWithoutTOI( const float dt_sec ) {
	// apply gravitational acceleration to velocity
	for ( int i = 0; i < m_bodies.size(); i++ ) {
		Body * body = &m_bodies[ i ];
		// Apply gravity as an impulse
		// Impulse = total delta momentum
		// Force   = delta momentum amortized over time
		//		  => delta momentum = Force * delta time = ( mass * accel ) * delta time
		//	      => Force = mass * gravitational accel * delta time 

		// retrieve actual mass from inverse so we can use it
		const float mass = 1.f / body->m_invMass;
		Vec3 gravityImpulse = GRAV_ACCEL * mass * dt_sec;
		body->ApplyImpulseLinear( gravityImpulse );
	}

	// Check collisions O( N^2 ) for now
	for ( int i = 0; i < m_bodies.size(); i++ ) {
		for ( int j = i + 1; j < m_bodies.size(); j++ ) {
			Body * bodyA = &m_bodies[ i ];
			Body * bodyB = &m_bodies[ j ];

			// skip pairs with infinite mass
			if ( bodyA->m_invMass == 0.f && bodyB->m_invMass == 0.f ) {
				continue;
			}

			contact_t contact;
			if ( Intersect( bodyA, bodyB, contact ) ) {
				ResolveContact( contact );
			}
		}
	}

	// apply displacement based on position 
	for ( int i = 0; i < m_bodies.size(); i++ ) {
		m_bodies[ i ].Update( dt_sec );
	}
}