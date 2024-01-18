//
//  Scene.h
//
#pragma once
#include <vector>

#include "Physics/Shapes.h"
#include "Physics/Body.h"
#include "Physics/Constraints.h"
#include "Physics/Manifold.h"
#include "Animation/AnimationData.h"
#include "Animation/AnimationState.h"
#include "Animation/ModelLoader.h"
#include "Config.h"

/*
====================================================
Scene
====================================================
*/
class Scene {
public:
	Scene() { m_bodies.reserve( 128 ); startDebugSession(); }
	~Scene();

	void Reset();
	void Initialize();
	void InitializeAnimInstanceDemo();
	void DeInitAnimInstanceDemo();
	void Update( const float dt_sec );
	void UpdateWithoutTOI( const float dt_sec );

	void TryCycleAnim();
	int GetFirstAnimatedBodyIdx();

	std::vector< Constraint * >	m_constraints;
	ManifoldCollector m_manifolds;
	std::vector< Body * > m_renderedBodies;
	std::array< AnimationInstance *, ANIM_DEMO_SKELETONS.size() > animInstanceDemo = {};

private:
	std::vector< Body > m_bodies;
};

