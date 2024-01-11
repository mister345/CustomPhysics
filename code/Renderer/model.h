//
//  model.h
//
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include "DeviceContext.h"
#include "Buffer.h"
#include "../Math/Vector.h"
#include "../Math/Quat.h"

class Shape;

/*
====================================================
Model Interface
====================================================
*/
class ModelBase {
public:
	ModelBase() : m_isVBO( false ) {}
	virtual ~ModelBase() {}
	virtual bool BuildFromShape( const Shape * shape ) = 0;
	virtual bool MakeVBO( DeviceContext * device ) = 0;
	virtual void DrawIndexed( VkCommandBuffer vkCommandBUffer ) = 0;

	// GPU Data
	bool m_isVBO;
	Buffer	m_vertexBuffer;
	Buffer	m_indexBuffer;

	// CPU Data
	std::vector< unsigned int > m_indices;

	void Cleanup( DeviceContext & deviceContext );
};

/*
====================================================
vert_t
// 8 * 4 = 32 bytes - data structure for drawable verts... this should be good for most things
====================================================
*/
struct vert_t {
	float			xyz[ 3 ];	// 12 bytes
	float			st[ 2 ];	// 8 bytes
	unsigned char	norm[ 4 ];	// 4 bytes
	unsigned char	tang[ 4 ];	// 4 bytes
	unsigned char	buff[ 4 ];	// 4 bytes

	static VkVertexInputBindingDescription GetBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof( vert_t );
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array< VkVertexInputAttributeDescription, 5 > GetAttributeDescriptions() {
		std::array< VkVertexInputAttributeDescription, 5 > attributeDescriptions = {};

		attributeDescriptions[ 0 ].binding = 0;
		attributeDescriptions[ 0 ].location = 0;
		attributeDescriptions[ 0 ].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[ 0 ].offset = offsetof( vert_t, xyz );

		attributeDescriptions[ 1 ].binding = 0;
		attributeDescriptions[ 1 ].location = 1;
		attributeDescriptions[ 1 ].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[ 1 ].offset = offsetof( vert_t, st );

		attributeDescriptions[ 2 ].binding = 0;
		attributeDescriptions[ 2 ].location = 2;
		attributeDescriptions[ 2 ].format = VK_FORMAT_R8G8B8A8_UNORM;
		attributeDescriptions[ 2 ].offset = offsetof( vert_t, norm );

		attributeDescriptions[ 3 ].binding = 0;
		attributeDescriptions[ 3 ].location = 3;
		attributeDescriptions[ 3 ].format = VK_FORMAT_R8G8B8A8_UNORM;
		attributeDescriptions[ 3 ].offset = offsetof( vert_t, tang );

		attributeDescriptions[ 4 ].binding = 0;
		attributeDescriptions[ 4 ].location = 4;
		attributeDescriptions[ 4 ].format = VK_FORMAT_R8G8B8A8_UNORM;
		attributeDescriptions[ 4 ].offset = offsetof( vert_t, buff );

		return attributeDescriptions;
	}
};

/*
====================================================
Model
====================================================
*/
class Model : public ModelBase {
public:
	~Model() override {}

	std::vector< vert_t > m_vertices;

	virtual bool BuildFromShape( const Shape * shape ) override;
	virtual bool MakeVBO( DeviceContext * device ) override;
	virtual void DrawIndexed( VkCommandBuffer vkCommandBUffer ) override;
};

/*
====================================================
vertSkinned_t
// 8 * 4 = 32 bytes - data structure for drawable verts... this should be good for most things
====================================================
*/
static constexpr int MAX_BONES_PER_VERTEX = 4;
struct vertSkinned_t {
	float			xyz[ 3 ];	// 12 bytes
	float			st[ 2 ];	// 8 bytes
	unsigned char	norm[ 4 ];	// 4 bytes
	unsigned char	tang[ 4 ];	// 4 bytes
	unsigned char	buff[ 4 ];	// 4 bytes

