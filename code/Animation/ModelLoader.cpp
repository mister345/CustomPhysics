#include <thread>
#include <iostream>
#include <iostream>
#include <cassert>
#include <unordered_map>
#include <queue>
#include "fbxInclude.h"
#include "Bone.h"
#include "ModelLoader.h"
#include "AnimationData.h"
#include "../Config.h"
#include <stack>

namespace FbxUtil {
	bool InitializeSdkObjects( fbxsdk::FbxManager *& pManager, fbxsdk::FbxImporter *& pImporter );
	void ProcessNodeInternal( fbxsdk::FbxNode * pNode, const FbxUtil::callbackAPI_t & callback, void * dataRecipient );
	void PrintNodeTransform( fbxsdk::FbxNode * node );
	void PrintSceneAnimData( fbxsdk::FbxImporter * pImporter );
	int CountBonesInSkeleton( fbxsdk::FbxNode * rootNode );
} // fwd declarations for internal use 


namespace FbxUtil {
	float g_scale = 1.f;

	// EXPERIMENT w T-pose
	void PrintNodeTransform( fbxsdk::FbxNode * node ) {
		if ( node == nullptr ) {
			printf( "Null node provided.\n" );
			return;
		}

		fbxsdk::FbxVector4 translation = node->LclTranslation.Get();
		fbxsdk::FbxVector4 rotation = node->LclRotation.Get();
		fbxsdk::FbxVector4 scaling = node->LclScaling.Get();

		printf( "%s:\n", node->GetName() );
		printf( "\tTranslation: %f, %f, %f\n", translation[ 0 ], translation[ 1 ], translation[ 2 ] );
		printf( "\tRotation: %f, %f, %f\n", rotation[ 0 ], rotation[ 1 ], rotation[ 2 ] );
		printf( "\tScaling: %f, %f, %f\n", scaling[ 0 ], scaling[ 1 ], scaling[ 2 ] );

	}

	/////////////////////////////////////
	// Data Extraction Functions
	/////////////////////////////////////
	void ProcessNodeTPose( fbxsdk::FbxNode * pNode, const onFoundTPose_fn & callback, void * dataRecipient ) {
		if ( pNode != nullptr ) {
			PrintNodeTransform( pNode );
			callback( pNode, dataRecipient );
			for ( int j = 0; j < pNode->GetChildCount(); j++ ) {
				ProcessNodeTPose( pNode->GetChild( j ), callback, dataRecipient );
			}
		}
	}
	void HarvestTPose( fbxsdk::FbxScene * pScene, const onFoundTPose_fn & callback, void * caller ) {
		fbxsdk::FbxNode * pRootNode = pScene->GetRootNode();
		if ( pRootNode != nullptr ) {
			for ( int i = 0; i < pRootNode->GetChildCount(); i++ ) {
				ProcessNodeTPose( pRootNode->GetChild( i ), callback, caller );
			}
		}
	}
	void ProcessNodes_Q( fbxsdk::FbxNode * pNode, const callbackAPI_t & callbacks, void * dataRecipient ) {
		// Aggregate
		std::stack< fbxsdk::FbxNode * > jobs;
		std::queue< fbxsdk::FbxNode * > q;
		q.push( pNode );
		while ( !q.empty() ) {
			FbxNode * curNode = q.front();
			q.pop();
			jobs.push( curNode );
			int childCt = curNode->GetChildCount();
			for ( int j = 0; j < childCt; j++ ) {
				fbxsdk::FbxNode * child = curNode->GetChild( j );
				if ( child != nullptr ) {	// @TODO - necessary?
					q.push( child );
				}
			}
		}

		// Process	
		struct worker_t {
			std::thread t;
			worker_t( std::vector< fbxsdk::FbxNode * > & assignedWork, const FbxUtil::callbackAPI_t & callbacks_, void * dataRecipient_ ) {
				t = std::move( std::thread(
						[]( std::vector< fbxsdk::FbxNode * > & jobs, const FbxUtil::callbackAPI_t & callbacks, void * dataRecipient ) {							
							for( int i = 0; i < jobs.size(); i++ ){
								if ( jobs[ i ] == nullptr ) {
									return;
								}
								ProcessNodeInternal( jobs[ i ], callbacks, dataRecipient ); 
							} },
						assignedWork,
						callbacks_, 
						dataRecipient_ 
					) 
				);

			}
			~worker_t() {
				t.join();
			}
		};


		// distribute jobs into n threads distinct arrays
		const unsigned nThreads = std::min( std::thread::hardware_concurrency(), NUM_THREADS_LOAD );
		std::vector< std::vector< fbxsdk::FbxNode * > >jobsAssigned( nThreads );
		while ( !jobs.empty() ) {
			for ( int i = 0; i < nThreads; i++ ) {
				FbxNode * node = nullptr;
				do {
					node = jobs.top();
					jobs.pop();
				} while ( node == nullptr );

				jobsAssigned[ i ].push_back( node );
			}
		}

		// create n threads workers, each with their own array of jobs
		std::vector< worker_t * > workers( nThreads, nullptr );
		for ( int i = 0; i < nThreads; i++ ) {
			workers[ i ] = new worker_t( jobsAssigned[ i ], callbacks, dataRecipient );
		}

		// destroy the workers ( this will call thread::join() for each one )
		for ( int i = 0; i < workers.size(); i++ ) {
			delete( workers[ i ] );
			workers[ i ] = nullptr;
		}
		workers.clear();

		/*	@TODO
		*	1. make sure all data being operated on is threadsafe
		*		>> already know this is calling emplace_back() on the matrix palette; need to preallocate instead
		*	2. switch to thread pool
		*	3. allow threads who finish first to steal from queue ( bc variant num verts per job )
		*/ 
	}

