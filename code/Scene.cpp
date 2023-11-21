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
static const Vec3 GRAVITATIONAL_ACCELERATION = { 0.f, 0.f, -GRAVITY_MAGNITUDE };

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
	body.m_shape = new ShapeSphere( 1.0f );
	m_bodies.push_back( body );

	body.m_position = Vec3( 0, 0, -101 );
	body.m_orientation = Quat( 0, 0, 0, 1 );
	body.m_shape = new ShapeSphere( 100.0f );
	m_bodies.push_back( body );

	// TODO: Add code
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
		body.m_linearVelocity += GRAVITATIONAL_ACCELERATION * halfDuration;
	}

	// apply displacement based on position 
	for ( Body & body : m_bodies ) {
		body.m_position += body.m_linearVelocity * fullDuration;
	}

	// apply gravitational acceleration to velocity
	for ( Body & body : m_bodies ) {
		body.m_linearVelocity += GRAVITATIONAL_ACCELERATION * halfDuration;
	}
}