#include <vector>
#include "ModelLoader.h"
#include "../Math/Vector.h"
#include "../Math/Quat.h"
#include "AnimationData.h"
#include "AnimationState.h"
#include <algorithm>

#include "../../libs/FBX/2020.3.4/include/fbxsdk/scene/animation/fbxanimstack.h"
#include "../../libs/FBX/2020.3.4/include/fbxsdk/scene/animation/fbxanimcurve.h"
#include <cassert>

namespace AnimationAssets {
	constexpr float TO_RAD = 3.14159265359f / 180.f;
	const char * animNames[ eWhichAnim::COUNT + 1 ] = {
		"SingleBone",
		"MultiBone",
		"SkeletonOnly",
		"SkinnedMesh",
		"Unknown",
	};

	void FillAnimInstanceData( SkinnedData *& skinnedData, eWhichAnim whichAnim, fbxsdk::FbxScene * sceneData ) {
		switch ( whichAnim ) {
			case SINGLE_BONE: {
				int boneCount;
				std::vector< BoneInfo_t > hierarchy;
				std::vector< BoneTransform > boneOffsets;
				AnimationClip clip( 1 );

				hierarchy.assign( { -1 } );
				boneOffsets.assign( { BoneTransform::Identity() } );
				clip.BoneAnimations.assign( { MakeBoneAnim1(), } );

				std::map< std::string, AnimationClip > animMap = { { animNames[ whichAnim ], clip } };
				skinnedData->Set( hierarchy, boneOffsets, animMap );
				break;
			}
			case MULTI_BONE: {
				constexpr int boneCount = 9;

				std::vector< BoneInfo_t > hierarchy;
				std::vector< BoneTransform > boneOffsets;
				AnimationClip clip( boneCount );

				hierarchy.assign( { -1, 0, 1, 2, 3, 4, 5, 6, 7 } );
				boneOffsets.assign( { { { 0, 0, 0, 1 }, { 0, 0 * 2.f, 0 }, true, },
									  { { 0, 0, 0, 1 }, { 0, 2 * 2.f, 0 }, false, },
									  { { 0, 0, 0, 1 }, { 0, 4 * 2.f, 0 }, false, },
									  { { 0, 0, 0, 1 }, { 0, 6 * 2.f, 0 }, false, },
									  { { 0, 0, 0, 1 }, { 0, 8 * 2.f, 0 }, false, },
									  { { 0, 0, 0, 1 }, { 0, 10 * 2.f, 0 }, false, },
									  { { 0, 0, 0, 1 }, { 0, 12 * 2.f, 0 }, false, },
									  { { 0, 0, 0, 1 }, { 0, 14 * 2.f, 0 }, false, },
									  { { 0, 0, 0, 1 }, { 0, 16 * 2.f, 0 }, false, } } );
				assert( hierarchy.size() == boneCount && boneOffsets.size() == boneCount );

				BoneAnimation boneAnim = MakeBoneAnim1();
				clip.BoneAnimations.assign( { { { } },
											  boneAnim,
											  boneAnim,
											  boneAnim,
											  boneAnim,
											  boneAnim,
											  boneAnim,
											  boneAnim,
											  boneAnim } );
				assert( clip.BoneAnimations.size() == boneCount );

				std::map< std::string, AnimationClip > animMap = { { animNames[ whichAnim ], clip } };
				skinnedData->Set( hierarchy, boneOffsets, animMap );
				break;
			}
			case SKELETON_ONLY:
			case SKINNED_MESH:
			default: {
				skinnedData->Set( sceneData, whichAnim );
				break;
			}
		}
	}

