//
//  model.cpp
//
#include "model.h"
#include "../Math/Vector.h"
#include "../Fileio.h"
#include <string.h>
#include "../Physics/Shapes.h"
#include "../Physics/Shapes/ShapeLoadedMesh.h"
#include <algorithm>


#pragma warning( disable : 4996 )

/*
====================================================
FloatToByte
// Assumes a float between [-1,1]
====================================================
*/
unsigned char FloatToByte_n11( const float f ) {
	int i = (int)(f * 127 + 128);
	return (unsigned char)i;
}
/*
====================================================
FloatToByte
// Assumes a float between [0,1]
====================================================
*/
unsigned char FloatToByte_01( const float f ) {
	int i = (int)(f * 255);
	return (unsigned char)i;
}

Vec3 Byte4ToVec3( const unsigned char * data ) {
	Vec3 pt;
	pt.x = float( data[ 0 ] ) / 255.0f;	// 0,1
	pt.y = float( data[ 1 ] ) / 255.0f;	// 0,1
	pt.z = float( data[ 2 ] ) / 255.0f;	// 0,1

	pt.x = 2.0f * ( pt.x - 0.5f ); //-1,1
	pt.y = 2.0f * ( pt.y - 0.5f ); //-1,1
	pt.z = 2.0f * ( pt.z - 0.5f ); //-1,1
	return pt;
}

void Vec3ToFloat3( const Vec3 & v, float * f ) {
	f[ 0 ] = v.x;
	f[ 1 ] = v.y;
	f[ 2 ] = v.z;
}
void Vec2ToFloat2( const Vec2 & v, float * f ) {
	f[ 0 ] = v.x;
	f[ 1 ] = v.y;
}
void Vec3ToByte4( const Vec3 & v, unsigned char * b ) {
	Vec3 tmp = v;
	tmp.Normalize();
	b[ 0 ] = FloatToByte_n11( tmp.x );
	b[ 1 ] = FloatToByte_n11( tmp.y );
	b[ 2 ] = FloatToByte_n11( tmp.z );
	b[ 3 ] = 0;
}

/*
====================================================
FillFullScreenQuad
====================================================
*/
void FillFullScreenQuad( Model & model ) {
	const int numVerts = 4;
	const int numIdxs = 6;
	vert_t	screenVerts[ numVerts ];
	int		screenIndices[ numIdxs ];

	memset( screenVerts, 0, sizeof( vert_t ) * 4 );

	screenVerts[ 0 ].xyz[ 0 ] = -1.0f;
	screenVerts[ 0 ].xyz[ 1 ] = -1.0f;
	screenVerts[ 0 ].xyz[ 2 ] = 0.0f;

	screenVerts[ 1 ].xyz[ 0 ] = 1.0f;
	screenVerts[ 1 ].xyz[ 1 ] = -1.0f;
	screenVerts[ 1 ].xyz[ 2 ] = 0.0f;

	screenVerts[ 2 ].xyz[ 0 ] = 1.0f;
	screenVerts[ 2 ].xyz[ 1 ] = 1.0f;
	screenVerts[ 2 ].xyz[ 2 ] = 0.0f;

	screenVerts[ 3 ].xyz[ 0 ] = -1.0f;
	screenVerts[ 3 ].xyz[ 1 ] = 1.0f;
	screenVerts[ 3 ].xyz[ 2 ] = 0.0f;


	screenVerts[ 0 ].st[ 0 ] = 0.0f;
	screenVerts[ 0 ].st[ 1 ] = 1.0f;

	screenVerts[ 1 ].st[ 0 ] = 1.0f;
	screenVerts[ 1 ].st[ 1 ] = 1.0f;

	screenVerts[ 2 ].st[ 0 ] = 1.0f;
	screenVerts[ 2 ].st[ 1 ] = 0.0f;

	screenVerts[ 3 ].st[ 0 ] = 0.0f;
	screenVerts[ 3 ].st[ 1 ] = 0.0f;

	screenVerts[ 0 ].buff[ 0 ] = 255;
	screenVerts[ 1 ].buff[ 0 ] = 255;
	screenVerts[ 2 ].buff[ 0 ] = 255;
	screenVerts[ 3 ].buff[ 0 ] = 255;


	screenIndices[ 0 ] = 0;
	screenIndices[ 1 ] = 1;
	screenIndices[ 2 ] = 2;

	screenIndices[ 3 ] = 0;
	screenIndices[ 4 ] = 2;
	screenIndices[ 5 ] = 3;

	for ( int i = 0; i < numVerts; i++ ) {
		model.m_vertices.push_back( screenVerts[ i ] );
	}

	for ( int i = 0; i < numIdxs; i++ ) {
		model.m_indices.push_back( screenIndices[ i ] );
	}
}

