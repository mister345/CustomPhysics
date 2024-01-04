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


/*
====================================================
Scene
====================================================
*/
class Scene {
public:
	Scene() { m_bodies.reserve( 128 ); }
	~Scene();

	void Reset();
	void Initialize();
	void InitializeAnimInstanceDemo();
	void InitializeAnimatedBodies();
	void Update( const float dt_sec );
	void UpdateWithoutTOI( const float dt_sec );

	void TryCycleAnim();

	std::vector< Constraint * >	m_constraints;
	ManifoldCollector m_manifolds;
	std::vector< Body * > m_renderedBodies;

private:
	std::vector< Body > m_bodies;
	std::vector< Body > m_animatedBodies;
	AnimationInstance * animInstanceDemo = nullptr;
	FbxManager * fbxManager = nullptr;
	FbxImporter * fbxImporter = nullptr;
	bool bAnimDataInitialized = false;
};