	int g_recursiveHitCt = 0;
	void ProcessNodes_R( fbxsdk::FbxNode * pNode, const callbackAPI_t & callback, void * dataRecipient ) {
		g_recursiveHitCt++;
		if ( pNode != nullptr ) {
			ProcessNodeInternal( pNode, callback, dataRecipient );

			for ( int j = 0; j < pNode->GetChildCount(); j++ ) {
				ProcessNodes_R( pNode->GetChild( j ), callback, dataRecipient );
			}
		}
	}

	void ProcessNodeInternal( fbxsdk::FbxNode * pNode, const FbxUtil::callbackAPI_t & callback, void * dataRecipient ) {
//		PrintNodeTransform( pNode );
		for ( int i = 0; i < pNode->GetNodeAttributeCount(); i++ ) {
			fbxsdk::FbxNodeAttribute * pAttribute = pNode->GetNodeAttributeByIndex( i );
			if ( pAttribute != nullptr ) {
				// PrintAttribute( pAttribute );
				const fbxsdk::FbxNodeAttribute::EType attrType = pAttribute->GetAttributeType();
				if ( attrType == fbxsdk::FbxNodeAttribute::EType::eSkeleton && callback.onFoundBone != nullptr ) {
					callback.onFoundBone( dataRecipient, pNode );
					// break;	// @TODO - can a node ever have BOTH bone and MESH attributes? if so, this is a problem!
				} else if ( attrType == fbxsdk::FbxNodeAttribute::EType::eMesh && callback.onFoundMesh != nullptr ) {
					callback.onFoundMesh( dataRecipient, pNode );
					// break; // @TODO - can a node ever have BOTH bone and MESH attributes? if so, this is a problem!
				}
			}
		}
	}

