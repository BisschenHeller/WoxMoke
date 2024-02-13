#version 450
#extension GL_KHR_vulkan_glsl: enable

// ______________________________________________________________________________
//                          Deferred Shading Pass 2
//                               Vertex Shader
// ______________________________________________________________________________
//                                   Input
// ______________________________________________________________________________
//          Set 0: [B0] LightData, [B1] CameraData
//          Set 1: [B0] Color [B1] Normal [B2] ViewSpacePos

// Not needed, we make everything up as we go :P

// ______________________________________________________________________________
//                                   Output
// ______________________________________________________________________________

layout(location = 0) out FragData{
    vec2 uv;
    vec4 color; // Debug only
}fragData;

// ______________________________________________________________________________
//                                 Shader Code
// ______________________________________________________________________________

vec2 uvs[6] = vec2[]( // Hard coded uvs for screen filling quad
    vec2(0.0, 0.0), //3
    vec2(0.0, 1.0), //2
    vec2(1.0, 1.0), //1

    vec2(0.0, 0.0), // 2
    vec2(1.0, 1.0), // 1
    vec2(1.0, 0.0) // 3
);

vec2 positions[24] = vec2[] ( 
    vec2(-0.95, -0.95),vec2(-0.95,  -0.05),vec2( -0.05,  -0.05),
    vec2(-0.95, -0.95),vec2( -0.05,  -0.05),vec2( -0.05, -0.95),   // Oben Links: Combined

    vec2( 0.1, -0.9),vec2( 0.1, -0.1),vec2( 0.9, -0.1),
    vec2( 0.1, -0.9),vec2( 0.9, -0.1),vec2( 0.9, -0.9),   // Oben Rechts: fragmentPosition

    vec2( -0.9, 0.1),vec2( -0.9,  0.9),vec2( -0.1,  0.9),
    vec2( -0.9, 0.1),vec2( -0.1,  0.9),vec2( -0.1, 0.1),    // Unten Links: Normal

    vec2(0.1, 0.1), vec2(0.1, 0.9), vec2(0.9, 0.9),
    vec2(0.1, 0.1), vec2(0.9, 0.9), vec2(0.9, 0.1)        // Unten rechts:: combined
);

vec2 positions_fullscreen[6] = vec2[] (
    vec2(-1, -1), vec2(-1, 1), vec2(1,1),
    vec2(-1, -1), vec2(1, 1), vec2(1,-1)
);

void main() // place big traingle and pass uv to fragData;
{
    fragData.uv = uvs[gl_VertexIndex % uvs.length()];
    if (gl_VertexIndex >= 18) {
        fragData.color = vec4(1,0,0,1);
    } else if (gl_VertexIndex >= 12) {
        fragData.color = vec4(0.6667,0,0,1);
    } else if (gl_VertexIndex >= 6) {
        fragData.color = vec4(0.3333,0,0,1);
    } else {
        fragData.color = vec4(0,0,0,1);
    }
    gl_Position = vec4(positions_fullscreen[gl_VertexIndex % positions_fullscreen.length()], 0.0, 1.0);
}