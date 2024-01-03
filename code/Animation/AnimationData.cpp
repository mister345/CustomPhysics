#include <vector>
#include "ModelLoader.h"
#include "../Math/Vector.h"
#include "../Math/Quat.h"
#include "AnimationData.h"
#include "AnimationState.h"
#include <algorithm>

#include "../../libs/FBX/2020.3.4/include/fbxsdk/scene/animation/fbxanimstack.h"
#include "../../libs/FBX/2020.3.4/include/fbxsdk/scene/animation/fbxanimcurve.h"
#include "../../libs/FBX/2020.3.4/include/fbxsdk/scene/animation/fbxanimevaluator.h"
#include <cassert>

// Utility functions
namespace {
	void FbxEulerToQuat( const fbxsdk::FbxVector4 & euler, Quat & outQuat ) {
		fbxsdk::FbxQuaternion fbxQuat;
		fbxQuat.ComposeSphericalXYZ( euler );
		outQuat.x = fbxQuat[ 0 ];
		outQuat.y = fbxQuat[ 1 ];
		outQuat.z = fbxQuat[ 2 ];
		outQuat.w = fbxQuat[ 3 ];
	}

	Quat Lerp( const Quat & from, const Quat & to, float t ) {
		Vec4 a( from.x, from.y, from.z, from.w );
		Vec4 b( to.x, to.y, to.z, to.w );

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

		const float theta = acos( cosTheta );
		const float sinTheta = sin( theta );
		const float denomInverse = 1.f / sinTheta;
		const float weightA = sin( ( 1.f - t ) * theta ) * denomInverse;
		const float weightB = sin( t * theta ) * denomInverse;

		const Vec4 result = qA * weightA + qB * weightB;

		//	note - old way has more rounding errors bc not memoizing produces more operations, more opportunities for precision degradation
		//	const Vec4 resultOld = 
		//		qA * ( sin( theta * ( 1.f - t ) ) / sinTheta ) + 
		//		qB * ( sin( theta * t ) / sinTheta );

		const Quat resultQ = { result.x, result.y, result.z, result.w }; // quat ctor normalizes automatically
		return resultQ;
	}

	void countBones_r( fbxsdk::FbxNode * node, int & boneCount ) {
		if ( node != nullptr ) {
			fbxsdk::FbxNodeAttribute * attribute = node->GetNodeAttribute();
			if ( attribute && attribute->GetAttributeType() == fbxsdk::FbxNodeAttribute::eSkeleton ) {
				boneCount++;
			}
			for ( int i = 0; i < node->GetChildCount(); i++ ) {
				countBones_r( node->GetChild( i ), boneCount );
			}
		}
	}

	int CountBonesInSkeleton( fbxsdk::FbxNode * rootNode ) {
		int boneCount = 0;
		countBones_r( rootNode, boneCount );
		return boneCount;
	}
} // anonymous namespace

namespace AnimationAssets {
	constexpr float TO_RAD = 3.14159265359f / 180.f;

