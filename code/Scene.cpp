//
//  Scene.cpp
//
#include "Scene.h"
#include "Physics/Contact.h"
#include "Physics/Intersections.h"
#include "Physics/Broadphase.h"
#include "Physics/Shapes/ShapeAnimated.h"
#include "SceneUtil.h"
#include <algorithm>
#include "Config.h"

/*
========================================================================================================

Scene

========================================================================================================
*/

/*
====================================================
Scene::~Scene
====================================================
*/
Scene::~Scene() {
	DeInitAnimInstanceDemo();

	for ( int i = 0; i < m_bodies.size(); i++ ) {
		delete m_bodies[ i ].m_shape;
	}
	m_bodies.clear();

	// note - this does NOT call the destructor on these pointers!
	m_renderedBodies.clear();

	endDebugSession();
} 

/*
====================================================
Scene::Reset
====================================================
*/
void Scene::Reset() {
	DeInitAnimInstanceDemo();

	for ( int i = 0; i < m_bodies.size(); i++ ) {
		delete m_bodies[ i ].m_shape;
	}
	m_bodies.clear();

	// note - this does NOT call the destructor on these pointers!
	m_renderedBodies.clear();

	Initialize();
}

/*
====================================================
Scene::Initialize
====================================================
*/
void Scene::Initialize() {
	///////////////////////////////////////////////////////////////////////
	// physics bodies
	///////////////////////////////////////////////////////////////////////

	Body body;

	// Dynamic bodies
	if ( RUN_PHYSICS_SIM ) {
		for ( int x = 0; x < 6; x++ ) {
			for ( int y = 0; y < 6; y++ ) {
				float radius = 0.5f;
				float xx = float( x - 1 ) * radius * 1.5f;
				float yy = float( y - 1 ) * radius * 1.5f;
				body.m_position = Vec3( xx, yy, 10.f );
				body.m_orientation = Quat( 0, 0, 0, 1 );
				body.m_linearVelocity.Zero();
				body.m_invMass = 1.f;
				body.m_elasticity = 0.5f;
				body.m_friction = 0.5f;
				body.m_shape = new ShapeSphere( radius );
				m_bodies.push_back( body );
			}
		}
		// Static floor
		for ( int x = 0; x < 3; x++ ) {
			for ( int y = 0; y < 3; y++ ) {
				float radius = 80.f;
				float xx = float( x - 1 ) * radius * 0.25f;
				float yy = float( y - 1 ) * radius * 0.25f;
				body.m_position = Vec3( xx, yy, -radius );
				body.m_orientation = Quat( 0, 0, 0, 1 );
				body.m_linearVelocity.Zero();
				body.m_invMass = 0.f;
				body.m_elasticity = 0.99f;
				body.m_friction = 0.5f;
				body.m_shape = new ShapeSphere( radius );
				m_bodies.push_back( body );
			}
		}
	}

	sceneUtil::MakeCoordGizmo( { 0, 0, 10 }, // origin
					{ 1, 0, 0 },  // fwd
					{ 0, 1, 0 },  // right
					{ 0, 0, 1 },  // up
					GIZMO_SCALE,
					&m_bodies 
	);

	if ( RUN_ANIMATION ) {		
		InitializeAnimInstanceDemo();
	}

	///////////////////////////////////////////////////////////////////////////////////
	// list of pointers to both ( todo - use placement new to allocate in same block )
	///////////////////////////////////////////////////////////////////////////////////
	int numAnimatedBodies = 0;
	for ( const AnimationInstance * animInst : animInstanceDemo ) {
		numAnimatedBodies += animInst ? animInst->bodiesToAnimate.size() : 0;
	}
	m_renderedBodies.resize( m_bodies.size() + numAnimatedBodies );

	// add all the physics bodies to the array of rendered bodies
	std::transform( m_bodies.begin(), m_bodies.end(), m_renderedBodies.begin(), []( Body & b ) { return &b; } );

	// add all the physics bodies to the array of rendered bodies ( always at the end )
	size_t offset = m_bodies.size();
	for ( AnimationInstance * animInst : animInstanceDemo ) {
			std::transform( 
				animInst->bodiesToAnimate.begin(), 
				animInst->bodiesToAnimate.end(), 
				m_renderedBodies.begin() + offset, 
				[]( Body & b )  { 
					return &b; 
				} );
			offset += animInst->bodiesToAnimate.size();
	}
}

