#version 450
#extension GL_KHR_vulkan_glsl: enable

// ______________________________________________________________________________
//                          Deferred Shading Pass 3
//                               Vertex Shader
// ______________________________________________________________________________
//                                   Input
// ______________________________________________________________________________
//          Set 0: [B0] Blinn-Phong Shaded Image

vec2 uvs[6] = vec2[](   // Hard coded uvs for screen filling quad
    vec2(0.0, 0.0), //3
    vec2(0.0, 1.0), //2
    vec2(1.0, 1.0), //1

    vec2(0.0, 0.0), // 2
    vec2(1.0, 1.0), // 1
    vec2(1.0, 0.0) // 3
);

// ______________________________________________________________________________
//                                   Output
// ______________________________________________________________________________

layout(location = 0) out FragData{
    vec2 uv;
    vec4 color; // Debug only
}fragData;

// ______________________________________________________________________________
//                                   Shader Code
// ______________________________________________________________________________
vec2 positions_fullscreen[6] = vec2[] (
    vec2(-1, -1), vec2(-1, 1), vec2(1,1),
    vec2(-1, -1), vec2(1, 1), vec2(1,-1)
);

void main() // place big traingle and pass uv to fragData;
{
    //fragData.color = (texture(colorTex, uvs[gl_VertexIndex % uvs.length()]));
    //fragData.normal = texture(normalTex, uvs[gl_VertexIndex % uvs.length()]).xyz;
    //fragData.viewSpacePosition = texture(viewSpacePositionTex, uvs[gl_VertexIndex % uvs.length()]).xyz;

    //fragData.color = colors[gl_VertexIndex % colors.length()];
    fragData.uv = uvs[gl_VertexIndex % uvs.length()];
    gl_Position = vec4(positions_fullscreen[gl_VertexIndex % positions_fullscreen.length()], 0.0, 1.0);
}