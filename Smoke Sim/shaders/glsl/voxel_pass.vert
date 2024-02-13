#version 450
#extension GL_KHR_vulkan_glsl: enable

// Position , uv, normal and tangent same as the tga::vertex struct
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;

layout(set = 0, binding = 0) uniform CameraData{
    mat4 toWorld;
    mat4 view;
    mat4 projection;
}camera;

layout(set = 0, binding = 1) buffer VoxelData {
        float voxels[120*120*20];
};

layout(location = 0) out FragData{
    vec2 uv;
    flat float value;
    vec3 normal;
    vec3 tangent;
}fragData;

void main() {

    float null_multiplier = voxels[gl_InstanceIndex] == 0? 0 : 1;

    int voxel_count_x = 120;
    int voxel_count_y = 20;
    int voxel_count_z = 120;


    vec3 scaled_position = position * 0.5;
    int index = gl_InstanceIndex;

    int x = index / (voxel_count_y * voxel_count_z);
	index -= x * voxel_count_y * voxel_count_z;
	
	int y = index / voxel_count_z;
	index -= y * voxel_count_z;

	int z = index;

    vec3 voxelPos = vec3(x,y,z) * 0.5;

	vec4 wPos = vec4(scaled_position + voxelPos,1) * null_multiplier;

    
    fragData.normal = normal;
    fragData.uv = uv;
    fragData.tangent = tangent;
    fragData.value = voxels[gl_InstanceIndex];

    gl_Position = camera.projection * camera.view * wPos + vec4(0,4,0,0);
}