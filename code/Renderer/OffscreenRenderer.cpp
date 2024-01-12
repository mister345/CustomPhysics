//
//  OffscreenRenderer.cpp
//
#include "OffscreenRenderer.h"
#include "model.h"
#include "Samplers.h"

#include "../application.h"
#include <assert.h>
#include <stdio.h>
#include <vector>

FrameBuffer	g_offscreenFrameBuffer;
Pipeline	g_skyPipeline;
Shader		g_skyShader;
Descriptors	g_skyDescriptors;
Model		g_skyModel;

Pipeline	g_checkerboardShadowPipeline;
Shader		g_checkerboardShadowShader;
Descriptors	g_checkerboardShadowDescriptors;

FrameBuffer	g_shadowFrameBuffer;
Pipeline	g_shadowPipeline;
Shader		g_shadowShader;
Descriptors	g_shadowDescriptors;

// @TODO - actually create these
Pipeline	g_skinningPipeline;
Shader		g_skinningShader;
Descriptors	g_skinningDescriptors;

/*
====================================================
InitOffscreen
====================================================
*/
bool InitOffscreen( DeviceContext * device, int width, int height ) {
	bool result;

	//
	//	Build the frame buffer to render into
	//
	{
		FrameBuffer::CreateParms_t frameBufferParms;
		frameBufferParms.width = width;
		frameBufferParms.height = height;
		frameBufferParms.hasColor = true;
		frameBufferParms.hasDepth = true;
		result = g_offscreenFrameBuffer.Create( device, frameBufferParms );
		if ( !result ) {
			printf( "ERROR: Failed to create off screen buffer\n" );
			assert( 0 );
			return false;
		}
	}

	//
	//	Shadow
	//
	{
		FrameBuffer::CreateParms_t frameBufferParms;
		frameBufferParms.width = 4096;
		frameBufferParms.height = 4096;
		frameBufferParms.hasColor = false;
		frameBufferParms.hasDepth = true;
		result = g_shadowFrameBuffer.Create( device, frameBufferParms );
		if ( !result ) {
			printf( "ERROR: Failed to create off screen buffer\n" );
			assert( 0 );
			return false;
		}

		result = g_shadowShader.Load( device, "shadow2" );
		if ( !result ) {
			printf( "ERROR: Failed to load shader\n" );
			assert( 0 );
			return false;
		}

		Descriptors::CreateParms_t descriptorParms;
		memset( &descriptorParms, 0, sizeof( descriptorParms ) );
		descriptorParms.numUniformsVertex = 2;
		result = g_shadowDescriptors.Create( device, descriptorParms );
		if ( !result ) {
			printf( "ERROR: Failed to build descriptors\n" );
			assert( 0 );
			return false;
		}

		Pipeline::CreateParms_t pipelineParms;
		pipelineParms.framebuffer = &g_shadowFrameBuffer;
		pipelineParms.descriptors = &g_shadowDescriptors;
		pipelineParms.shader = &g_shadowShader;
		pipelineParms.width = frameBufferParms.width;
		pipelineParms.height = frameBufferParms.height;
		pipelineParms.cullMode = Pipeline::CULL_MODE_FRONT;
		pipelineParms.depthTest = true;
		pipelineParms.depthWrite = true;
		result = g_shadowPipeline.Create( device, pipelineParms );
		if ( !result ) {
			printf( "ERROR: Failed to build pipeline\n" );
			assert( 0 );
			return false;
		}
	}

	//
	//	Sky
	//
	{
		result = g_skyShader.Load( device, "sky" );
		if ( !result ) {
			printf( "ERROR: Failed to load shader\n" );
			assert( 0 );
			return false;
		}

		Descriptors::CreateParms_t descriptorParms;
		memset( &descriptorParms, 0, sizeof( descriptorParms ) );
		descriptorParms.numUniformsVertex = 1;
		result = g_skyDescriptors.Create( device, descriptorParms );
		if ( !result ) {
			printf( "ERROR: Failed to build descriptors\n" );
			assert( 0 );
			return false;
		}

		Pipeline::CreateParms_t pipelineParms;
		pipelineParms.framebuffer = &g_offscreenFrameBuffer;
		pipelineParms.descriptors = &g_skyDescriptors;
		pipelineParms.shader = &g_skyShader;
		pipelineParms.width = g_offscreenFrameBuffer.m_parms.width;
		pipelineParms.height = g_offscreenFrameBuffer.m_parms.height;
		pipelineParms.cullMode = Pipeline::CULL_MODE_NONE;
		pipelineParms.depthTest = false;
		pipelineParms.depthWrite = false;
		result = g_skyPipeline.Create( device, pipelineParms );
		if ( !result ) {
			printf( "ERROR: Failed to build pipeline\n" );
			assert( 0 );
			return false;
		}

		ShapeSphere sphereShape( 1.0f );
		g_skyModel.BuildFromShape( &sphereShape );
		g_skyModel.MakeVBO( device );
	}

	//
	//	CheckerBoard Shadow
	//
	{
		result = g_checkerboardShadowShader.Load( device, "checkerboardShadowed2" );
		if ( !result ) {
			printf( "ERROR: Failed to load shader\n" );
			assert( 0 );
			return false;
		}

		Descriptors::CreateParms_t descriptorParms;
		memset( &descriptorParms, 0, sizeof( descriptorParms ) );
		descriptorParms.numUniformsVertex = 3;
		descriptorParms.numUniformsFragment = 1;
		descriptorParms.numImageSamplers = 1;
		result = g_checkerboardShadowDescriptors.Create( device, descriptorParms );
		if ( !result ) {
			printf( "ERROR: Failed to build descriptors\n" );
			assert( 0 );
			return false;
		}

		Pipeline::CreateParms_t pipelineParms;
		pipelineParms.framebuffer = &g_offscreenFrameBuffer;
		pipelineParms.descriptors = &g_checkerboardShadowDescriptors;
		pipelineParms.shader = &g_checkerboardShadowShader;
		pipelineParms.width = g_offscreenFrameBuffer.m_parms.width;
		pipelineParms.height = g_offscreenFrameBuffer.m_parms.height;
		pipelineParms.cullMode = Pipeline::CULL_MODE_BACK;
		pipelineParms.depthTest = true;
		pipelineParms.depthWrite = true;
		result = g_checkerboardShadowPipeline.Create( device, pipelineParms );
		if ( !result ) {
			printf( "ERROR: Failed to build pipeline\n" );
			assert( 0 );
			return false;
		}
	}

	//
	//	Skinning Shader
	//
	{
		result = g_skinningShader.Load( device, "skinnedMesh" );
		if ( !result ) {
			printf( "ERROR: Failed to load shader\n" );
			assert( 0 );
			return false;
		}

		Descriptors::CreateParms_t descriptorParms;
		memset( &descriptorParms, 0, sizeof( descriptorParms ) );
//		descriptorParms.numUniformsVertex = 3;
		descriptorParms.numUniformsVertex = 4;
		descriptorParms.numUniformsFragment = 1;
		descriptorParms.numImageSamplers = 1;
		result = g_skinningDescriptors.Create( device, descriptorParms );
		if ( !result ) {
			printf( "ERROR: Failed to build descriptors\n" );
			assert( 0 );
			return false;
		}

		Pipeline::CreateParms_t pipelineParms;
		pipelineParms.framebuffer = &g_offscreenFrameBuffer;
		pipelineParms.descriptors = &g_skinningDescriptors;
		pipelineParms.shader = &g_skinningShader;
		pipelineParms.width = g_offscreenFrameBuffer.m_parms.width;
		pipelineParms.height = g_offscreenFrameBuffer.m_parms.height;
		pipelineParms.cullMode = Pipeline::CULL_MODE_BACK;
		pipelineParms.depthTest = true;
		pipelineParms.depthWrite = true;
		pipelineParms.isSkinned = true;

		result = g_skinningPipeline.Create( device, pipelineParms );
		if ( !result ) {
			printf( "ERROR: Failed to build pipeline\n" );
			assert( 0 );
			return false;
		}
	}

	return true;
}

