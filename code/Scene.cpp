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
	body.m_position = Vec3( 0, 0, 20 );
	body.m_orientation = Quat( 0, 0, 0, 1 );
	body.m_invMass = 1 / 1.f;
	body.m_elasticity = 1.f;
	body.m_shape = new ShapeSphere( 1.0f );
	m_bodies.push_back( body );

	body.m_position = Vec3( 0, 15, 20 );
	body.m_orientation = Quat( 0, 0, 0, 1 );
	body.m_invMass = 1 / 1000.f;
	body.m_elasticity = 0.5f;
	body.m_shape = new ShapeSphere( 1.0f );
	m_bodies.push_back( body );

	// "ground" sphere unaffected by gravity
	body.m_position = Vec3( 0, 0, -101 );
	body.m_orientation = Quat( 0, 0, 0, 1 );
	body.m_invMass = 0.f;
	body.m_elasticity = 1.f;
	body.m_shape = new ShapeSphere( 100.0f );
	m_bodies.push_back( body );
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