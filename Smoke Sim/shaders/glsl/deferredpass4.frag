#version 450
#extension GL_KHR_vulkan_glsl: enable

// ______________________________________________________________________________
//                          Deferred Shading Pass 3
//                               Fragment Shader
// ______________________________________________________________________________
//                                   Input
// ______________________________________________________________________________
//          Set 0: [B0] Blinn-Phong Shaded Image
layout(location = 0) in FragData{
    vec2 uv;
    vec4 color;
}fragData; // Interpolated uvs from the giant triangle

layout(set = 0, binding = 0) uniform sampler2D gBufferBlinnPhong;
layout(set = 0, binding = 1) uniform sampler2D gBufferBrightness;

// ______________________________________________________________________________
//                                   Output
// ______________________________________________________________________________

layout(location = 0) out vec4 screenColor;

// ______________________________________________________________________________
//                                   Shader Code
// ______________________________________________________________________________

void main() {
    vec4 hdr_sample = texture(gBufferBlinnPhong, fragData.uv);
    vec3 hdrColor = hdr_sample.rgb;
    float frag_depth = hdr_sample.a;


    vec3 bloomColor = texture(gBufferBrightness, fragData.uv).rgb/2;
    hdrColor += bloomColor; // additive blending
    // tone mapping
    screenColor = vec4(hdrColor, frag_depth);
}