/*
====================================================
FillCube
====================================================
*/
void FillCube( Model & model ) {
	const int numIdxs = 3 * 2 * 6;
	const int numVerts = 4 * 6;
	vert_t	cubeVerts[ numVerts ];
	int		cubeIdxs[ numIdxs ];

	memset( cubeVerts, 0, sizeof( vert_t ) * 4 * 6 );

	for ( int face = 0; face < 6; face++ ) {
		const int dim0 = face / 2;
		const int dim1 = ( dim0 + 1 ) % 3;
		const int dim2 = ( dim0 + 2 ) % 3;
		const float val = ( ( face & 1 ) == 0 ) ? -1.0f : 1.0f;

		cubeVerts[ face * 4 + 0 ].xyz[ dim0 ] = val;
		cubeVerts[ face * 4 + 0 ].xyz[ dim1 ] = val;
		cubeVerts[ face * 4 + 0 ].xyz[ dim2 ] = val;

		cubeVerts[ face * 4 + 1 ].xyz[ dim0 ] = val;
		cubeVerts[ face * 4 + 1 ].xyz[ dim1 ] = -val;
		cubeVerts[ face * 4 + 1 ].xyz[ dim2 ] = val;

		cubeVerts[ face * 4 + 2 ].xyz[ dim0 ] = val;
		cubeVerts[ face * 4 + 2 ].xyz[ dim1 ] = -val;
		cubeVerts[ face * 4 + 2 ].xyz[ dim2 ] = -val;

		cubeVerts[ face * 4 + 3 ].xyz[ dim0 ] = val;
		cubeVerts[ face * 4 + 3 ].xyz[ dim1 ] = val;
		cubeVerts[ face * 4 + 3 ].xyz[ dim2 ] = -val;


		cubeVerts[ face * 4 + 0 ].st[ 0 ] = 0.0f;
		cubeVerts[ face * 4 + 0 ].st[ 1 ] = 1.0f;

		cubeVerts[ face * 4 + 1 ].st[ 0 ] = 1.0f;
		cubeVerts[ face * 4 + 1 ].st[ 1 ] = 1.0f;

		cubeVerts[ face * 4 + 2 ].st[ 0 ] = 1.0f;
		cubeVerts[ face * 4 + 2 ].st[ 1 ] = 0.0f;

		cubeVerts[ face * 4 + 3 ].st[ 0 ] = 0.0f;
		cubeVerts[ face * 4 + 3 ].st[ 1 ] = 0.0f;


		cubeVerts[ face * 4 + 0 ].norm[ dim0 ] = FloatToByte_n11( val );
		cubeVerts[ face * 4 + 1 ].norm[ dim0 ] = FloatToByte_n11( val );
		cubeVerts[ face * 4 + 2 ].norm[ dim0 ] = FloatToByte_n11( val );
		cubeVerts[ face * 4 + 3 ].norm[ dim0 ] = FloatToByte_n11( val );


		cubeVerts[ face * 4 + 0 ].tang[ dim1 ] = FloatToByte_n11( val );
		cubeVerts[ face * 4 + 1 ].tang[ dim1 ] = FloatToByte_n11( val );
		cubeVerts[ face * 4 + 2 ].tang[ dim1 ] = FloatToByte_n11( val );
		cubeVerts[ face * 4 + 3 ].tang[ dim1 ] = FloatToByte_n11( val );


		cubeIdxs[ face * 6 + 0 ] = face * 4 + 0;
		cubeIdxs[ face * 6 + 1 ] = face * 4 + 1;
		cubeIdxs[ face * 6 + 2 ] = face * 4 + 2;

		cubeIdxs[ face * 6 + 3 ] = face * 4 + 0;
		cubeIdxs[ face * 6 + 4 ] = face * 4 + 2;
		cubeIdxs[ face * 6 + 5 ] = face * 4 + 3;
	}

	for ( int i = 0; i < numVerts; i++ ) {
		model.m_vertices.push_back( cubeVerts[ i ] );
	}

	for ( int i = 0; i < numIdxs; i++ ) {
		model.m_indices.push_back( cubeIdxs[ i ] );
	}
}

