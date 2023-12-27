#include "ModelLoader.h"
#include <iostream>
#include <cassert>

namespace FbxUtil {
	/////////////////////////////////////
	// Debug print functions
	/////////////////////////////////////
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

	/////////////////////////////////////
	// Data Extraction Functions
	/////////////////////////////////////
	void ProcessNode( fbxsdk::FbxNode * pNode, const callbackAPI_t & callback, void * dataRecipient ) {
		if ( pNode != nullptr ) {
//			PrintNode( pNode );
			for ( int i = 0; i < pNode->GetNodeAttributeCount(); i++ ) {
				fbxsdk::FbxNodeAttribute * pAttribute = pNode->GetNodeAttributeByIndex( i );
				if ( pAttribute != nullptr ) {
					PrintAttribute( pAttribute );

					if( pNode->GetNodeAttribute()->GetAttributeType() == 
						fbxsdk::FbxNodeAttribute::EType::eSkeleton && 
						callback.onFoundBone != nullptr ) {

//						PrintBone( pNode );
						callback.onFoundBone( dataRecipient, pNode );
					}

					fbxsdk::FbxAnimStack * animStack = pNode->GetScene()->GetCurrentAnimationStack();
					if ( animStack != nullptr && 
						 callback.onFoundAnimNode != nullptr ) {
						callback.onFoundAnimNode( dataRecipient, pNode );
					}
				}
			}
			for ( int j = 0; j < pNode->GetChildCount(); j++ ) {
				ProcessNode( pNode->GetChild( j ), callback, dataRecipient );
			}
		}
	}

	void ConvertScene( fbxsdk::FbxScene * pScene ) {
		constexpr auto xIntoScreen  = fbxsdk::FbxAxisSystem::EFrontVector::eParityEven; // even = positive
		constexpr auto xOutOfScreen = fbxsdk::FbxAxisSystem::EFrontVector::eParityOdd; // odd  = negative
		fbxsdk::FbxAxisSystem newAxisSystem( fbxsdk::FbxAxisSystem::EUpVector::eZAxis, xIntoScreen, fbxsdk::FbxAxisSystem::ECoordSystem::eRightHanded );
//		newAxisSystem.ConvertScene( pScene );
		newAxisSystem.DeepConvertScene( pScene );
	}

	void HarvestSceneData( fbxsdk::FbxScene * pScene, bool bConvert, const callbackAPI_t & callback, void * caller ) {
		PrintScene( pScene );
		if ( bConvert ) {
			ConvertScene( pScene );
			printf( "/nAFTER CONVERSION:/n" );
			PrintScene( pScene );
		}

		fbxsdk::FbxNode * pRootNode = pScene->GetRootNode();
		if ( pRootNode != nullptr ) {
			for ( int i = 0; i < pRootNode->GetChildCount(); i++ ) {
				ProcessNode( pRootNode->GetChild( i ), callback, caller );
			}
		}
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

	bool LoadFBXFile( const char * filename, FbxUtil::onLoadedCallback_fn onLoaded, void * userData ) {
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

		// Destroy the importer
		pImporter->Destroy();

		onLoaded( lStatus, pScene, userData );

		// Destroy the SDK manager and all other objects it was handling
		pManager->Destroy();

		return lStatus;
	}

} // namepsace fbx util