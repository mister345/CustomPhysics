#include "ModelLoader.h"
#include <iostream>

void PrintNode( FbxNode * pNode ) {
    if ( pNode ) {
        std::cout << "Node Name: " << pNode->GetName() << std::endl;
        for ( int i = 0; i < pNode->GetNodeAttributeCount(); i++ ) {
            FbxNodeAttribute * pAttribute = pNode->GetNodeAttributeByIndex( i );

            if ( pAttribute != nullptr ) {
                FbxString typeName = pAttribute->GetTypeName();
                FbxString attrName = pAttribute->GetName();
                std::cout << "    Attribute Type: " << typeName.Buffer() << std::endl;
                std::cout << "    Attribute Name: " << attrName.Buffer() << std::endl;
            }
        }
        for ( int j = 0; j < pNode->GetChildCount(); j++ ) {
            PrintNode( pNode->GetChild( j ) );
        }
    }
}

void PrintScene( FbxScene * pScene ) {
    std::cout << "Scene Name: " << pScene->GetName() << std::endl;
    std::cout << "--------------------------------------" << std::endl;

    FbxNode * pRootNode = pScene->GetRootNode();

    if ( pRootNode ) {
        for ( int i = 0; i < pRootNode->GetChildCount(); i++ ) {
            PrintNode( pRootNode->GetChild( i ) );
        }
    }
}

bool InitializeSdkObjects( FbxManager *& pManager, FbxImporter *& pImporter ) {
    pManager = FbxManager::Create();
    if ( !pManager ) {
        std::cout << "Error: Unable to create FBX Manager!\n";
        return false;
    }

    FbxIOSettings * ios = FbxIOSettings::Create( pManager, IOSROOT );
    pManager->SetIOSettings( ios );

    pImporter = FbxImporter::Create( pManager, "" );
    if ( !pImporter ) {
        std::cout << "Error: Unable to create FBX Importer!\n";
        return false;
    }

    return true;
}

bool LoadFBXFile( const char * filename ) {
    FbxManager * pManager = nullptr;
    FbxImporter * pImporter = nullptr;

    if ( !InitializeSdkObjects( pManager, pImporter ) ) {
        return false;
    }

    if ( !pImporter->Initialize( filename, -1, pManager->GetIOSettings() ) ) {
        std::cout << "Error: Unable to initialize FBX Importer!\n";
        return false;
    }

    // Create a new scene so it can be populated by the imported file
    FbxScene * pScene = FbxScene::Create( pManager, "myScene" );

    // Import the contents of the file into the scene
    bool lStatus = pImporter->Import( pScene );

    // Destroy the importer
    pImporter->Destroy();

    if ( lStatus ) {
        // Print the contents of the scene
        PrintScene( pScene );
    }

    // Destroy the SDK manager and all other objects it was handling
    pManager->Destroy();

    return lStatus;
}