	// NOTE - this works bc vertSkinned_t is currently identical to vert_t
	// mesh will get shredded as soon as we introduce these variables:
	uint32_t boneIdxes[ MAX_BONES_PER_VERTEX ];
	float boneWeights[ MAX_BONES_PER_VERTEX ];

	// why? bc size of vertSkinned_t will change, and that requires updating the
	// CreateParms_t for the CreatePipeline function - see OffscreenRenderer line 190
	// ( just clone the default vert shader for now, call it "skinning" and create a brand new pipeline for it
	// w the proper VkVertexInputAttributeDescription

	static VkVertexInputBindingDescription GetBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof( vertSkinned_t );
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

//	static std::array< VkVertexInputAttributeDescription, 5 > GetAttributeDescriptions() {
	static std::array< VkVertexInputAttributeDescription, 7 > GetAttributeDescriptions() {
//		std::array< VkVertexInputAttributeDescription, 5 > attributeDescriptions = {};
		std::array< VkVertexInputAttributeDescription, 7 > attributeDescriptions = {};

		attributeDescriptions[ 0 ].binding = 0;
		attributeDescriptions[ 0 ].location = 0;
		attributeDescriptions[ 0 ].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[ 0 ].offset = offsetof( vertSkinned_t, xyz );

		attributeDescriptions[ 1 ].binding = 0;
		attributeDescriptions[ 1 ].location = 1;
		attributeDescriptions[ 1 ].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[ 1 ].offset = offsetof( vertSkinned_t, st );

		attributeDescriptions[ 2 ].binding = 0;
		attributeDescriptions[ 2 ].location = 2;
		attributeDescriptions[ 2 ].format = VK_FORMAT_R8G8B8A8_UNORM;
		attributeDescriptions[ 2 ].offset = offsetof( vertSkinned_t, norm );

		attributeDescriptions[ 3 ].binding = 0;
		attributeDescriptions[ 3 ].location = 3;
		attributeDescriptions[ 3 ].format = VK_FORMAT_R8G8B8A8_UNORM;
		attributeDescriptions[ 3 ].offset = offsetof( vertSkinned_t, tang );

		attributeDescriptions[ 4 ].binding = 0;
		attributeDescriptions[ 4 ].location = 4;
		attributeDescriptions[ 4 ].format = VK_FORMAT_R8G8B8A8_UNORM;
		attributeDescriptions[ 4 ].offset = offsetof( vertSkinned_t, buff );

//		uint32_t boneIdxes[ MAX_BONES_PER_VERTEX ];
		attributeDescriptions[ 5 ].binding = 0;
		attributeDescriptions[ 5 ].location = 5;
		attributeDescriptions[ 5 ].format = VK_FORMAT_R32G32B32A32_UINT;
		attributeDescriptions[ 5 ].offset = offsetof( vertSkinned_t, boneIdxes );

//		float boneWeights[ MAX_BONES_PER_VERTEX ];
		attributeDescriptions[ 6 ].binding = 0;
		attributeDescriptions[ 6 ].location = 6;
		attributeDescriptions[ 6 ].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[ 6 ].offset = offsetof( vertSkinned_t, boneWeights );

		return attributeDescriptions;
	}
};

/*
====================================================
ModelSkinned
====================================================
*/
class ModelSkinned : public ModelBase {
public:
	~ModelSkinned() override {}

	std::vector< vertSkinned_t > m_skinnedVerts;
	Buffer m_boneMatrixBuffer; // @TODO - allocate and populate w matrix palette every frame

	virtual bool BuildFromShape( const Shape * shape ) override;
	virtual bool MakeVBO( DeviceContext * device ) override;
	virtual void DrawIndexed( VkCommandBuffer vkCommandBUffer ) override;
};

void FillCube( Model & model );
void FillFullScreenQuad( Model & model );


struct RenderModel {
	ModelBase * model;		// The vao buffer to draw
	uint32_t uboByteOffset;	// The byte offset into the uniform buffer
	uint32_t uboByteSize;	// how much space we consume in the uniform buffer

	Vec3 pos;
	Quat orient;
	int numBones = 0;
};