	/////////////////////////////////////
	// Init Functions
	/////////////////////////////////////
	bool LoadFBXFile( const char * filename, FbxUtil::onLoadedCallback_fn onLoaded, void * userData, bool bConvert, float scale /* = 1.f */ ) {
		FbxUtil::g_scale = scale;

		fbxsdk::FbxManager * pManager = nullptr;
		fbxsdk::FbxImporter * pImporter = nullptr;

		if ( !InitializeSdkObjects( pManager, pImporter ) ) {
			return false;
		}

		if ( !pImporter->Initialize( filename, -1, pManager->GetIOSettings() ) ) {
			std::cout << "Error: Unable to initialize FBX Importer!\n";
			return false;
		}

		// Create a new scene so it can be populated by the imported file
		fbxsdk::FbxScene * pScene = fbxsdk::FbxScene::Create( pManager, "myScene" );

		// Import the contents of the file into the scene
		bool lStatus = pImporter->Import( pScene );

		if ( bConvert ) {
			// triangulate
			FbxGeometryConverter clsConverter( pManager );
			clsConverter.Triangulate( pScene, true );

			// convert to our coordinate system
			fbxsdk::FbxAxisSystem targetAxisSystem( fbxsdk::FbxAxisSystem::EUpVector::eZAxis,
													fbxsdk::FbxAxisSystem::EFrontVector::eParityEven,
													fbxsdk::FbxAxisSystem::ECoordSystem::eRightHanded );
			targetAxisSystem.DeepConvertScene( pScene );
		}

		onLoaded( lStatus, pImporter, pScene, userData );

		// Destroy the SDK manager and all other objects it was handling
		// Destroy the importer
		pImporter->Destroy();
		pManager->Destroy();

		return lStatus;
	}

	bool InitializeSdkObjects( fbxsdk::FbxManager *& pManager, fbxsdk::FbxImporter *& pImporter ) {
		pManager = fbxsdk::FbxManager::Create();
		if ( !pManager ) {
			std::cout << "Error: Unable to create FBX Manager!\n";
			return false;
		}

		fbxsdk::FbxIOSettings * ios = fbxsdk::FbxIOSettings::Create( pManager, IOSROOT );
		pManager->SetIOSettings( ios );

		pImporter = FbxImporter::Create( pManager, "" );
		if ( !pImporter ) {
			std::cout << "Error: Unable to create FBX Importer!\n";
			return false;
		}

		return true;
	}

