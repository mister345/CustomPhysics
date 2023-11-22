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
	body.m_position = Vec3( 0, 0, 30 );
	body.m_orientation = Quat( 0, 0, 0, 1 );
	body.m_invMass = 1 / 1.f;
	body.m_shape = new ShapeSphere( 1.0f );
	m_bodies.push_back( body );

	body.m_position = Vec3( 0, 30, 30 );
	body.m_orientation = Quat( 0, 0, 0, 1 );
	body.m_invMass = 1 / 1000.f;
	body.m_shape = new ShapeSphere( 1.0f );
	m_bodies.push_back( body );

	// "ground" sphere unaffected by gravity
	body.m_position = Vec3( 0, 0, -101 );
	body.m_orientation = Quat( 0, 0, 0, 1 );
	body.m_invMass = 0.f;
	body.m_shape = new ShapeSphere( 100.0f );
	m_bodies.push_back( body );
}

/*
====================================================
Scene::Update
====================================================
*/
void Scene::Update( const float dt_sec ) {
	// integrate delta time so acceleration is introduced correctly
	const float fullDuration = dt_sec;
	const float halfDuration = dt_sec * 0.5f;

	// apply gravitational acceleration to velocity
	for ( Body & body : m_bodies ) {
		// Apply gravity as an impulse
		// Impulse = total delta momentum
		// Force   = delta momentum amortized over time
		//		  => delta momentum = Force * delta time = ( mass * accel ) * delta time
		//	      => Force = mass * gravitational accel * delta time 

		// retrieve actual mass from inverse so we can use it
		const float mass	= 1.f / body.m_invMass;
		Vec3 gravityImpulse = GRAV_ACCEL * mass * halfDuration;
		body.ApplyImpulseLinear( gravityImpulse );
	}

	// apply displacement based on position 
	for ( Body & body : m_bodies ) {
		body.m_position += body.m_linearVelocity * fullDuration;
	}

	// apply remainder of acceleration to bodies
	for ( Body & body : m_bodies ) {
		// same as above
		const float mass = 1.f / body.m_invMass;
		Vec3 gravityImpulse = GRAV_ACCEL * mass * halfDuration;
		body.ApplyImpulseLinear( gravityImpulse );
	}

	// Check collisions O( N^2 ) for now
	for ( Body & bodyA : m_bodies ) {
		for ( Body & bodyB : m_bodies ) {
			// skip self
			if ( &bodyA == &bodyB ) {
				continue;
			}

			// skip pairs with infinite mass
			if ( bodyA.m_invMass == 0.f && bodyB.m_invMass == 0.f ) {
				continue;
			}

			if ( Intersect( &bodyA, &bodyB ) ) {
				bodyA.m_linearVelocity.Zero();
				bodyB.m_linearVelocity.Zero();
			}
		}
	}
}