	BoneAnimation MakeBoneAnim0() {
		BoneAnimation anim;
		anim.keyframes.resize( 5 );

		anim.keyframes[ 0 ].timePos = 0.0f;
		anim.keyframes[ 0 ].transform.translation = Vec3( -5.0f, 0.0f, 0.0f );
		anim.keyframes[ 0 ].transform.rotation = { 0, 0, 0, 1 };

		anim.keyframes[ 1 ].timePos = 1.0f;
		anim.keyframes[ 1 ].transform.translation = Vec3( 0.0f, 0.0f, 0.0f );
		anim.keyframes[ 1 ].transform.rotation = { 0, 0, 0, 1 };

		anim.keyframes[ 2 ].timePos = 1.0f;
		anim.keyframes[ 2 ].transform.translation = Vec3( 5.0f, 0.0f, 0.0f );
		anim.keyframes[ 2 ].transform.rotation = { 0, 0, 0, 1 };

		anim.keyframes[ 3 ].timePos = 3.0f;
		anim.keyframes[ 3 ].transform.translation = Vec3( 0.0f, 0.0f, 0.0f );
		anim.keyframes[ 3 ].transform.rotation = { 0, 0, 0, 1 };

		anim.keyframes[ 4 ].timePos = 4.0f;
		anim.keyframes[ 4 ].transform.translation = Vec3( -5.f, 0.0f, 0.0f );
		anim.keyframes[ 4 ].transform.rotation = { 0, 0, 0, 1 };

		return anim;
	}

	BoneAnimation MakeBoneAnim1() {
		BoneAnimation anim;
		anim.keyframes.resize( 5 );

		anim.keyframes[ 0 ].timePos = 0.0f;
		anim.keyframes[ 0 ].transform.translation = Vec3( -1.0f, 0.0f, 0.0f );
		anim.keyframes[ 0 ].transform.rotation = Quat( Vec3( 0, 1, 0 ), 30 * TO_RAD );

		anim.keyframes[ 1 ].timePos = 1.0f;
		anim.keyframes[ 1 ].transform.translation = Vec3( 0.0f, 2.0f, 3.0f );
		anim.keyframes[ 1 ].transform.rotation = Quat( Vec3( 1, 1, 2 ), 45 * TO_RAD );

		anim.keyframes[ 2 ].timePos = 1.0f;
		anim.keyframes[ 2 ].transform.translation = Vec3( 3.0f, 0.0f, 0.0f );
		anim.keyframes[ 2 ].transform.rotation = Quat( Vec3( 0, 1, 0 ), -30 * TO_RAD );

		anim.keyframes[ 3 ].timePos = 3.0f;
		anim.keyframes[ 3 ].transform.translation = Vec3( 0.0f, 1.0f, -3.0f );
		anim.keyframes[ 3 ].transform.rotation = Quat( Vec3( 0, 1, 0 ), 70 * TO_RAD );

		anim.keyframes[ 4 ].timePos = 4.0f;
		anim.keyframes[ 4 ].transform.translation = Vec3( -2.5f, 0.0f, 0.0f );
		anim.keyframes[ 4 ].transform.rotation = Quat( Vec3( 0, 1, 0 ), 30 * TO_RAD );

		return anim;
	}
} // namespace AnimationAssets

float BoneAnimation::GetStartTime() const {
	return keyframes.front().timePos;
}

float BoneAnimation::GetEndTime() const {
	return keyframes.back().timePos;
}

Quat Lerp( const Quat & from, const Quat & to, float t ) {
	Vec4 a( from.x, from.y, from.z, from.w );
	Vec4 b(   to.x,   to.y,   to.z,   to.w );

	const bool isLongWayAround = a.Dot( b ) < 0;
	if ( isLongWayAround ) {
		a *= -1;
	}
	Vec4 lerped = a * ( 1 - t ) + b * t;

	Quat out( lerped.x, lerped.y, lerped.z, lerped.w );
	out.Normalize();
	return out;
}

Quat Slerp( const Quat & from, const Quat & to, float t ) {
	Vec4 qA( from.x, from.y, from.z, from.w );
	Vec4 qB( to.x, to.y, to.z, to.w );

	const float cosTheta = qA.Dot( qB );
	if ( cosTheta > ( 1.f - 0.001 ) ) {		
		return Lerp( from, to, t ); // small enough ang to just lerp
	}

	const float theta		 = acos( cosTheta );
	const float sinTheta	 = sin( theta );
	const float denomInverse = 1.f / sinTheta;
	const float weightA		 = sin( ( 1.f - t ) * theta ) * denomInverse;
	const float weightB		 = sin(         t   * theta ) * denomInverse;

	const Vec4 result	 = qA * weightA + qB * weightB;

//	note - old way has more rounding errors bc not memoizing produces more operations, more opportunities for precision degradation
//	const Vec4 resultOld = 
//		qA * ( sin( theta * ( 1.f - t ) ) / sinTheta ) + 
//		qB * ( sin( theta * t ) / sinTheta );

	const Quat resultQ   = { result.x, result.y, result.z, result.w }; // quat ctor normalizes automatically
	return resultQ;
}

