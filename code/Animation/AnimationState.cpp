#include "inttypes.h"
#include "AnimationData.h"
#include "AnimationState.h"
#include "../Physics/Shapes/ShapeAnimated.h"
#include "../Config.h"

////////////////////////////////////////////////////////////////////////////////
// ANIMATION INSTANCE
////////////////////////////////////////////////////////////////////////////////
AnimationInstance::AnimationInstance( const Vec3 & worldPos_, AnimationAssets::eSkeleton whichSkeleton ) {
	worldPos = worldPos_;

	// spawn a single debug sphere to indicate the origin pos of the animated object
	if ( SHOW_ORIGIN ) {
		bodiesToAnimate.push_back( Body() );
		bodiesToAnimate.back().m_position = worldPos;
		bodiesToAnimate.back().m_orientation = { 0, 0, 0, 1 };
		bodiesToAnimate.back().m_linearVelocity.Zero();
		bodiesToAnimate.back().m_invMass = 0.f;	// no grav
		bodiesToAnimate.back().m_elasticity = 1.f;
		bodiesToAnimate.back().m_friction = 0.f;
		bodiesToAnimate.back().m_shape = new ShapeAnimated( 0.45f, true );
	}

	// Load the animation data ( bones and verts ) from fbx file
	animData = new SkinnedData( whichSkeleton );
	AnimationAssets::FillAnimInstanceData( this, ANIMDEMO_FILENAME, ANIMDEMO_SCALE );
	if ( animData->BoneCount() <= 0 ) {
		printf( "~WARNING~\tSkeleton contains 0 bones, AnimationIstance will NOT be initialized!\n" );
		return;
	}
	// assign current animation to the first one so it can be cycled
	pCurAnim = animData->animations.begin();
	const char * curClipName = GetCurClipName();

	// Create corresponding bodies to debug bones, etc, in the game world, based on loaded skeleton
	Shape * shapeToAnimate = nullptr;
	switch ( whichSkeleton ) {
		case AnimationAssets::SINGLE_BONE:
		case AnimationAssets::MULTI_BONE:
		case AnimationAssets::DEBUG_SKELETON: {
			const int numBodies = animData->BoneCount();
			for ( int i = 0; i < numBodies; i++ ) {
				bodiesToAnimate.push_back( Body() );
				bodiesToAnimate.back().m_shape = new ShapeAnimated( DEBUG_BONE_RAD, false );;
			}
			break;
		}
		case AnimationAssets::SKINNED_MESH: {
			bodiesToAnimate.push_back( Body() );
			bodiesToAnimate.back().isSkinnedMesh = true;
			bodiesToAnimate.back().m_shape = new ShapeLoadedMesh( 
				animData->renderedVerts,
				animData->numVerts,
				animData->idxes,
				animData->numIdxes,
				animData->BoneCount()
			);
			break;
		}
	}
	if ( bodiesToAnimate.empty() ) {
		printf( "~WARNING~\tNo bodies to animate, AnimationIstance will NOT be initialized!\n" );
		return;
	}

	// setup the initial bone transforms
	std::vector< BoneTransform > initialTransforms;
	if ( startInTPose || animData->animations.empty() ) {
		initialTransforms.assign( animData->BindPoseMatrices.begin(), animData->BindPoseMatrices.end() );
	} else {
		animData->GetFinalTransforms( curClipName, 0, initialTransforms );
	}

	if ( whichSkeleton == AnimationAssets::eSkeleton::SKINNED_MESH ) {
		ShapeLoadedMesh * mesh = reinterpret_cast< ShapeLoadedMesh * >( bodiesToAnimate[ 0 ].m_shape );
		mesh->PopulateMatrixPalette( &initialTransforms );
	} else {
		// move bodies into position, assign appropriate shapes to them ( could be a skinned mesh! )
		for ( int i = 0; i < bodiesToAnimate.size(); i++ ) {
			Body & bodyToAnimate = bodiesToAnimate[ i ];
			bodyToAnimate.m_position = worldPos + initialTransforms[ i ].translation;
			bodyToAnimate.m_orientation = initialTransforms[ i ].rotation; // @TODO - add world rotation member
			bodyToAnimate.m_linearVelocity.Zero();
			bodyToAnimate.m_invMass = 0.f;	// no grav
			bodyToAnimate.m_elasticity = 1.f;
			bodyToAnimate.m_friction = 0.f;
		}
	}
}

