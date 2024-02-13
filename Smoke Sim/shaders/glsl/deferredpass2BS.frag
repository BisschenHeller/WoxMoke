#version 450
#extension GL_KHR_vulkan_glsl: enable

#define LIGHT_COUNT (51)

// ______________________________________________________________________________
//                          Deferred Shading Pass 2
//                              Fragment Shader
// ______________________________________________________________________________
//                                   Input
// ______________________________________________________________________________
// ___________________________ From Vertex Shader _______________________________

layout(location = 0) in FragData{
    vec2 uv;
    vec4 color;
}fragData; // Interpolated uvs from the giant triangle

// _______________________________ Input Sets _______________________________
//          Set 0: [B0] LightData, [B1] CameraData
//          Set 1: [B0] Color [B1] Normal [B2] ViewSpacePos

layout(set = 0, binding = 0, std140) buffer LightData{
    vec3 colors[LIGHT_COUNT];
    vec3 positions[LIGHT_COUNT];
}light;

layout(set = 0, binding = 1) uniform CameraData{
    mat4 toWorld;
    mat4 view;
    mat4 projection;
}camera;

//layout(set = 1, binding = 0) uniform sampler2D depthTex;
layout(set = 1, binding = 0) uniform sampler2D colorTex;
layout(set = 1, binding = 1) uniform sampler2D normalTex;
layout(set = 1, binding = 2) uniform sampler2D viewSpacePositionTex;
layout(set = 1, binding = 3) uniform sampler2D roughnessTex;

// ______________________________________________________________________________
//                                   Output
// ______________________________________________________________________________

layout(location = 0) out vec4 gColor; // Blinn Phong gBuffer
layout(location = 1) out vec4 gBrightness; // Brightness buffer


// ______________________________________________________________________________
//                                   Shader Code
// ______________________________________________________________________________



void main()
{
    float constantAttenuation = 10;
    float linearAttenuation = 30;
    float quadraticAttenuation = 30;
    
    vec4 diffuse_sample = texture(colorTex, fragData.uv);
    vec3 diffuse_color = diffuse_sample.rgb;
    float frag_depth = diffuse_sample.a;
    vec3 normal = normalize(texture(normalTex, fragData.uv).xyz);
    float roughness = texture(roughnessTex, fragData.uv).r;
    vec3 viewSpacePosition = texture(viewSpacePositionTex, fragData.uv).xyz;
    vec3 col = vec3(0,0,0);
    
    for (int i = 0; i < LIGHT_COUNT; i++) {
        float distance_to_light = distance(light.positions[i], viewSpacePosition);

        if (distance_to_light > 10) continue;

        float attentuation = 1.0 / (constantAttenuation +
                                   linearAttenuation * distance_to_light +
                                   quadraticAttenuation * distance_to_light * distance_to_light);

        // Blinn Phong Illumination model
        vec3 lightDir = normalize(light.positions[i] - viewSpacePosition);
        vec3 viewingDir = normalize(camera.toWorld[3].xyz-viewSpacePosition);
        vec3 halfwayDir = normalize(viewingDir+lightDir);

        // Diffuse Component
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = 1 * light.colors[i] * diff * diffuse_color;

        // Specular Component
        float n = 16.;
        float spec = pow(max(dot(normal,halfwayDir),0),n);
        vec3 specular = light.colors[i] * spec * pow((1-roughness),4);

        //vec3 col = specular + ambient + diffuse;
        col += attentuation * (diffuse + specular);
    }
    vec3 ambient = diffuse_color * 0.05;
    col += ambient;
    gColor = vec4(clamp(col, 0, 1),frag_depth);

    // Output for Bloom Pass

    float brightness = dot(gColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0.8)
        gBrightness = vec4(gColor.rgb, 1.0);
    else
        gBrightness = vec4(0.0, 0.0, 0.0, 1.0);
    return;
}