void BoneAnimation::Interpolate( float t, BoneTransform & outTransform ) const {
	if ( keyframes.empty() ) {
		puts( "Bone Animation had no keyframe! Returning..." );
		return;
	}

	// return first keyframe
	if ( t <= keyframes.front().timePos ) {
		outTransform.rotation = keyframes.front().transform.rotation;
		outTransform.translation = keyframes.front().transform.translation;
	} else if ( t >= keyframes.back().timePos ) {
		outTransform.rotation = keyframes.back().transform.rotation;
		outTransform.translation = keyframes.back().transform.translation;
	} else {
		// where does the given time t fall within our list of keyframes?
		for ( size_t i = 0; i < keyframes.size() - 1; i++ ) {
			const Keyframe & start = keyframes[ i ];
			const Keyframe & end   = keyframes[ i + 1 ];
			if ( t >= start.timePos && t <= end.timePos ) {
				// we have found the two keyframes that t lies between, 
				// so interpolate it within the range of start ~ end

				// lerp translation
				const float range		 = end.timePos - start.timePos;
				const float progress	 = ( t - start.timePos ) / range;
				outTransform.translation = start.transform.translation + ( end.transform.translation - start.transform.translation ) * progress;

				// slerp quat
				outTransform.rotation = Slerp( start.transform.rotation, end.transform.rotation, progress );
				printf( "cur time: %10.2f\ncur prog: %2.2f\ncur keyframes: %zu:%zu", t, progress, i, i + 1 );
				break;
			}
		}
	}
}

float AnimationClip::GetClipStartTime() const {
	float minTime = std::numeric_limits< float >::max();
	for ( const BoneAnimation & anim : BoneAnimations ) {
		if ( anim.HasData() ) {
			minTime = std::min( anim.GetStartTime(), minTime );
		}
	}
	return minTime;
}

float AnimationClip::GetClipEndTime() const {
	float maxTime = std::numeric_limits< float >::min();
	for ( const BoneAnimation & anim : BoneAnimations ) {
		if ( anim.HasData() ) {
			maxTime = std::max( anim.GetEndTime(), maxTime );
		}
	}
	return maxTime;
}

// interpolate all individual bone animations and populate a list of the exact transform of each bone
// ( aka, the pose ), at this given time
void AnimationClip::Interpolate( float t, std::vector<BoneTransform> & boneTransforms ) const {
	for ( int i = 0; i < BoneAnimations.size(); i++ ) {
		BoneAnimations[ i ].Interpolate( t, boneTransforms[ i ] );
	}
}

void SkinnedData::Set( 
	const std::vector<BoneInfo_t> & boneHierarchy,
	std::vector<BoneTransform> & boneOffsets, 
	std::map<std::string, AnimationClip> & animations ) {

	// make sure we have no negative keyframes
	for ( auto & iter = animations.begin(); iter != animations.end(); iter++ ) {
		assert( iter->second.HasValidKeyframes() );
	}

	BoneHierarchy.assign( boneHierarchy.begin(), boneHierarchy.end() );
	OffsetMatrices.assign( boneOffsets.begin(), boneOffsets.end() );
	mAnimations.insert( animations.begin(), animations.end() );
}

BoneTransform SkinnedData::FbxToBoneTransform( fbxsdk::FbxQuaternion * q, const fbxsdk::FbxVector4 * t ) {
	return { { 
			static_cast< float >( q->mData[ 0 ] ), 
			static_cast< float >( q->mData[ 1 ] ), 
			static_cast< float >( q->mData[ 2 ] ), 
			static_cast< float >( q->mData[ 3 ]  ) 
		}, { 
			static_cast< float >( t->mData[ 0 ] ), 
			static_cast< float >( t->mData[ 1 ] ), 
			static_cast< float >( t->mData[ 2 ] ) 
		}, false 
	};
}

