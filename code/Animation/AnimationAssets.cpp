#include "AnimationAssets.h"
#include "AnimationData.h"
#include "AnimationState.h"
#include "ModelLoader.h"
#include "fbxInclude.h"
#include "../Config.h"

namespace {
	constexpr float TO_RAD = 3.14159265359f / 180.f;

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
} // anonymous namespace


namespace AnimationAssets {
	#define stringify( s ) #s

	void FillAnimInstanceData( AnimationInstance * animInstance, eSkeleton whichSkeleton, const char * fileName, float scale ) {
		animInstance->animData->skeletonType = whichSkeleton;

		switch ( whichSkeleton ) {
			case SINGLE_BONE: {
				int boneCount;
				std::vector< BoneInfo_t > hierarchy;
				std::vector< BoneTransform > boneOffsets;
				AnimationClip clip( 1 );

				hierarchy.assign( { -1 } );
				boneOffsets.assign( { BoneTransform::Identity() } );
				clip.BoneAnimations.assign( { MakeBoneAnim1(), } );

				std::map< std::string, AnimationClip > animMap = { { stringify( SINGLE_BONE ), clip } };
				animInstance->animData->Set( hierarchy, boneOffsets, animMap );
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
				clip.BoneAnimations.assign( { {},
											  boneAnim,
											  boneAnim,
											  boneAnim,
											  boneAnim,
											  boneAnim,
											  boneAnim,
											  boneAnim,
											  boneAnim } );
				assert( clip.BoneAnimations.size() == boneCount );

				std::map< std::string, AnimationClip > animMap = { { stringify( MULTI_BONE ), clip } };
				animInstance->animData->Set( hierarchy, boneOffsets, animMap );
				break;
			}
			case DEBUG_SKELETON:
			case SKINNED_MESH:
			default: {
				const bool loaded = FbxUtil::LoadFBXFile(
					fileName,
					[]( bool status, fbxsdk::FbxImporter * pImporter, FbxScene * scene, void * userData ) {
						if ( !status ) {
							printf( "Error - Failed to load FBX Scene.\n" );
							return;
						}
						reinterpret_cast< SkinnedData * >( userData )->Set( scene );
					},
					animInstance->animData,
					CONVERT_SCENE,
					scale
				);
				break;
			}
		}
	}
	#undef stringify
} // namespace AnimationAssets