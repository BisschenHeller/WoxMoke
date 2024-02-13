#version 450
#extension GL_KHR_vulkan_glsl: enable

#define BIAS_main 0.01
#define BIAS_others 0.05


layout(location = 0) out FragData{
    vec2 uv;
    flat uint decider;
}fragData;

vec2 uvs[6] = vec2[]( // Hard coded uvs for screen filling quad
    vec2(0.0, 0.0), //3
    vec2(0.0, 1.0), //2
    vec2(1.0, 1.0), //1

    vec2(0.0, 0.0), // 2
    vec2(1.0, 1.0), // 1
    vec2(1.0, 0.0) // 3
);

vec2 positions_four_quads[24] = vec2[] ( 
    vec2( -1, -1),vec2( -1, -0),vec2( 0, -0),
    vec2( -1, -1),vec2( 0, -0),vec2( 0, -1),   // Oben Rechts: Deferred Rendering Result

    vec2( 0, -1),vec2( 0, -0),vec2( 1, -0),
    vec2( 0, -1),vec2( 1, -0),vec2( 1, -1),   // Oben Links: Combined end result

    vec2( -1, 0),vec2( -1,  1),vec2( -0, 1),
    vec2( -1, 0),vec2( -0,  1),vec2( -0, 0),    // Unten Links: Path Tracing result

    vec2(0, 0), vec2(0, 1), vec2(1, 1),
    vec2(0, 0), vec2(1, 1), vec2(1, 0)        // Unten rechts:: instanced 3d voxel grid
);

vec2 positions_fullscreen[6] = vec2[] (
    vec2(-1, -1), vec2(-1, 1), vec2(1,1),
    vec2(-1, -1), vec2(1, 1), vec2(1,-1)
);

bool showBuffersIdependently = false;

void main() {
    fragData.uv = uvs[gl_VertexIndex % 6];
    fragData.decider = gl_VertexIndex / 6;

    if (showBuffersIdependently) {
        gl_Position = vec4(positions_four_quads[gl_VertexIndex % positions_four_quads.length()], 0.0, 1.0);
    } else {
        gl_Position = vec4(positions_fullscreen[gl_VertexIndex % positions_fullscreen.length()], 0.0, 1.0);
    }
}