void Scene::ToggleTPose() {
	for ( AnimationInstance * animInst : animInstanceDemo ) {
		if ( animInst != nullptr ) {
			animInst->CycleAnimMode();
		}
	}
}

void Scene::TryCycleAnim() {
	for ( AnimationInstance * animInst : animInstanceDemo ) {
		if ( animInst != nullptr ) {
			printf( "\nACTIVE ANIMATION WAS CHANGED TO: %s\n", animInst->CycleCurClip() );
		}
	}
}

int Scene::GetFirstAnimatedBodyIdx() {
	if ( !RUN_ANIMATION || animInstanceDemo[ 0 ]->bodiesToAnimate.empty() ) {
		return -1;
	}
	return m_bodies.size();
}

void Scene::InitializeAnimInstanceDemo() {
	if ( !RUN_ANIMATION ) {
		return;
	}

	for ( int i = 0; i < ANIM_DEMO_SKELETONS.size(); i++ ) {
		AnimationInstance *& animInst = animInstanceDemo[ i ];
		if ( animInst != nullptr ) {
			continue;
		}
		animInst = new AnimationInstance( { 0, 0, 0 }, ANIM_DEMO_SKELETONS[ i ] );
	}
}
void Scene::DeInitAnimInstanceDemo() {
	if ( !RUN_ANIMATION ) {
		return;
	}

	for ( int i = ANIM_DEMO_SKELETONS.size() - 1; i != 0; i-- ) {
		AnimationInstance *& animInst = animInstanceDemo[ i ];
		if ( animInst == nullptr ) {
			continue;
		}
		delete( animInst );
		animInst = nullptr;
	}
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
	// anim demo
	if ( RUN_ANIMATION ) {
		for ( AnimationInstance * animInst : animInstanceDemo ) {
			animInst->Update( dt_sec );
		}
	}

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

	// Broadphase ( identify potential pairs )
	std::vector< collisionPair_t > collisionPairs;
	BroadPhase( m_bodies.data(), static_cast< int >( m_bodies.size() ), collisionPairs, dt_sec );


	// Narrow Phase ( actual collision detection )
	int numContacts = 0;
	const int maxContacts = m_bodies.size() * m_bodies.size();
	contact_t * contacts = reinterpret_cast< contact_t * >( alloca( sizeof( contact_t ) * maxContacts ) );
	for ( int i = 0; i < collisionPairs.size(); i++ ) {
		const collisionPair_t & pair = collisionPairs[ i ];
		Body * bodyA = &m_bodies[ pair.a ];
		Body * bodyB = &m_bodies[ pair.b ];

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

	// Sort time of impact from earliest to latest
	if ( numContacts > 1 ) {
		qsort( contacts, numContacts, sizeof( contact_t ), CompareContacts );
	}

	// resolve all the bodies that have contacted first
	float accumulatedTime = 0.f;
	for ( int i = 0; i < numContacts; i++ ) {
		contact_t & contact = contacts[ i ];
		const float dt = contact.timeOfImpact - accumulatedTime;

		// update pos
		for ( int j = 0; j < m_bodies.size(); j++ ) {
			m_bodies[ j ].Update( dt );
		}

		ResolveContact( contact );
		accumulatedTime += dt;
	}

	// update all the bodies for the frame time remaining after all contacts were resolved
	// NOTE - if there were additional ( secondary ) contacts during this period, we just
	// ignore them, bc that would be way too expensive
	const float timeRemaining = dt_sec - accumulatedTime;
	if ( timeRemaining <= 0.f ) {
		return;
	}
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