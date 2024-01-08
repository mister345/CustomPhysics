#include "inttypes.h"
#include "AnimationData.h"
#include "AnimationState.h"
#include "../Physics/Shapes/ShapeAnimated.h"
#include "../Config.h"

////////////////////////////////////////////////////////////////////////////////
// ANIMATION INSTANCE
////////////////////////////////////////////////////////////////////////////////
AnimationInstance::AnimationInstance() {
	animData = new SkinnedData();
}

AnimationInstance::~AnimationInstance() {
//	bodiesToAnimate = nullptr;
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

void AnimationInstance::Initialize( const Vec3 & startPos_WS ) {
	if ( animData->BoneCount() <= 0 ) {
		printf( "~WARNING~\tSkeleton contains 0 bones, AnimationIstance will NOT be initialized!\n" );
		return;
	}

	Shape * shapeToAnimate = nullptr;
	switch ( WHICH_SKELETON ) {
		case AnimationAssets::SINGLE_BONE:
		case AnimationAssets::MULTI_BONE:
		case AnimationAssets::DEBUG_SKELETON: {
			shapeToAnimate = new ShapeAnimated( DEBUG_BONE_RAD, false );
			const int numBodies = animData->BoneCount();
			for ( int i = 0; i < numBodies; i++ ) {
				bodiesToAnimate.push_back( Body() );
			}
			break;
		}
		case AnimationAssets::SKINNED_MESH: {
			shapeToAnimate = new ShapeLoadedMesh(
				animData->renderedVerts,
				animData->numVerts,
				animData->idxes,
				animData->numIdxes 
			);
			bodiesToAnimate.push_back( Body() );
			break;
		}
	}
	if ( bodiesToAnimate.empty() ) {
		printf( "~WARNING~\tNo bodies to animate, AnimationIstance will NOT be initialized!\n" );
		return;
	}

	curClipName	= animData->animations.empty() ? "NONE" : animData->animations.begin()->first.c_str();
	std::vector< BoneTransform > initialTransforms;
	if ( !animData->animations.empty() ) {
		animData->GetFinalTransforms( curClipName, 0, initialTransforms );
	} else { // if no anims, just T-pose
		initialTransforms.assign( animData->OffsetMatrices_DIRECT_DEBUG.begin(), animData->OffsetMatrices_DIRECT_DEBUG.end() );
	}
	// note - this will be meaningless for a skinned mesh
	for ( int i = 0; i < bodiesToAnimate.size(); i++ ) {
		Body & bodyToAnimate = bodiesToAnimate[ i ];
		bodyToAnimate.m_position = startPos_WS + initialTransforms[ i ].translation;
		bodyToAnimate.m_orientation = initialTransforms[ i ].rotation; // @TODO - add world rotation member
		bodyToAnimate.m_linearVelocity.Zero();
		bodyToAnimate.m_invMass = 0.f;	// no grav
		bodyToAnimate.m_elasticity = 1.f;
		bodyToAnimate.m_friction = 0.f;
		bodyToAnimate.m_shape = shapeToAnimate;
	}

	pCurAnim = animData->animations.begin();

	// spawn a single debug sphere to indicate the origin pos of the animated object
	// put it at the end so it gets rendered on top ( hopefully )
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
}

void AnimationInstance::Update( float deltaT ) {
	if ( animData->animations.empty() || bodiesToAnimate.empty() ) {
		// no anim data, so dont update the pose ( just stay in t-pose )
		return;
	}

	// find our time bounds for looping
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
			// @TODO - we will convert all these BoneTransforms into matrices,
			// the rest is up to the GPU skinning stage in teh vertex shader
			mesh->PopulateMatrixPalette( &boneTransforms );
			break;
		}
	}
}