AnimationInstance::~AnimationInstance() {
	if ( bodiesToAnimate.size() > 0 ) {
		if ( isInstanced ) {
			if ( bodiesToAnimate[ 0 ].m_shape != nullptr ) {
				delete bodiesToAnimate[ 0 ].m_shape;	
				bodiesToAnimate[ 0 ].m_shape = nullptr;
			}
		} else {
			for ( int i = 0; i < bodiesToAnimate.size(); i++ ) {
				if ( bodiesToAnimate[ i ].m_shape != nullptr ) {
					delete( bodiesToAnimate[ i ].m_shape );
					bodiesToAnimate[ i ].m_shape = nullptr;
				}
			}
		}
		bodiesToAnimate.clear();
	}

	delete animData;
	animData = nullptr;
}

void AnimationInstance::Update( float deltaT ) {
	if ( animData->animations.empty() || bodiesToAnimate.empty() ) {
		// no anim data, so dont update the pose ( just stay in t-pose )
		return;
	}

	// find our time bounds for looping
	const char * curClipName    = GetCurClipName();
	const AnimationClip curClip = animData->animations.at( curClipName );
	const float loopBoundary	= curClip.GetClipEndTime();

	// increment time and wrap around
	animTimePos += deltaT * speedMultiplier;
	if ( animTimePos >= loopBoundary ) {
		animTimePos = 0.f;
	}

	// ask AnimData to interpolate each bone transform across its keyframes @ given time point
	// also concatenates the bones down the skeletal hierarchy to produce component space transforms
	std::vector< BoneTransform > boneTransforms( animData->BoneCount() );
	animData->GetFinalTransforms( curClipName, animTimePos, boneTransforms );

	// Apply transforms to all bodies rendered in the scene
	if ( bodiesToAnimate.empty() ) {
		return;
	}
	switch ( animData->skeletonType ) {
		case AnimationAssets::SINGLE_BONE:
		case AnimationAssets::MULTI_BONE:
		case AnimationAssets::DEBUG_SKELETON: {
			// apply each bone transform to each body ( 1 to 1 )
			for ( int i = 0; i < animData->BoneCount(); i++ ) {
				Body & bodyToAnimate			= bodiesToAnimate[ i ];
				const BoneTransform & transform = boneTransforms[ i ];
				bodyToAnimate.m_orientation = transform.rotation;
				bodyToAnimate.m_position = worldPos + transform.translation;
			}
			break;
		}
		case AnimationAssets::SKINNED_MESH: {
			// handle the case of only ONE BODY, with ONE SHAPE,
			Body & bodyToAnimate = bodiesToAnimate[ 0 ];
			ShapeLoadedMesh * mesh = reinterpret_cast< ShapeLoadedMesh * >( bodyToAnimate.m_shape );
			// convert all these BoneTransforms into matrices,
			// the rest is up to the GPU skinning stage in teh vertex shader
			mesh->PopulateMatrixPalette( &boneTransforms );
			break;
		}
	}
}

const char * AnimationInstance::GetCurClipName() {
	if ( animData->animations.empty() ) {
		return "NONE";
	}
	return pCurAnim->first.c_str();
}

const char * AnimationInstance::CycleCurClip() {
	if ( animData->animations.empty() ) {
		return nullptr;
	}
	if ( ++pCurAnim == animData->animations.end() ) {
		pCurAnim = animData->animations.begin();
	}
	return pCurAnim->first.c_str();
}
