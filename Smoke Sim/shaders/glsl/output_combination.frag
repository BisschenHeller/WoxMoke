#version 450
#extension GL_KHR_vulkan_glsl : enable

#define VALUE_Wall (100.0f)
#define VALUE_Smoke (1.0f)

layout(location = 0) in FragData {
    vec2 uv;
    flat uint decider;
}fragData;

layout(set = 0, binding = 0) uniform sampler2D sceneRender;
layout(set = 0, binding = 1) uniform sampler2D pathTracing;
layout(set = 0, binding = 2) uniform sampler2D voxelRender;
layout(set = 0, binding = 3) uniform sampler2D crosshair;

layout(location = 0) out vec4 screen;

bool showBuffersIdependently = false;

vec3 calculate_combination() {
    vec4 path_tracing_sample = texture(pathTracing, fragData.uv);
    float path_tracing_absorption= clamp(path_tracing_sample.a, 0, 1);
    vec3 path_tracing_illumination = clamp(path_tracing_sample.rgb,0,1);

    vec4 scene_render_sample = texture(sceneRender, fragData.uv);
    vec3 scene_render_color = scene_render_sample.xyz;
    float scene_render_distance = abs(scene_render_sample.a);

    vec3 smoke_color = path_tracing_illumination;
    
    return mix((1-path_tracing_absorption) * scene_render_color + path_tracing_absorption * smoke_color, vec3(0,0.2,0.3), texture(crosshair, fragData.uv).a);
}

void main() {
    if (showBuffersIdependently) {
        switch (fragData.decider) {
            case 0:
                screen = vec4(calculate_combination(), 1);
                break;
            case 1:
                screen = texture(sceneRender, fragData.uv);
                break;
            case 2:
                screen = texture(pathTracing, fragData.uv);
                break;
            case 3:
                screen = texture(voxelRender, fragData.uv);
                break;
        }
    } else {
        screen = vec4(calculate_combination(),1);
    }
}

