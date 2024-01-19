#version 450
#extension GL_ARB_separate_shader_objects : enable

/*
==========================================
uniforms
==========================================
*/

layout( binding = 0 ) uniform uboCamera {
    mat4 view;
    mat4 proj;
} camera;
layout( binding = 1 ) uniform uboModel {
    mat4 model;
} model;
layout( binding = 2 ) uniform uboShadow {
    mat4 view;
    mat4 proj;
} shadow;
layout( set = 0, binding = 3 ) uniform matrixPalette {
    mat4 bones[80];
} matPalette;

/*
==========================================
attributes
==========================================
*/

layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec2 inTexCoord;
layout( location = 2 ) in vec4 inNormal;
layout( location = 3 ) in vec4 inTangent;
layout( location = 4 ) in vec4 inColor;
layout( location = 5 ) in ivec4 inBoneIdxes;
layout( location = 6 ) in vec4 inBoneWeights;

/*
==========================================
output
==========================================
*/

layout( location = 0 ) out vec4 worldNormal;
layout( location = 1 ) out vec4 modelPos;
layout( location = 2 ) out vec3 modelNormal;
layout( location = 3 ) out vec4 shadowPos;

out gl_PerVertex {
    vec4 gl_Position;
};

/*
==========================================
main
==========================================
*/
void main() {
    vec3 normal = 2.0 * ( inNormal.xyz - vec3( 0.5 ) );
	modelNormal = normal;

    // @TODO - actually use this value for the position
    const int boneIdx0 = inBoneIdxes.x;
    const int boneIdx1 = inBoneIdxes.y;
    const int boneIdx2 = inBoneIdxes.z;
    const int boneIdx3 = inBoneIdxes.w;
    
    const vec4 undeformedPos = vec4( inPosition, 1.0 );

    vec4 deformedPos0 = boneIdx0 == -1 ? vec4( 0, 0, 0, 0 ) : undeformedPos * matPalette.bones[ boneIdx0 ];
    vec4 deformedPos1 = boneIdx1 == -1 ? vec4( 0, 0, 0, 0 ) : undeformedPos * matPalette.bones[ boneIdx1 ];
    vec4 deformedPos2 = boneIdx2 == -1 ? vec4( 0, 0, 0, 0 ) : undeformedPos * matPalette.bones[ boneIdx2 ];
    vec4 deformedPos3 = boneIdx3 == -1 ? vec4( 0, 0, 0, 0 ) : undeformedPos * matPalette.bones[ boneIdx3 ];

    vec4 weightedPos  = deformedPos0 * inBoneWeights[ 0 ] + 
                        deformedPos1 * inBoneWeights[ 1 ] + 
                        deformedPos2 * inBoneWeights[ 2 ] + 
                        deformedPos3 * inBoneWeights[ 3 ];

    const vec3 position = weightedPos.xyz;
//    const vec3 position = inPosition;

	modelPos = vec4( inPosition, 1.0 );
   
    // Get the tangent space in world coordinates
    worldNormal = model.model * vec4( normal.xyz, 0.0 );
   
    // Project coordinate to screen
    gl_Position = camera.proj * camera.view * model.model * vec4( position, 1.0 );

    // Project the world position into the shadow texture position
    shadowPos = shadow.proj * shadow.view * model.model * vec4( position, 1.0 );
}