	/////////////////////////////////////
	// Debug INFO functions
	/////////////////////////////////////
	int CountBonesInSkeleton( fbxsdk::FbxNode * rootNode ) {
		struct local_t {
			static void countBones_r( fbxsdk::FbxNode * node, int & boneCount ) {
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
		};

		int boneCount = 0;
		local_t::countBones_r( rootNode, boneCount );
		return boneCount;
	}

	void PrintScene( fbxsdk::FbxScene * pScene ) {
		printf( "Scene Name: %s\n--------------------------------------\n", pScene->GetName() );

		// Get and print coordinate system information
		fbxsdk::FbxAxisSystem axisSystem = pScene->GetGlobalSettings().GetAxisSystem();
		std::string coordSystem = axisSystem.GetCoorSystem() == fbxsdk::FbxAxisSystem::eRightHanded ? "Right-Handed" : "Left-Handed";
		printf( "Coordinate System: %s\n", coordSystem.c_str() );

		// Determine and print the Up, Front and Right Axis
		int upAxisSign, frontAxisSign;
		fbxsdk::FbxAxisSystem::EUpVector upAxis = axisSystem.GetUpVector( upAxisSign );
		fbxsdk::FbxAxisSystem::EFrontVector frontAxis = axisSystem.GetFrontVector( frontAxisSign );

		// Determine Right Axis based on the right-handed or left-handed coordinate system
		std::string rightAxis = coordSystem == "Right-Handed" ? "+X" : "-X";

		// Determine Up Axis
		std::string upAxisString = ( upAxis == fbxsdk::FbxAxisSystem::eXAxis ? "X" : ( upAxis == fbxsdk::FbxAxisSystem::eYAxis ? "Y" : "Z" ) );
		upAxisString = ( upAxisSign > 0 ? "+" : "-" ) + upAxisString;

		// Determine Front Axis (assuming Z is the default front axis)
		std::string frontAxisString = ( frontAxis == fbxsdk::FbxAxisSystem::eParityOdd ? "-Z" : "+Z" );
		// @TODO - this is currently hardcoded and will no longer work if front axis isnt Z; fix

		// Print axes in X, Y, Z order
		printf( "Right Axis: %s\n", rightAxis.c_str() );
		printf( "Up    Axis: %s\n", upAxisString.c_str() );
		printf( "Fwd   Axis: %s\n", frontAxisString.c_str() );
	}
	void PrintSceneAnimData( fbxsdk::FbxImporter * pImporter ) {
		// print anim data
		FBXSDK_printf( "Animation Stack Information\n" );
		int lAnimStackCount = pImporter->GetAnimStackCount();
		FBXSDK_printf( "    Number of Animation Stacks: %d\n", lAnimStackCount );
		FBXSDK_printf( "    Current Animation Stack: \"%s\"\n", pImporter->GetActiveAnimStackName().Buffer() );
		FBXSDK_printf( "\n" );
		for ( int i = 0; i < lAnimStackCount; i++ ) {
			FbxTakeInfo * lTakeInfo = pImporter->GetTakeInfo( i );
			FBXSDK_printf( "    Animation Stack %d\n", i );
			FBXSDK_printf( "         Name: \"%s\"\n", lTakeInfo->mName.Buffer() );
			FBXSDK_printf( "         Description: \"%s\"\n", lTakeInfo->mDescription.Buffer() );
			// Change the value of the import name if the animation stack should be imported 
			// under a different name.
			FBXSDK_printf( "         Import Name: \"%s\"\n", lTakeInfo->mImportName.Buffer() );
			// Set the value of the import state to false if the animation stack should be not
			// be imported. 
			FBXSDK_printf( "         Import State: %s\n", lTakeInfo->mSelect ? "true" : "false" );
			FBXSDK_printf( "\n" );
		}
	}
	void PrintNode( fbxsdk::FbxNode * pNode ) {
		printf( "Node Name: %s\n", pNode->GetName() );

		// Print rotation order
		fbxsdk::FbxEuler::EOrder rotOrder;
		pNode->GetRotationOrder( fbxsdk::FbxNode::eSourcePivot, rotOrder );
		std::string rotOrderStr;
		switch ( rotOrder ) {
			case fbxsdk::FbxEuler::eEulerXYZ: rotOrderStr = "XYZ"; break;
			case fbxsdk::FbxEuler::eEulerXZY: rotOrderStr = "XZY"; break;
			case fbxsdk::FbxEuler::eEulerYZX: rotOrderStr = "YZX"; break;
			case fbxsdk::FbxEuler::eEulerYXZ: rotOrderStr = "YXZ"; break;
			case fbxsdk::FbxEuler::eEulerZXY: rotOrderStr = "ZXY"; break;
			case fbxsdk::FbxEuler::eEulerZYX: rotOrderStr = "ZYX"; break;
			case fbxsdk::FbxEuler::eSphericXYZ: rotOrderStr = "Spherical XYZ"; break;
			default: rotOrderStr = "Unknown"; break;
		}
		printf( "    Rotation Order: %s\n", rotOrderStr.c_str() );
	}
	void PrintBone( fbxsdk::FbxNode * pNode ) {
		fbxsdk::FbxAMatrix localTransform = pNode->EvaluateLocalTransform( FBXSDK_TIME_INFINITE ); // infinite gets default w/o any anims
		fbxsdk::FbxVector4 translation = localTransform.GetT();
		fbxsdk::FbxVector4 rotation = localTransform.GetR();
		fbxsdk::FbxVector4 scaling = localTransform.GetS();
		printf( "    Bone Transform:\n" );
		printf( "        Translation: %f, %f, %f\n", translation[ 0 ], translation[ 1 ], translation[ 2 ] );
		printf( "        Rotation: %f, %f, %f\n", rotation[ 0 ], rotation[ 1 ], rotation[ 2 ] );
		printf( "        Scaling: %f, %f, %f\n", scaling[ 0 ], scaling[ 1 ], scaling[ 2 ] );
	}
	void PrintAttribute( fbxsdk::FbxNodeAttribute * pAttribute ) {
		fbxsdk::FbxString typeName = pAttribute->GetTypeName();
		fbxsdk::FbxString attrName = pAttribute->GetName();
		printf( "    Attribute Type: %s\n", typeName.Buffer() );
		printf( "    Attribute Name: %s\n", attrName.Buffer() );
	}
} // namepsace fbx util