	std::vector< std::string > animNames = {
		"SingleBone",
		"MultiBone",
		"SkeletonOnly",
		"SkinnedMesh",
		"Count"
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
	const float scale = FbxUtil::g_scale;

	return { { 
			static_cast< float >( q->mData[ 0 ] ), 
			static_cast< float >( q->mData[ 1 ] ), 
			static_cast< float >( q->mData[ 2 ] ), 
			static_cast< float >( q->mData[ 3 ]  ) 
		}, { 
			static_cast< float >( t->mData[ 0 ] * scale ),
			static_cast< float >( t->mData[ 1 ] * scale ),
			static_cast< float >( t->mData[ 2 ] * scale )
		}, false 
	};
}

void OnFoundBoneCB( void * user, fbxsdk::FbxNode * boneNode ) {
	assert( boneNode->GetNodeAttribute()->GetAttributeType() == fbxsdk::FbxNodeAttribute::EType::eSkeleton );

	SkinnedData * me	  = reinterpret_cast< SkinnedData * >( user );
	const char * boneName = boneNode->GetName();

	/////////////////////////////////////////////////////////////////
	// Populate OffsetMatrices array Bone Map, and Bone Hierarchy
	/////////////////////////////////////////////////////////////////
	// @BUG - this is NOT getting the bind pose; it's getting the pose @ time 0 of top anim in stack!
	fbxsdk::FbxAMatrix localTransform = boneNode->EvaluateLocalTransform( FBXSDK_TIME_INFINITE ); // infinite gets default w/o any anims
	fbxsdk::FbxVector4 translation    = localTransform.GetT();
	fbxsdk::FbxQuaternion rotation    = localTransform.GetQ();
	me->OffsetMatrices.push_back( SkinnedData::FbxToBoneTransform( &rotation, &translation ) );

	const int boneIdx = me->OffsetMatrices.size() - 1;
	if ( !me->BoneIdxMap.insert( { boneName, boneIdx } ).second ) {
		return;
	}

	const char * parentName = boneNode->GetParent()->GetName();
	const bool bFoundParent = me->BoneIdxMap.find( parentName ) != me->BoneIdxMap.end();
	me->BoneHierarchy.push_back( bFoundParent ? me->BoneIdxMap[ parentName ] : -1 );

	//////////////////////////////////////
	// Get all animations for this bone
	//////////////////////////////////////
	fbxsdk::FbxAnimStack * curAnimStack = me->animStack;
	fbxsdk::FbxAnimLayer * curAnimLayer = curAnimStack->GetMember< fbxsdk::FbxAnimLayer >(); // Assume only one layer for now

	// @TODO - curAnimName shouldnt be a thing bc this is not STATEFUL
	AnimationClip & clip = me->mAnimations[ me->curAnimName ];

	// Now we pass the responsibility to FillBoneAnimKeyframes to populate the animation
//	me->FillBoneAnimKeyframes( boneNode, curAnimLayer, clip.BoneAnimations[ boneIdx ] );
	me->FillBoneAnimKeyframesResample( boneNode, curAnimLayer, clip, boneIdx );
}

/* NOTE - 2 ways to get frames from fbx file
	1. Store key frames only ( raw animation data from within the fbx file )
	2. Sample frames from the fbx file at a fixed rate - loses raw animation data, 
		bc its resampling the original keyframes according to some arbitrary logic. */

// https://www.gamedev.net/articles/programming/graphics/how-to-work-with-fbx-sdk-r3582/
// alternate ( more common way of doing it - sample the bone transform at current time
void SkinnedData::FillBoneAnimKeyframesResample( fbxsdk::FbxNode * boneNode, fbxsdk::FbxAnimLayer * layer, AnimationClip & clip, int whichBoneIdx ) {
	using namespace fbxsdk;
	fbxsdk::FbxAnimStack * anim = fbxScene->GetCurrentAnimationStack();
	FbxTime start				= anim->GetLocalTimeSpan().GetStart();
	FbxTime stop				= anim->GetLocalTimeSpan().GetStop();
	FbxLongLong animLen			= stop.GetFrameCount( FbxTime::eFrames24 ) - start.GetFrameCount( FbxTime::eFrames24 ) + 1;
	BoneAnimation & outBoneAnim = clip.BoneAnimations[ whichBoneIdx ];

	for ( FbxLongLong i = start.GetFrameCount( FbxTime::eFrames24 ); i <= stop.GetFrameCount( FbxTime::eFrames24 ); ++i ) {
		outBoneAnim.keyframes.push_back( {} );
		Keyframe & keyframeToFill = outBoneAnim.keyframes.back();

		FbxTime curTime;
		curTime.SetFrame( i, FbxTime::eFrames24 );
		FbxAMatrix curTransform  = boneNode->EvaluateLocalTransform( curTime ); // infinite gets default w/o any anims
		FbxVector4 translation   = curTransform.GetT();
		FbxQuaternion rotation   = curTransform.GetQ();
		keyframeToFill.transform = SkinnedData::FbxToBoneTransform( &rotation, &translation );
		keyframeToFill.timePos   = curTime.GetSecondCount();
	}
}

// This function assumes that the animation curves for translation and rotation are directly associated with the node,
// and that the curves are for the node's local translation and rotation
// @TODO - do we need to undo the pre- and pos- rotation quats?
// https://www.gamedev.net/forums/topic/515878-fbx-sdk-how-to-get-bind-pose/4354881/
void SkinnedData::FillBoneAnimKeyframesDirect( fbxsdk::FbxNode * boneNode, fbxsdk::FbxAnimLayer * layer, AnimationClip & clip, int whichBoneIdx ) {
	const float scale = FbxUtil::g_scale;

	BoneAnimation & outBoneAnim = clip.BoneAnimations [ whichBoneIdx ];

	auto GetCurveValue = []( fbxsdk::FbxAnimCurve * curve, int keyIndex ) {
		return curve ? static_cast< float >( curve->KeyGetValue( keyIndex ) ) : 0.0f;
	};
	auto CurveKeyCount = []( fbxsdk::FbxAnimCurve * curve ) {
		return curve ? curve->KeyGetCount() : 0;
	};

	// Extract the animation curves for translation
	fbxsdk::FbxAnimCurve * tCurveX = boneNode->LclTranslation.GetCurve( layer, FBXSDK_CURVENODE_COMPONENT_X );
	fbxsdk::FbxAnimCurve * tCurveY = boneNode->LclTranslation.GetCurve( layer, FBXSDK_CURVENODE_COMPONENT_Y );
	fbxsdk::FbxAnimCurve * tCurveZ = boneNode->LclTranslation.GetCurve( layer, FBXSDK_CURVENODE_COMPONENT_Z );

	// Extract the animation curves for rotation
	fbxsdk::FbxAnimCurve * rCurveX = boneNode->LclRotation.GetCurve( layer, FBXSDK_CURVENODE_COMPONENT_X );
	fbxsdk::FbxAnimCurve * rCurveY = boneNode->LclRotation.GetCurve( layer, FBXSDK_CURVENODE_COMPONENT_Y );
	fbxsdk::FbxAnimCurve * rCurveZ = boneNode->LclRotation.GetCurve( layer, FBXSDK_CURVENODE_COMPONENT_Z );

	// Ensure we have valid curves and determine the keyframe count

	// BUG - we cant assume all props have the same num keryframes!
	//int keyframeCount = std::max( { CurveKeyCount( tCurveX ), CurveKeyCount( tCurveY ), CurveKeyCount( tCurveZ ),
	//							  CurveKeyCount( rCurveX ), CurveKeyCount( rCurveY ), CurveKeyCount( rCurveZ ) } );
	int keyframeCount0 = CurveKeyCount( tCurveX );
	int keyframeCount1 = CurveKeyCount( tCurveY );
	int keyframeCount2 = CurveKeyCount( tCurveZ );
	int keyframeCount3 = CurveKeyCount( rCurveX );
	int keyframeCount4 = CurveKeyCount( rCurveY );
	int keyframeCount5 = CurveKeyCount( rCurveZ );

	int keyframeCount = std::min( keyframeCount0, 
						std::min( keyframeCount1, 
						std::min( keyframeCount2, 
						std::min( keyframeCount3, 
						std::min( keyframeCount4, keyframeCount5 ) ) ) ) );

	// @TODO ....
	for ( int k = 0; k < keyframeCount; k++ ) {
		Keyframe keyframe;

		// Assume the time is the same for all translation and rotation curves
		keyframe.timePos = static_cast< float >( ( tCurveY ? tCurveY : rCurveY )->KeyGetTime( k ).GetSecondDouble() );

		// Translation
		keyframe.transform.translation.x = GetCurveValue( tCurveX, k ) * scale;
		keyframe.transform.translation.y = GetCurveValue( tCurveY, k ) * scale;
		keyframe.transform.translation.z = GetCurveValue( tCurveZ, k ) * scale;

		// Rotation - converting from Euler to quaternion
		const float x = GetCurveValue( rCurveX, k );
		const float y = GetCurveValue( rCurveY, k );
		const float z = GetCurveValue( rCurveZ, k );

		Quat q;

		FbxEulerToQuat( { x, y, z }, q );
		keyframe.transform.rotation = q;
		outBoneAnim.keyframes.push_back( keyframe );
	}
}

//	https://download.autodesk.com/us/fbx/20112/fbx_sdk_help/index.html?url=WS73099cc142f48755-13b8418f12724676a534302.htm,topicNumber=d0e6765
//	An animation curve ...  can be connected to many curve nodes; one animation curve can animate many properties of many FBX objects.
//	An animation curve NODE can be connected to ONLY ONE property of one FBX object. 
std::string SkinnedData::BoneNameFromCurve( fbxsdk::FbxAnimCurve * curve ) {
	fbxsdk::FbxProperty prop = curve->GetDstProperty();
	if ( !prop.IsValid() ) { 
		return "";
	}
	return prop.GetDstObject()->GetName();
}

void SkinnedData::BoneSpaceToModelSpace( int boneIdx, std::vector< BoneTransform > & inOutBoneTransforms ) const {
	const int parentIdx = BoneHierarchy[ boneIdx ].GetParent();
	if ( parentIdx < 0 ) {
		return;
	}

	BoneTransform & outBoneTransform = inOutBoneTransforms[ boneIdx ];
	outBoneTransform = inOutBoneTransforms[ parentIdx ] * outBoneTransform;
}

// https://stackoverflow.com/questions/45690006/fbx-sdk-skeletal-animations
void SkinnedData::Set( fbxsdk::FbxScene * scene, const AnimationAssets::eWhichAnim whichAnim ) {
	// set active layer first so that the per-node callbacks can access it later
	if ( scene == nullptr ) {
		assert( !"ERROR - trying to load data from an empty FbxScene!" );
		return;
	}

	fbxScene				   = scene;
	animStack				   = fbxScene->GetCurrentAnimationStack();
	activeLayer				   = animStack->GetMember<fbxsdk::FbxAnimLayer>();
	const std::string animName = animStack->GetName();
	AnimationAssets::animNames[ AnimationAssets::eWhichAnim::SKELETON_ONLY ] = animName;
	AnimationAssets::animNames[ AnimationAssets::eWhichAnim::SKINNED_MESH ]  = animName;
	assert( activeLayer != nullptr && !animName.empty() );
	const int boneCount = CountBonesInSkeleton( fbxScene->GetRootNode() );

	curAnimName = animName.c_str();
	mAnimations.insert( { curAnimName, AnimationClip( boneCount ) } );
	FbxUtil::callbackAPI_t cb{ &OnFoundBoneCB, nullptr };
	FbxUtil::HarvestSceneData( fbxScene, false, cb, this );

	// accumulate local bone transforms to bring into component space
	for ( int i = 1; i < boneCount; i++ ) {
		BoneSpaceToModelSpace( i, OffsetMatrices );
	}

	OffsetMatrices_DIRECT_DEBUG.assign( OffsetMatrices.begin(), OffsetMatrices.end() );
	std::transform( OffsetMatrices.begin(), OffsetMatrices.end(), OffsetMatrices.begin(),
					[]( BoneTransform & bTransform ) { return bTransform.Inverse(); } );
}

void SkinnedData::GetFinalTransforms( const std::string & cName, float time, std::vector<BoneTransform> & outFinalTransforms ) const {
	const int boneCount = BoneCount();
	AnimationClip clip  = mAnimations.at( cName );

	// get interpolated transform for every bone at THIS TIME
	std::vector< BoneTransform > interpolatedBoneSpaceTransforms( boneCount );
	clip.Interpolate( time, interpolatedBoneSpaceTransforms );

	// bring all the interpolated bones into model space ( accumulate from root to leaf )
	for ( int i = 1; i < boneCount; i++ ) {
		BoneSpaceToModelSpace( i, interpolatedBoneSpaceTransforms );
	}

	// @TODO - multiply these by inverse bind pose matrix ONLY if vertex shader bound
	// for debug purposes, we just use the loaded anim poses directly as they are already in model space
	outFinalTransforms = std::vector< BoneTransform >( boneCount, BoneTransform::Identity() );
	for ( int i = 0; i < boneCount; i++ ) {
		outFinalTransforms[ i ] *= interpolatedBoneSpaceTransforms[ i ];
	}
}