std::string MakePropName( const std::string & nodeName, const fbxsdk::FbxProperty & prop ) {
	return nodeName + "_" + std::string( prop.GetHierarchicalName() );
}

// @TODO - do we need to undo the pre- and pos- rotation quats?
// https://www.gamedev.net/forums/topic/515878-fbx-sdk-how-to-get-bind-pose/4354881/
void OnFoundBoneCB( void * user, fbxsdk::FbxNode * node ) {
	assert( node->GetNodeAttribute()->GetAttributeType() == fbxsdk::FbxNodeAttribute::EType::eSkeleton );

	SkinnedData * me	  = reinterpret_cast< SkinnedData * >( user );
	const char * boneName = node->GetName();

	fbxsdk::FbxAMatrix localTransform = node->EvaluateLocalTransform( FBXSDK_TIME_INFINITE ); // infinite gets default w/o any anims
	fbxsdk::FbxVector4 translation    = localTransform.GetT();
	fbxsdk::FbxQuaternion rotation    = localTransform.GetQ();

	me->OffsetMatrices.push_back( SkinnedData::FbxToBoneTransform( &rotation, &translation ) );
	const int boneIdx = me->OffsetMatrices.size() - 1;
	if ( !me->BoneIdxMap.insert( { boneName, boneIdx } ).second ) {
		return;
	}

	const char * parentName = node->GetParent()->GetName();
	const bool bFoundParent = me->BoneIdxMap.find( parentName ) != me->BoneIdxMap.end();
	me->BoneHierarchy.push_back( bFoundParent ? me->BoneIdxMap[ parentName ] : -1 );

	// map bone names to properties so we can find them later from animations
	// @TODO - this way of uniquely mapping is obviously broken; need to get the unique identifier from curve instead
	// so we can later map the curve to the bone name when we find animations inside of OnFoundAnimCB
	fbxsdk::FbxAnimLayer * layer = me->activeLayer;
	std::string curveNameS = node->LclScaling.GetCurve( layer )->GetName();
	std::string curveNameR = node->LclRotation.GetCurve( layer )->GetName();
	std::string curveNameT = node->LclTranslation.GetCurve( layer )->GetName();

	std::string nodeName = node->GetName();
	std::string s0 = MakePropName( nodeName, node->LclScaling );
	std::string s1 = MakePropName( nodeName, node->LclRotation );
	std::string s2 = MakePropName( nodeName, node->LclTranslation );
	me->propsToBoneNames[ s0 ] = boneName;
	me->propsToBoneNames[ s1 ] = boneName;
	me->propsToBoneNames[ s2 ] = boneName;
}

const char * SkinnedData::FindBoneFromCurve( fbxsdk::FbxNode * node, fbxsdk::FbxAnimCurve * curve ) const {
	fbxsdk::FbxProperty prop = curve->GetDstProperty();
	if ( !prop.IsValid() ) {
		return nullptr;
	}

	const std::string propName = MakePropName( node->GetName(), prop );
//	const std::string propName = prop.GetHierarchicalName();
	if ( propName.empty() ) {
		return nullptr;
	}

	if ( propsToBoneNames.find( propName ) == propsToBoneNames.end() ) {
		return nullptr;
	}

	return propsToBoneNames.at( propName ).data();
}

void OnFoundAnimCB( void * user, fbxsdk::FbxNode * node ) {
	SkinnedData * me			 = reinterpret_cast< SkinnedData * >( user );
	fbxsdk::FbxAnimLayer * layer = me->activeLayer;
	
	// try all three
	fbxsdk::FbxAnimCurve * curveX = node->LclTranslation.GetCurve( layer, FBXSDK_CURVENODE_COMPONENT_X );
	fbxsdk::FbxAnimCurve * curveY = node->LclTranslation.GetCurve( layer, FBXSDK_CURVENODE_COMPONENT_Y );
	fbxsdk::FbxAnimCurve * curveZ = node->LclTranslation.GetCurve( layer, FBXSDK_CURVENODE_COMPONENT_Z );
	const char * boneName = me->FindBoneFromCurve( node, curveX );
	if ( !boneName ){
		boneName = me->FindBoneFromCurve( node, curveY );
		if ( !boneName ) {
			boneName = me->FindBoneFromCurve( node, curveZ );
			if ( !boneName ) {
				assert( !"FOUND NO BONES ASSOCIATED WITH THIS CURVE!!!!" );
				return;
			}
		}
	}

	BoneAnimation & bAnim = me->mAnimations[ me->activeAnimName.data() ].BoneAnimations[ me->BoneIdxMap.at( boneName ) ];
	me->FillBoneAnimKeyframes( node, layer, bAnim );
}