/*
====================================================
FillCubeTessellated
====================================================
*/
void FillCubeTessellated( Model & model, int numDivisions ) {
	if ( numDivisions < 1 ) {
		numDivisions = 1;
	}

	const int numnum = ( numDivisions + 1 ) * ( numDivisions + 1 );
	Vec3 * v = (Vec3 *)malloc( numnum * sizeof( Vec3 ) );
	Vec2 * st = (Vec2 *)malloc( numnum * sizeof( Vec2 ) );
	for ( int y = 0; y < numDivisions + 1; y++ ) {
		for ( int x = 0; x < numDivisions + 1; x++ ) {
			float xf = ( ( (float)x / (float)numDivisions ) - 0.5f ) * 2.0f;
			float yf = ( ( (float)y / (float)numDivisions ) - 0.5f ) * 2.0f;
			v[ y * ( numDivisions + 1 ) + x ] = Vec3( xf, yf, 1.0f );

			float sf = (float)x / (float)numDivisions;
			float tf = (float)y / (float)numDivisions;
			st[ y * ( numDivisions + 1 ) + x ] = Vec2( sf, tf );
		}
	}

	const int numFaces = numDivisions * numDivisions;

	int faceIdx = 0;
	int * faceIdxs = (int *)malloc( 3 * 2 * numFaces * sizeof( int ) );
	for ( int y = 0; y < numDivisions; y++ ) {
		for ( int x = 0; x < numDivisions; x++ ) {
			int y0 = y;
			int y1 = y + 1;
			int x0 = x;
			int x1 = x + 1;

			faceIdxs[ faceIdx * 6 + 1 ] = y0 * ( numDivisions + 1 ) + x0;
			faceIdxs[ faceIdx * 6 + 0 ] = y1 * ( numDivisions + 1 ) + x0;
			faceIdxs[ faceIdx * 6 + 2 ] = y1 * ( numDivisions + 1 ) + x1;

			faceIdxs[ faceIdx * 6 + 4 ] = y0 * ( numDivisions + 1 ) + x0;
			faceIdxs[ faceIdx * 6 + 3 ] = y1 * ( numDivisions + 1 ) + x1;
			faceIdxs[ faceIdx * 6 + 5 ] = y0 * ( numDivisions + 1 ) + x1;

			faceIdx++;
		}
	}

	Mat3 matOrients[ 6 ];
	for ( int i = 0; i < 6; i++ ) {
		matOrients[ i ].Identity();
	}
	// px
	matOrients[ 0 ].rows[ 0 ] = Vec3( 0.0f, 0.0f, 1.0f );
	matOrients[ 0 ].rows[ 1 ] = Vec3( 1.0f, 0.0f, 0.0f );
	matOrients[ 0 ].rows[ 2 ] = Vec3( 0.0f, 1.0f, 0.0f );
	// nx
	matOrients[ 1 ].rows[ 0 ] = Vec3( 0.0f, 0.0f,-1.0f );
	matOrients[ 1 ].rows[ 1 ] = Vec3(-1.0f, 0.0f, 0.0f );
	matOrients[ 1 ].rows[ 2 ] = Vec3( 0.0f, 1.0f, 0.0f );

	// py
	matOrients[ 2 ].rows[ 0 ] = Vec3( 1.0f, 0.0f, 0.0f );
	matOrients[ 2 ].rows[ 1 ] = Vec3( 0.0f, 0.0f, 1.0f );
	matOrients[ 2 ].rows[ 2 ] = Vec3( 0.0f,-1.0f, 0.0f );
	// ny
	matOrients[ 3 ].rows[ 0 ] = Vec3( 1.0f, 0.0f, 0.0f );
	matOrients[ 3 ].rows[ 1 ] = Vec3( 0.0f, 0.0f,-1.0f );
	matOrients[ 3 ].rows[ 2 ] = Vec3( 0.0f, 1.0f, 0.0f );

	// pz
	matOrients[ 4 ].rows[ 0 ] = Vec3( 1.0f, 0.0f, 0.0f );
	matOrients[ 4 ].rows[ 1 ] = Vec3( 0.0f, 1.0f, 0.0f );
	matOrients[ 4 ].rows[ 2 ] = Vec3( 0.0f, 0.0f, 1.0f );
	// nz
	matOrients[ 5 ].rows[ 0 ] = Vec3(-1.0f, 0.0f, 0.0f );
	matOrients[ 5 ].rows[ 1 ] = Vec3( 0.0f, 1.0f, 0.0f );
	matOrients[ 5 ].rows[ 2 ] = Vec3( 0.0f, 0.0f,-1.0f );

	const int numIdxs = 3 * 2 * 6 * numFaces;
	const int numVerts = 4 * 6 * numFaces;
	vert_t *	cubeVerts = (vert_t *)malloc( numVerts * sizeof( vert_t ) );
	int *		cubeIdxs = (int *)malloc( numIdxs * sizeof( int ) );

	memset( cubeVerts, 0, sizeof( vert_t ) * numVerts );

	for ( int side = 0; side < 6; side++ ) {
		const Mat3 & mat = matOrients[ side ];

		const Vec3 tang = mat * Vec3( 1.0f, 0.0f, 0.0f );
		const Vec3 norm = mat * Vec3( 0.0f, 0.0f, 1.0f );

		for ( int vid = 0; vid < numnum; vid++ ) {
			const Vec3 xyz	= mat * v[ vid ];
			const Vec2 uv	= st[ vid ];

			cubeVerts[ side * numnum + vid ].xyz[ 0 ] = xyz[ 0 ];
			cubeVerts[ side * numnum + vid ].xyz[ 1 ] = xyz[ 1 ];
			cubeVerts[ side * numnum + vid ].xyz[ 2 ] = xyz[ 2 ];

			cubeVerts[ side * numnum + vid ].st[ 0 ] = uv[ 0 ];
			cubeVerts[ side * numnum + vid ].st[ 1 ] = uv[ 1 ];

			cubeVerts[ side * numnum + vid ].norm[ 0 ] = FloatToByte_n11( norm[ 0 ] );
			cubeVerts[ side * numnum + vid ].norm[ 1 ] = FloatToByte_n11( norm[ 1 ] );
			cubeVerts[ side * numnum + vid ].norm[ 2 ] = FloatToByte_n11( norm[ 2 ] );
			cubeVerts[ side * numnum + vid ].norm[ 3 ] = FloatToByte_n11( 0.0f );

			cubeVerts[ side * numnum + vid ].tang[ 0 ] = FloatToByte_n11( tang[ 0 ] );
			cubeVerts[ side * numnum + vid ].tang[ 1 ] = FloatToByte_n11( tang[ 1 ] );
			cubeVerts[ side * numnum + vid ].tang[ 2 ] = FloatToByte_n11( tang[ 2 ] );
			cubeVerts[ side * numnum + vid ].tang[ 3 ] = FloatToByte_n11( 0.0f );
		}

		for ( int idx = 0; idx < 3 * 2 * numFaces; idx++ ) {
			const int offset = 3 * 2 * numFaces * side;
			cubeIdxs[ idx + offset ] = faceIdxs[ idx ] + numnum * side;
		}
	}

	for ( int i = 0; i < numVerts; i++ ) {
		model.m_vertices.push_back( cubeVerts[ i ] );
	}

	for ( int i = 0; i < numIdxs; i++ ) {
		model.m_indices.push_back( cubeIdxs[ i ] );
	}

	free( v );
	free( st );
	free( faceIdxs );
	free( cubeVerts );
	free( cubeIdxs );
}

