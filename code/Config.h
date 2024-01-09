#pragma once

#include "Math/Vector.h"

//////////////////////////////
/////////// CONFIG ///////////
//////////////////////////////
static constexpr bool RUN_ANIMATION = true;
static constexpr bool RUN_PHYSICS_SIM = false;

// Animation
static constexpr bool SHOW_ORIGIN = false;
// type
//static constexpr AnimationAssets::eSkeleton WHICH_SKELETON = AnimationAssets::SINGLE_BONE;
//static constexpr AnimationAssets::eSkeleton WHICH_SKELETON = AnimationAssets::MULTI_BONE;
//static constexpr AnimationAssets::eSkeleton WHICH_SKELETON = AnimationAssets::DEBUG_SKELETON;
static constexpr AnimationAssets::eSkeleton WHICH_SKELETON = AnimationAssets::SKINNED_MESH;
// asset
static constexpr bool CONVERT_SCENE = false;
static constexpr float ANIMDEMO_SCALE = 0.0105f;
//static constexpr float ANIMDEMO_SCALE = 1.f;
//static constexpr float DEBUG_BONE_RAD = 7.5f;
static constexpr float DEBUG_BONE_RAD = 0.075f;

//constexpr float ANIMDEMO_SCALE = 1.f;
static constexpr const char * ANIMDEMO_FILENAME = "assets/humanoid.fbx";
// constexpr const char * ANIMDEMO_FILENAME = "assets/human.fbx";
// constexpr const char * ANIMDEMO_FILENAME = "assets/testSkeleton.fbx";
// constexpr const char * ANIMDEMO_FILENAME = "assets/human_idle.fbx";

// Physics
static constexpr float GRAVITY_MAGNITUDE = 10.f;
static const Vec3 GRAV_ACCEL = { 0.f, 0.f, -GRAVITY_MAGNITUDE };

// Rendering
static constexpr float FAR_CLIPPING_PLANE_CAM	 = 10000.f;
static constexpr float FAR_CLIPPING_PLANE_SHADOW = 175.f;