/*
====================================================
CleanupOffscreen
====================================================
*/
bool CleanupOffscreen( DeviceContext * device ) {
	g_skinningPipeline.Cleanup( device );
	g_skinningDescriptors.Cleanup( device );
	g_skinningShader.Cleanup( device );

	g_skyPipeline.Cleanup( device );
	g_skyDescriptors.Cleanup( device );
	g_skyShader.Cleanup( device );
	g_offscreenFrameBuffer.Cleanup( device );
	g_skyModel.Cleanup( *device );

	g_checkerboardShadowPipeline.Cleanup( device );
	g_checkerboardShadowShader.Cleanup( device );
	g_checkerboardShadowDescriptors.Cleanup( device );
	
	g_shadowPipeline.Cleanup( device );
	g_shadowShader.Cleanup( device );
	g_shadowDescriptors.Cleanup( device );
	g_shadowFrameBuffer.Cleanup( device );
	return true;
}

/*
====================================================
DrawOffscreen
====================================================
*/
void DrawOffscreen( DeviceContext * device, int cmdBufferIndex, Buffer * uniforms, const RenderModel * renderModels, const int numModels ) {
	VkCommandBuffer cmdBuffer = device->m_vkCommandBuffers[ cmdBufferIndex ];

	const int camOffset = 0;
	const int camSize = sizeof( float ) * 16 * 4;

	const int shadowCamOffset = device->GetAligendUniformByteOffset( camOffset + camSize );
	const int shadowCamSize = camSize;

	//
	//	Update the Shadows
	//
	{
		g_shadowFrameBuffer.m_imageDepth.TransitionLayout( cmdBuffer, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL );

		g_shadowFrameBuffer.BeginRenderPass( device, cmdBufferIndex );

		// Binding the pipeline is effectively the "use shader" we had back in our opengl apps
		g_shadowPipeline.BindPipeline( cmdBuffer );
		for ( int i = 0; i < numModels; i++ ) {
			const RenderModel & renderModel = renderModels[ i ];

			// @TODO - for now, skinned object dont cast shadows
			// later, save the deformed verts somehwere on gpu, 
			// then reuse them from both the camera and light's perspective
			if ( renderModel.numBones > 0 ) {
				continue;
			}

			// Descriptor is how we bind our buffers and images
			Descriptor descriptor = g_shadowPipeline.GetFreeDescriptor();
			descriptor.BindBuffer( uniforms, shadowCamOffset, shadowCamSize, 0 );						// bind the camera matrices
			descriptor.BindBuffer( uniforms, renderModel.uboByteOffset, renderModel.uboByteSize, 1 );	// bind the model matrices
			descriptor.BindDescriptor( device, cmdBuffer, &g_shadowPipeline );
			renderModel.model->DrawIndexed( cmdBuffer );
		}

		g_shadowFrameBuffer.EndRenderPass( device, cmdBufferIndex );

		g_shadowFrameBuffer.m_imageDepth.TransitionLayout( cmdBuffer, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL );
	}

	//
	//	Draw the World
	//
	{
		g_offscreenFrameBuffer.m_imageColor.TransitionLayout( cmdBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL );

		g_offscreenFrameBuffer.BeginRenderPass( device, cmdBufferIndex );

		//
		//	Draw the sky
		//
		{
			// Binding the pipeline is effectively the "use shader" we had back in our opengl apps
			g_skyPipeline.BindPipeline( cmdBuffer );
	
			// Descriptor is how we bind our buffers and images
			Descriptor descriptor = g_skyPipeline.GetFreeDescriptor();
			descriptor.BindBuffer( uniforms, camOffset, camSize, 0 );
			descriptor.BindDescriptor( device, cmdBuffer, &g_skyPipeline );
			g_skyModel.DrawIndexed( cmdBuffer );
		}
	
		//
		//	Draw the models
		//
		{
			// Binding the pipeline is effectively the "use shader" we had back in our opengl apps
			g_checkerboardShadowPipeline.BindPipeline( cmdBuffer );
			for ( int i = 0; i < numModels; i++ ) {
				const RenderModel & renderModel = renderModels[ i ];

				if ( renderModel.numBones > 0 ) {
					continue;
				}

				// Descriptor is how we bind our buffers and images
				Descriptor descriptor = g_checkerboardShadowPipeline.GetFreeDescriptor();
				descriptor.BindBuffer( uniforms, camOffset, camSize, 0 );									// bind the camera matrices
				descriptor.BindBuffer( uniforms, renderModel.uboByteOffset, renderModel.uboByteSize, 1 );	// bind the model matrices
				descriptor.BindBuffer( uniforms, shadowCamOffset, shadowCamSize, 2 );						// bind the shadow camera matrices
				descriptor.BindImage( VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, g_shadowFrameBuffer.m_imageDepth.m_vkImageView, Samplers::m_samplerStandard, 0 );
				descriptor.BindDescriptor( device, cmdBuffer, &g_checkerboardShadowPipeline );
				renderModel.model->DrawIndexed( cmdBuffer );
			}
		}

		// @TODO - super hacky way for now, proceeding w MINIMAL incremental changes
		// eventually want to maintain two vectors of RenderModel for skinned and unskinned
		// ( or can we ensure all the skinned ones live at the end of the array? )
		//
		//	Draw the skinned models
		//
		{
			// Binding the pipeline is effectively the "use shader" we had back in our opengl apps
			g_skinningPipeline.BindPipeline( cmdBuffer );

			const RenderModel * lastRenderModel	   = &renderModels[ numModels - 1 ];
			const size_t offsetPastLastRenderModel = device->GetAligendUniformByteOffset( lastRenderModel->uboByteOffset + lastRenderModel->uboByteSize );
			size_t offsetToCurMatrixPalette		   = offsetPastLastRenderModel;

			// @TODO - we need to expand the UBO to support multiple matrixPalettes, 
			// so num animated meshes * 80!

			for ( int i = 0; i < numModels; i++ ) {
				const RenderModel & renderModel = renderModels[ i ];

				if ( renderModel.numBones <= 0 ) {
					continue;
				}

				// @TODO - we either need a dynamic buffer in the vert shader, 
				// or create a new vert shader for every skeleton size!
				// ( currently hardcoded to 80 in the vert shader )
				assert( renderModel.numBones == 80 );

				/*	ALL YOU NEED TO KNOW
				/*	ALL YOU NEED TO KNOW
				/*	ALL YOU NEED TO KNOW
				
				~~ drawing skinned mesh for the 206th time ~~ numbones = 80
				~~ drawing skinned mesh for the 207th time ~~ numbones = 80
				~~ drawing skinned mesh for the 208th time ~~ numbones = 80
				~~ drawing skinned mesh for the 209th time ~~ numbones = 80
				~~ drawing skinned mesh for the 210th time ~~ numbones = 80
				~~ drawing skinned mesh for the 211th time ~~ numbones = 80

	*** this happens on AFTER you hit T, to start running sim
	*** therefore, it is being causing by the update loop!

				~~ drawing skinned mesh for the 212th time ~~ numbones = 240
				~~ drawing skinned mesh for the 213th time ~~ numbones = 400
				~~ drawing skinned mesh for the 214th time ~~ numbones = 560
				~~ drawing skinned mesh for the 215th time ~~ numbones = 720
				~~ drawing skinned mesh for the 216th time ~~ numbones = 880
				ERROR: VulkanErrorMessage: Validation Error: [ VUID-VkDescriptorBufferInfo-range-00342 ] Object 0: handle = 0xd897d90000000016, type = VK_OBJECT_TYPE_BUFFER; | MessageID = 0xe9e00038 | vkUpdateDescriptorSets(): pDescriptorWrites[3].pBufferInfo[0].range (56320) is larger than buffer size (53248) + offset (3264). The Vulkan spec states: If range is not equal to VK_WHOLE_SIZE, range must be less than or equal to the size of buffer minus offset (https://vulkan.lunarg.com/doc/view/1.3.268.0/windows/1.3-extensions/vkspec.html#VUID-VkDescriptorBufferInfo-range-00342)
				~~ drawing skinned mesh for the 217th time ~~ numbones = 1040
				ERROR: VulkanErrorMessage: Validation Error: [ VUID-VkDescriptorBufferInfo-range-00342 ] Object 0: handle = 0xd897d90000000016, type = VK_OBJECT_TYPE_BUFFER; | MessageID = 0xe9e00038 | vkUpdateDescriptorSets(): pDescriptorWrites[3].pBufferInfo[0].range (66560) is larger than buffer size (53248) + offset (3264). The Vulkan spec states: If range is not equal to VK_WHOLE_SIZE, range must be less than or equal to the size of buffer minus offset (https://vulkan.lunarg.com/doc/view/1.3.268.0/windows/1.3-extensions/vkspec.html#VUID-VkDescriptorBufferInfo-range-00342)
				~~ drawing skinned mesh for the 218th time ~~ numbones = 1200
				ERROR: VulkanErrorMessage: Validation Error: [ VUID-VkDescriptorBufferInfo-range-00342 ] Object 0: handle = 0xd897d90000000016, type = VK_OBJECT_TYPE_BUFFER; | MessageID = 0xe9e00038 | vkUpdateDescriptorSets(): pDescriptorWrites[3].pBufferInfo[0].range (76800) is larger than buffer size (53248) + offset (3264). The Vulkan spec states: If range is not equal to VK_WHOLE_SIZE, range must be less than or equal to the size of buffer minus offset (https://vulkan.lunarg.com/doc/view/1.3.268.0/windows/1.3-extensions/vkspec.html#VUID-VkDescriptorBufferInfo-range-00342)
				~~ drawing skinned mesh for the 219th time ~~ numbones = 1360
				ERROR: VulkanErrorMessage: Validation Error: [ VUID-VkDescriptorBufferInfo-range-00342 ] Object 0: handle = 0xd897d90000000016, type = VK_OBJECT_TYPE_BUFFER; | MessageID = 0xe9e00038 | vkUpdateDescriptorSets(): pDescriptorWrites[3].pBufferInfo[0].range (87040) is larger than buffer size (53248) + offset (3264). The Vulkan spec states: If range is not equal to VK_WHOLE_SIZE, range must be less than or equal to the size of buffer minus offset (https://vulkan.lunarg.com/doc/view/1.3.268.0/windows/1.3-extensions/vkspec.html#VUID-VkDescriptorBufferInfo-range-00342)
				~~ drawing skinned mesh for the 220th time ~~ numbones = 1520
				ERROR: VulkanErrorMessage: Validation Error: [ VUID-VkDescriptorBufferInfo-range-00342 ] Object 0: handle = 0xd897d90000000016, type = VK_OBJECT_TYPE_BUFFER; | MessageID = 0xe9e00038 | vkUpdateDescriptorSets(): pDescriptorWrites[3].pBufferInfo[0].range (97280) is larger than buffer size (53248) + offset (3264). The Vulkan spec states: If range is not equal to VK_WHOLE_SIZE, range must be less than or equal to the size of buffer minus offset (https://vulkan.lunarg.com/doc/view/1.3.268.0/windows/1.3-extensions/vkspec.html#VUID-VkDescriptorBufferInfo-range-00342)
				~~ drawing skinned mesh for the 221th time ~~ numbones = 1680
				

	*** OTHER ISSUE - the new command buffer doesnt even show up in renderdoc! ( why not? )

				ALL YOU NEED TO KNOW 
				ALL YOU NEED TO KNOW 
				ALL YOU NEED TO KNOW */

				static int hitcount = 0;
				printf( "~~ \t\t\tdrawing skinned mesh for the %ith time ~~ numbones = %i\n", hitcount++, renderModel.numBones );
				/////////////////////////////////////////////////////////////////////////
				// @TODO - MEMORY LEAK HERE! - renderModelBones NOT BEING CLEANED UP!!!
				/////////////////////////////////////////////////////////////////////////

				const size_t matrixPaletteSize = sizeof( Mat4 ) * renderModel.numBones;

				// Descriptor is how we bind our buffers and images
				Descriptor descriptor = g_skinningPipeline.GetFreeDescriptor();
				descriptor.BindBuffer( uniforms, camOffset, camSize, 0 );									// bind the camera matrices
				descriptor.BindBuffer( uniforms, renderModel.uboByteOffset, renderModel.uboByteSize, 1 );	// bind the model matrices
				descriptor.BindBuffer( uniforms, shadowCamOffset, shadowCamSize, 2 );						// bind the shadow camera matrices
				descriptor.BindBuffer( uniforms, offsetToCurMatrixPalette, matrixPaletteSize, 3 );		// @TODO bind the skinning matrices layout info
				descriptor.BindImage( VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, g_shadowFrameBuffer.m_imageDepth.m_vkImageView, Samplers::m_samplerStandard, 0 );
				descriptor.BindDescriptor( device, cmdBuffer, &g_skinningPipeline );
				renderModel.model->DrawIndexed( cmdBuffer );

				offsetToCurMatrixPalette += device->GetAligendUniformByteOffset( matrixPaletteSize );
			}
		}

		g_offscreenFrameBuffer.EndRenderPass( device, cmdBufferIndex );

		g_offscreenFrameBuffer.m_imageColor.TransitionLayout( cmdBuffer, VK_IMAGE_LAYOUT_GENERAL );		
	}
}
