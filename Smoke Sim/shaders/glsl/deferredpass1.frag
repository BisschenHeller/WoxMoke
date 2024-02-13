#version 450
#extension GL_KHR_vulkan_glsl: enable

// ______________________________________________________________________________
//                          Deferred Shading Pass 1
//                              Fragment Shader
// ______________________________________________________________________________
//                                   Input
// ______________________________________________________________________________


// Data from the vertex shader (interpolated by rasterizer)
layout(location = 0) in FragData{
    vec3 positionWorld;
    vec3 normal;
    vec2 uv;
    vec3 tangent;
}fragData;

layout(set = 0, binding = 0) uniform CameraData{
    mat4 toWorld;
    mat4 view;
    mat4 projection;
}camera;

// Model Textures on Set 1 Binding 1
layout(set = 1, binding = 1) uniform sampler2D modelDiffuseTexture;
layout(set = 1, binding = 2) uniform Material {
    float roughness;
} mat;


// ______________________________________________________________________________
//                                   Output
// ______________________________________________________________________________

//layout(location = 0) out vec4 gDepth;
layout(location = 0) out vec4 gColor;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gFragmentPos;
layout(location = 3) out float gRoughness;


// ______________________________________________________________________________
//                                Shader Code
// ______________________________________________________________________________

void main() // Save everything in the respective textures
{
    gColor        = texture(modelDiffuseTexture, fragData.uv);
    gColor.w = distance(camera.toWorld[3].xyz, fragData.positionWorld);
    gNormal       = fragData.normal;
    gFragmentPos  = fragData.positionWorld;
    gRoughness = mat.roughness;

    float brightness = dot(gColor.rgb, vec3(0.2126, 0.7152, 0.0722));
}

