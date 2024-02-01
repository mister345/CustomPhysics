#pragma once

#include <stdio.h>
#include "Animation/AnimationAssets.h"
#include <array>

//////////////////////////////
/////////// CONFIG ///////////
//////////////////////////////

//#define ANIM_DEBUG_LOGGING
#ifdef ANIM_DEBUG_LOGGING
constexpr int DEBUG_FILE_COUNT = 4;
static std::array< const char *, DEBUG_FILE_COUNT > debugNames = {
	"bones",
	"clusters",
	"bindPosesBefore",
	"bindPosesAfter",
};
#endif // ANIM_DEBUG_LOGGING

void startDebugSession();
void endDebugSession();
void openDebugLog( int whichFile );
void closeDebugLog( int whichFile );
void writeToDebugLog( int whichFile, const char * s, ... );

static constexpr bool RUN_ANIMATION    = true;
static constexpr bool RUN_PHYSICS_SIM  = false;
static constexpr float ZOOM_MULTIPLIER = 35.f;
static constexpr float GIZMO_SCALE	   = 20.f;

static constexpr bool TEST_WITHOUT_TOI = false;
static constexpr bool PRINT_FRAME_TIME = false;

// Animation
static constexpr bool SHOW_ORIGIN = false;

//static constexpr unsigned NUM_THREADS_LOAD = 1;
static constexpr unsigned NUM_THREADS_LOAD = 16;

// skeleton type
static constexpr std::array< AnimationAssets::eSkeleton, 1 > ANIM_DEMO_SKELETONS = {
//	AnimationAssets::DEBUG_SKELETON,
	AnimationAssets::SKINNED_MESH 
};
//static constexpr AnimationAssets::eSkeleton WHICH_SKELETON = AnimationAssets::SKINNED_MESH;

// assets
//static constexpr float ANIMDEMO_SCALE = 0.0105f;
static constexpr float ANIMDEMO_SCALE = 1.f;
static constexpr float DEBUG_BONE_RAD = 7.5f * ANIMDEMO_SCALE;
//static constexpr const char * ANIMDEMO_FILENAME = "assets/humanoid.fbx";
// constexpr const char * ANIMDEMO_FILENAME = "assets/human_idle.fbx";
//constexpr const char * ANIMDEMO_FILENAME = "assets/bird.fbx";
constexpr const char * ANIMDEMO_FILENAME = "assets/femaleDance.fbx";
 
//static constexpr bool CONVERT_SCENE = false;
static constexpr bool CONVERT_SCENE = true;

// Physics
#include "Math/Vector.h"
static constexpr float GRAVITY_MAGNITUDE = 10.f;
static const Vec3 GRAV_ACCEL = { 0.f, 0.f, -GRAVITY_MAGNITUDE };

// Rendering
static constexpr float FAR_CLIPPING_PLANE_CAM	 = 30000.f;
static constexpr float FAR_CLIPPING_PLANE_SHADOW = 175.f;