void SkinnedData::FillBoneAnimKeyframes( fbxsdk::FbxNode * node, fbxsdk::FbxAnimLayer * layer, BoneAnimation & outBoneAnim ) {
	fbxsdk::FbxAnimCurve * curveX = node->LclTranslation.GetCurve( layer, FBXSDK_CURVENODE_COMPONENT_X );
	fbxsdk::FbxAnimCurve * curveY = node->LclTranslation.GetCurve( layer, FBXSDK_CURVENODE_COMPONENT_Y );
	fbxsdk::FbxAnimCurve * curveZ = node->LclTranslation.GetCurve( layer, FBXSDK_CURVENODE_COMPONENT_Z );

	// Determine the number of keyframes. For simplicity, assuming all curves have the same number of keyframes
	int keyframeCount = curveX ? curveX->KeyGetCount() : 0;

	for ( int k = 0; k < keyframeCount; k++ ) {
		Keyframe keyframe;

		// Extract time position
		keyframe.timePos = static_cast< float >( curveX->KeyGetTime( k ).GetSecondDouble() );

		// Extract transforms (translation, rotation, scaling...)
		// This is a simplified example. You will need to extract and convert rotation and scaling as well
		keyframe.transform.translation.x = static_cast< float >( curveX->KeyGetValue( k ) );
		keyframe.transform.translation.y = static_cast< float >( curveY->KeyGetValue( k ) );
		keyframe.transform.translation.z = static_cast< float >( curveZ->KeyGetValue( k ) );

		// Add the keyframe to the bone animation
		outBoneAnim.keyframes.push_back( keyframe );
	}
}

void SkinnedData::Set( fbxsdk::FbxScene * scene, const AnimationAssets::eWhichAnim whichAnim ) {
	// set active layer first so that the per-node callbacks can access it later
	fbxsdk::FbxAnimStack * animStack = scene->GetCurrentAnimationStack();
	activeLayer = animStack->GetMember<fbxsdk::FbxAnimLayer>();
	activeAnimName = animStack->GetName();
	assert( activeLayer != nullptr && !activeAnimName.empty() );

	std::string layerName = activeLayer->GetName();

	// first we need to get the bone hierarchy and populat the bone property map
	FbxUtil::callbackAPI_t cb{ &OnFoundBoneCB, nullptr };
	FbxUtil::HarvestSceneData( scene, false, cb, this );

	assert( BoneCount() > 0 );
	mAnimations.insert( { activeAnimName.data(), BoneCount() } );

	// then we can populate the anim data
	cb = { nullptr, &OnFoundAnimCB };
	FbxUtil::HarvestSceneData( scene, false, cb, this );
	
	// accumulate local bone transforms to bring into component space
	for ( int i = 1; i < BoneCount(); i++ ) {
		const int parentIdx = BoneHierarchy[ i ].GetParent();
		if ( parentIdx >= 0 ) {
			const BoneTransform parentSpaceTransform = OffsetMatrices[ parentIdx ];
			OffsetMatrices[ i ] = parentSpaceTransform * OffsetMatrices[ i ];
		}
	}

	OffsetMatrices_DIRECT_DEBUG.assign( OffsetMatrices.begin(), OffsetMatrices.end() );
	std::transform( OffsetMatrices.begin(), OffsetMatrices.end(), OffsetMatrices.begin(),
					[]( BoneTransform & bTransform ) { return bTransform.Inverse(); } );
}

