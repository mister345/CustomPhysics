#include "inttypes.h"
#include "AnimationData.h"
#include "AnimationState.h"
#include "../Physics/Shapes/ShapeAnimated.h"
#include "../Physics/Shapes/ShapeLoadedMesh.h"
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
	std::vector< FbxAMatrix > fbxInitialTransforms( animData->BoneCount() );

	if ( startInTPose || animData->animations.empty() ) {
		initialTransforms.assign( animData->BindPoseMatrices.begin(), animData->BindPoseMatrices.end() );
//		fbxInitialTransforms.assign( animData->FbxInvBindPoseMatrices.begin(), animData->FbxInvBindPoseMatrices.end() );
	} else {
		animData->GetFinalTransformsLocal( curClipName, 0, initialTransforms );
	}

	if ( whichSkeleton == AnimationAssets::eSkeleton::SKINNED_MESH ) {
		ShapeLoadedMesh * mesh = reinterpret_cast< ShapeLoadedMesh * >( bodiesToAnimate[ 0 ].m_shape );
		if ( animMode == eAnimMode::GLOBAL ) {
			mesh->PopulateMatrixPalette( fbxInitialTransforms );
		} else if ( animMode == eAnimMode::LOCAL ) {
			mesh->PopulateMatrixPalette( initialTransforms );
		} else { // TPose
			mesh->PopulateMatrixPalette( fbxInitialTransforms );			
		}
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
	std::vector< BoneTransform > boneTransforms( animData->BoneCount() ); // initialized to identity
	std::vector< FbxAMatrix > fbxBoneTransforms( animData->BoneCount() ); // initialized to identity

	switch ( animMode ) {
		case LOCAL: {
			animData->GetFinalTransformsLocal( curClipName, animTimePos, boneTransforms );
			break;
		}
		case GLOBAL: {
			animData->GetFinalTransformsGlobal( curClipName, animTimePos, fbxBoneTransforms );
			break;
		}
		case TPOSE:
		default: {
			boneTransforms.assign( animData->BindPoseMatrices.begin(), animData->BindPoseMatrices.end() );

			// experiment - completely broken
			// in the case of vert deformation, the deformation matrix for the "tpose" would technically just be
			// an identity matrix, aka doing NOTHING to the verts. but let's see if we can reconstruct it instead
			//for ( int i = 0; i < fbxBoneTransforms.size(); i++ ) {
			//	BoneTransform & tPose = animData->BindPoseMatrices[ i ];

			//	fbxsdk::FbxAMatrix fbxTPose;
			//	fbxsdk::FbxQuaternion q( tPose.rotation.x, tPose.rotation.y, tPose.rotation.z, tPose.rotation.w );
			//	fbxTPose.SetQ( q );
			//	FbxVector4 t( tPose.translation.x, tPose.translation.y, tPose.translation.z, 1 );
			//	fbxTPose.SetT( t );
			//	fbxTPose.SetS( { 0.01, 0.01, 0.01, 1 } );

			//	fbxBoneTransforms[ i ] = animData->FbxInvBindPoseMatrices[ i ] * fbxTPose;
			//}
		}
	}

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

				BoneTransform transform;
				if ( animMode == eAnimMode::GLOBAL ) {
					transform = { &fbxBoneTransforms[ i ].GetQ(), &fbxBoneTransforms[ i ].GetT() };
				} else if ( animMode == eAnimMode::LOCAL ) {
					transform = boneTransforms[ i ];
				} else { // TPOSE
					transform = boneTransforms[ i ];
				}

				bodyToAnimate.m_orientation		= transform.rotation;
				bodyToAnimate.m_position		= worldPos + transform.translation;
			}
			break;
		}
		case AnimationAssets::SKINNED_MESH: {
			// handle the case of only ONE BODY, with ONE SHAPE,
			Body & bodyToAnimate   = bodiesToAnimate[ 0 ];
			ShapeLoadedMesh * mesh = reinterpret_cast< ShapeLoadedMesh * >( bodyToAnimate.m_shape );
			if ( animMode == eAnimMode::GLOBAL ) {
				mesh->PopulateMatrixPalette( fbxBoneTransforms );
			} else if ( animMode == eAnimMode::LOCAL ) {
				mesh->PopulateMatrixPalette( boneTransforms );
			} else { // TPose
				mesh->PopulateMatrixPalette( fbxBoneTransforms );
			}
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

	const char * newAnim = pCurAnim->first.c_str();
	printf( "\n# # # Active animation was changed to: %s # # #\n", newAnim );
	return newAnim;
}

void AnimationInstance::CycleAnimMode() {
	animMode = static_cast< eAnimMode >( ( animMode + 1 ) % MODE_COUNT );
	printf( "\n# # # # # # Animation mode was changed to: %s# # # # # #\n", AnimModeToStr( animMode ) );
}

const char * AnimModeToStr( eAnimMode m ) {
	switch ( m ) {
		case TPOSE: {
			return "TPOSE";
		}
		case LOCAL: {
			return "LOCAL";
		}
		case GLOBAL: {
			return "GLOBAL";
		}
		case MODE_COUNT: {
			return "INVALID";
		}
	}
	return "INVALID";
}