/*
====================================================
FillSphere
====================================================
*/
void FillSphere( Model & model, const float radius ) {
	float t = radius;
	if ( t < 0.0f ) {
		t = 0.0f;
	}
	if ( t > 100.0f ) {
		t = 100.0f;
	}
	t /= 100.0f;
	float min = 5;
	float max = 30;
	float s = min * ( 1.0f - t ) + max * t;
	FillCubeTessellated( model, (int)s );

	// Project the tessellated cube onto a sphere
	for ( int i = 0; i < model.m_vertices.size(); i++ ) {
		Vec3 xyz = model.m_vertices[ i ].xyz;
		xyz.Normalize();

		model.m_vertices[ i ].xyz[ 0 ] = xyz[ 0 ];
		model.m_vertices[ i ].xyz[ 1 ] = xyz[ 1 ];
		model.m_vertices[ i ].xyz[ 2 ] = xyz[ 2 ];

		model.m_vertices[ i ].norm[ 0 ] = FloatToByte_n11( xyz[ 0 ] );
		model.m_vertices[ i ].norm[ 1 ] = FloatToByte_n11( xyz[ 1 ] );
		model.m_vertices[ i ].norm[ 2 ] = FloatToByte_n11( xyz[ 2 ] );
		model.m_vertices[ i ].norm[ 3 ] = FloatToByte_n11( 0.0f );

		Vec3 tang = Byte4ToVec3( model.m_vertices[ i ].norm );
		Vec3 bitang = xyz.Cross( tang );
		bitang.Normalize();
		tang = bitang.Cross( xyz );

		model.m_vertices[ i ].tang[ 0 ] = FloatToByte_n11( tang[ 0 ] );
		model.m_vertices[ i ].tang[ 1 ] = FloatToByte_n11( tang[ 1 ] );
		model.m_vertices[ i ].tang[ 2 ] = FloatToByte_n11( tang[ 2 ] );
		model.m_vertices[ i ].tang[ 3 ] = FloatToByte_n11( 0.0f );
	}
}

