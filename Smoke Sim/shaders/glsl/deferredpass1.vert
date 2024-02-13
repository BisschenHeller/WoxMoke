#version 450
#extension GL_KHR_vulkan_glsl: enable

// ______________________________________________________________________________
//                          Deferred Shading Pass 1
//                               Vertex Shader
// ______________________________________________________________________________
//                                   Input
// ______________________________________________________________________________
//          Set 0: [B0] LightData, [B1] CameraData
//          Set 1: [B0] ModelData, [B1] ModelTexture

// Position , uv, normal and tangent same as the tga::vertex struct
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;

// Use this set here to for view and projection
layout(set = 0, binding = 0) uniform CameraData{
    mat4 toWorld;
    mat4 view;
    mat4 projection;
}camera;

// Per model transform
layout(set = 1, binding = 0) uniform ModelData{
    mat4 toWorld;
} model;



// ______________________________________________________________________________
//                                   Output
// ______________________________________________________________________________

layout(location = 0) out FragData{
    vec3 positionWorld;
    vec3 normal;
    vec2 uv;
    vec3 tangent;
}fragData;


// ______________________________________________________________________________
//                                Shader Code
// ______________________________________________________________________________

void main() // Give information for interpolation to rasterizer and then to Frag Shader 1 
{
    vec4 wPos = model.toWorld * vec4(position,1);
    vec4 cPos = camera.toWorld[3];

    // There is no sheering or skew, so the model matrix contains a pure rotation
    fragData.normal = mat3(model.toWorld)*normal;
    fragData.uv = uv;
    fragData.positionWorld = wPos.xyz;
    fragData.tangent = tangent;

    gl_Position = camera.projection * camera.view * wPos;
}
