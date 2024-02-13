#version 450
#extension GL_KHR_vulkan_glsl: enable

/*
* Debug-Only, shows the voxels and their values on a 3D grid
*/

layout(location = 0) in FragData{
    vec2 uv;
	flat float value;
	vec3 normal;
    vec3 tangent;
};

layout(set = 0, binding = 2) uniform sampler2D voxelTexture;

layout(location = 0) out vec4 screen;

vec4 colorFromValue() {
	vec4 texture_color = texture(voxelTexture, uv);
	if (texture_color.x >0.5) {
		
		if (value < 1) {
			return vec4(value, 0, value, 1);
		} else if (value == 500) {				// Wand
			return vec4(0, 0, 0, 1);
		} else {
			return vec4(0, value/10, value/10, 1);
		}
	} else {
		return texture_color;
	}
}

void main() {
	screen = colorFromValue();
}