/*
================================
ModelBase::Cleanup
================================
*/
void ModelBase::Cleanup( DeviceContext & deviceContext ) {
	if ( !m_isVBO ) {
		return;
	}

	m_vertexBuffer.Cleanup( &deviceContext );
	m_indexBuffer.Cleanup( &deviceContext );
}

/*
====================================================
Model::BuildFromShape
====================================================
*/
bool Model::BuildFromShape( const Shape * shape ) {
	if ( NULL == shape ) {
		return false;
	}

	const ShapeSphere * shapeSphere = ( const ShapeSphere * )shape;
	m_vertices.clear();
	m_indices.clear();
	FillSphere( *this, shapeSphere->m_radius );
	for ( int v = 0; v < m_vertices.size(); v++ ) {
		for ( int i = 0; i < 3; i++ ) {
			m_vertices[ v ].xyz[ i ] *= shapeSphere->m_radius;
		}
	}

	return true;
}

/*
================================
Model::MakeVBO
================================
*/
bool Model::MakeVBO( DeviceContext * device ) {
	VkCommandBuffer vkCommandBuffer = device->m_vkCommandBuffers[ 0 ];

	int bufferSize;

	// Create Vertex Buffer
	bufferSize = (int)( sizeof( m_vertices[ 0 ] ) * m_vertices.size() );
	if ( !m_vertexBuffer.Allocate( device, m_vertices.data(), bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ) ) {
		printf( "failed to allocate vertex buffer!\n" );
		assert( 0 );
		return false;
	}

	// Create Index Buffer
	bufferSize = (int)( sizeof( m_indices[ 0 ] ) * m_indices.size() );
	if ( !m_indexBuffer.Allocate( device, m_indices.data(), bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT ) ) {
		printf( "failed to allocate index buffer!\n" );
		assert( 0 );
		return false;
	}

	m_isVBO = true;
	return true;
}

