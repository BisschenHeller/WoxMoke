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

layout(set = 0, binding = 0) uniform sampler2D gBufferBloom;

// ______________________________________________________________________________
//                                   Output
// ______________________________________________________________________________

layout(location = 0) out vec4 screenColor;

// ______________________________________________________________________________
//                                   Shader Code
// ______________________________________________________________________________
float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{             
    vec2 tex_offset = 1.0 / textureSize(gBufferBloom, 0); // gets size of single texel
    vec3 result = texture(gBufferBloom, fragData.uv).rgb * weight[0]; // current fragment's contribution
    
    for(int i = 1; i < 5; ++i) {
        result += texture(gBufferBloom, fragData.uv + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        result += texture(gBufferBloom, fragData.uv - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
    }
    
    for(int i = 1; i < 5; ++i)
    {
        result += texture(gBufferBloom, fragData.uv + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        result += texture(gBufferBloom, fragData.uv - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
    }
    
    screenColor = vec4(result, 1.0);
}