/*
// Frank Luna uses EXTREMELY unclear language, so I reframe it here more explicitly:

do only once at initialization time:
0. accumulate transforms of bones 0 ~ n in BIND POSE, and store that info

	every frame:
1. interpolate the authored bone transforms ( as defined by the animator ) of bones 0~n as a flat array, 
	each in bone space, based on keyframes ( do NOT concatenate them yet )
2. accumulate transforms of bones 0 ~ n in the ANIMATED POSE that you just interpolated, and store that info
3. multiply each of these bone transforms that you have stored, as a flat array ( dont accumulate ), 
	with the INVERSE of their corresponding bindpose bone transforms stored in step 0, 
	to define these accumulated bone transforms, RELATIVE to the accumulated bone transforms of the bind pose. 
	now they are in model space, and RELATIVE to the bind pose.
4. upload these model space, relative-to-bind-pose bone transforms, to the GPU.
5. in the GPU ( pretend each vert is only influenced by one bone, ignore blending for now ), multiply each vertex by its corresponding bone transform from step 4.
a
	TODO - 
	1) read							https://www.gamedevs.org/uploads/skinned-mesh-and-character-animation-with-directx9.pdf
	2) tutorial						https://www.youtube.com/watch?v=f3Cr8Yx3GGA&list=PLRIWtICgwaX2tKWCxdeB7Wv_rTET9JtWW&index=1
	3) look at how they do skinning https://github.com/vovan4ik123/assimp-Cpp-OpenGL-skeletal-animation
	4) other refs					https://vladh.net/game-engine-skeletal-animation
	5)								https://animcoding.com/post/animation-tech-intro-part-1-skinning/
*/

void SkinnedData::GetFinalTransforms( const std::string & cName, float time, std::vector<BoneTransform> & outFinalTransforms ) const {
	// interpolate flat array of bone transforms in parent space, in the TIME DOMAIN, based on keyframes at this moment
	AnimationClip clip = mAnimations.at( cName );
	std::vector< BoneTransform > interpolatedBoneSpaceTransforms( BoneCount() );
	clip.Interpolate( time, interpolatedBoneSpaceTransforms );

	/********** Bring the flat array of interpolated bones, into MODEL SPACE **********
		accumulating local space bone transforms from ROOT -> LEAF 
			== 
		converting that LEAF bone from local space -> COMPONENT space ( root space ) 
	***********************************************************************************/
	for ( int i = 1; i < BoneCount(); i++ ) {
		const int parentIdx = BoneHierarchy[ i ].GetParent();
		const BoneTransform parentSpaceTransform = interpolatedBoneSpaceTransforms[ parentIdx ];
		interpolatedBoneSpaceTransforms[ i ] = parentSpaceTransform * interpolatedBoneSpaceTransforms[ i ];
	}

	// NOTE - we are pre-concatenating the sequence: inv Bind Pose * Animated Pose
	// normally, the vertex shader will directly multiply each vert by the inv Bind Pose, then the Animated Pose.
	// the way we do it multiplies PER bone instead of per vert, so it's effectively representing each bone animation as a DELTA from its bind pose.
	// ( mathematically same fucking thing ) we do this for debugging, we want to have access to that value to set body transforms directly.
	// 
	// 
	/*	( READING LEFT TO RIGHT... )
		FINAL VERT TRANSFORM ( WS ) = 
			VERT TRANSFORM ( MS ) * [ INV ] BONE BIND_POSE TRANSFORM ( MS ) * BONE ANIMATED_POSE TRANSFORM ( MS ) * 
			MODEL TRANSFORM ( WS ) * VIEW TRANSFORM ( INV CAM MATRIX WS ) * PROJECTION MATRIX ( -> NDC )

			( note model -> world transform happens after the skinning stuff bc that stuff all happens in local space model space )
	
		>> Matrix multiplication is Associative ( can regroup ), so we do these two steps in advance, before sending the bones to the GPU:
			[ INV ] BONE BIND_POSE TRANSFORM ( MS ) * BONE ANIMATED_POSE TRANSFORM ( MS ) *
	*/

	outFinalTransforms.assign( OffsetMatrices.begin(), OffsetMatrices.end() );
	for ( int i = 0; i < BoneCount(); i++ ) {
		outFinalTransforms[ i ] *= interpolatedBoneSpaceTransforms[ i ];
	}
}