/*
====================================================
Model::DrawIndexed
====================================================
*/
void Model::DrawIndexed( VkCommandBuffer vkCommandBUffer ) {
	// Bind the model
	VkBuffer vertexBuffers[] = { m_vertexBuffer.m_vkBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers( vkCommandBUffer, 0, 1, vertexBuffers, offsets );
	vkCmdBindIndexBuffer( vkCommandBUffer, m_indexBuffer.m_vkBuffer, 0, VK_INDEX_TYPE_UINT32 );

	// Issue draw command
	vkCmdDrawIndexed( vkCommandBUffer, (uint32_t)m_indices.size(), 1, 0, 0, 0 );
}


/*
====================================================
ModelSkinned::BuildFromShape
====================================================
*/
bool ModelSkinned::BuildFromShape( const Shape * shape ) {
	if ( NULL == shape ) {
		return false;
	}

	assert( shape->GetType() == Shape::SHAPE_LOADED_MESH );

	const ShapeLoadedMesh * shapeMesh = reinterpret_cast< const ShapeLoadedMesh * >( shape );
	m_skinnedVerts.clear();
	m_indices.clear();
	// @TODO - i dont like copying all this data here, refactor the architecture later to do it in place here
	m_skinnedVerts.assign( shapeMesh->verts, shapeMesh->verts + shapeMesh->numVerts );
	m_indices.assign( shapeMesh->idxes, shapeMesh->idxes + shapeMesh->numIdxes );

	return true;
}


/*
================================
ModelSkinned::MakeVBO
================================
*/
bool ModelSkinned::MakeVBO( DeviceContext * device ) {
	VkCommandBuffer vkCommandBuffer = device->m_vkCommandBuffers[ 0 ];

	int bufferSize;

	// Create Vertex Buffer
	bufferSize = ( int )( sizeof( m_skinnedVerts[ 0 ] ) * m_skinnedVerts.size() );
	if ( !m_vertexBuffer.Allocate( device, m_skinnedVerts.data(), bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ) ) {
		printf( "failed to allocate vertex buffer!\n" );
		assert( 0 );
		return false;
	}

	// Create Index Buffer
	bufferSize = ( int )( sizeof( m_indices[ 0 ] ) * m_indices.size() );
	if ( !m_indexBuffer.Allocate( device, m_indices.data(), bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT ) ) {
		printf( "failed to allocate index buffer!\n" );
		assert( 0 );
		return false;
	}

	m_isVBO = true;
	return true;
}

/*
====================================================
ModelSkinned::DrawIndexed
====================================================
*/
void ModelSkinned::DrawIndexed( VkCommandBuffer vkCommandBUffer ) {
	// Bind the model
	VkBuffer vertexBuffers[ ] = { m_vertexBuffer.m_vkBuffer };
	VkDeviceSize offsets[ ] = { 0 };
	vkCmdBindVertexBuffers( vkCommandBUffer, 0, 1, vertexBuffers, offsets );
	vkCmdBindIndexBuffer( vkCommandBUffer, m_indexBuffer.m_vkBuffer, 0, VK_INDEX_TYPE_UINT32 );

	// Issue draw command
	vkCmdDrawIndexed( vkCommandBUffer, ( uint32_t )m_indices.size(), 1, 0, 0, 0 );
}