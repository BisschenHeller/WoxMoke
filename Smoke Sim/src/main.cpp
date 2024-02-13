#include <chrono>
#include <iostream>
#include <vector>
#include <algorithm>

#include "tga/tga.hpp"
#include "tga/tga_utils.hpp"
#include "tga/tga_math.hpp"
#include "transform.h"
#include <cstdlib>
#include "voxel.hpp"
#include "scene.hpp"
#include "paths.hpp"
#include "flood_fill.hpp"
#include "worley.hpp"
#include "helpers.hpp"

#define CAMERA_HEIGHT (2.36f)

#define BOOL_SHOW_FOUR_SCREENS (false)

/*      main.cpp
* 
*       This file puts the different parts of the simulation together, including handeling the tga Interface,
*       Physics, user input and Texture handling.
* 
*       Written by Tobias Heller
*/

using voxel_value_type = float;

int main(int argc, char **argv)
{
    // Establish Entry Point
    tga::Interface tgai;

    // We need a vertex buffer and an index buffer for each model
    struct ModelBufferHandles {
        tga::Buffer vsBuffer;
        tga::Buffer idBuffer;
    };
    std::vector<ModelBufferHandles> modelBufferHandles;
    // We also neede to remember the number of indices in each model
    std::vector<uint32_t> modelIndiceCounts;
    // Since each model should also at least a color texture, store those as well
    std::vector<tga::Texture> modelTextures;
    std::vector<tga::Buffer> materialRoughnessBuffers;

    std::cout << "\n\n          Opening Scene\n\n\n";

    Scene scene{};
    bool loaded = scene.load_from_file("Scene.txt");
    if (!loaded) return 1;

    std::cout << "\n\n          Creating Buffers\n\n\n";

    {
        for (uint32_t i = 0; i < scene._toWorlds.size(); i++) {
            try {
                auto makeBuffer = [&](tga::BufferUsage usage, auto& vec) {
                    auto size = vec.size() * sizeof(vec[0]);
                    auto staging = tgai.createStagingBuffer({ size, tga::memoryAccess(vec) });
                    auto buffer = tgai.createBuffer({ usage, size, staging });
                    //tgai.free(staging);
                    return buffer;
                    };

                auto vsBuffer = makeBuffer(tga::BufferUsage::vertex, scene._meshes.at(i).vertexBuffer);
                auto idxBuffer = makeBuffer(tga::BufferUsage::index, scene._meshes.at(i).indexBuffer);

                try {
                    modelTextures.push_back(tga::loadTexture(PATH_to_Texture(scene._materials.at(i).texture_name_diffuse.append("_diffuse.png")),
                        tga::Format::r32g32b32a32_sfloat,
                        tga::SamplerMode::nearest,
                        tga::AddressMode::repeat
                        , tgai));
                    auto [mat_data, mat_staging, mat_size] = stagingBufferOfType<float>(tgai);
                    mat_data = scene._materials.at(i).roughness;
                    auto mat_buffer = tgai.createBuffer({ tga::BufferUsage::uniform, mat_size, mat_staging });
                    materialRoughnessBuffers.push_back(mat_buffer);
                }
                catch (std::exception e) {
                    std::cout << "[Scene]    " << scene._names.at(i) << " had no texture called " << (PATH_to_Texture(scene._materials.at(i).texture_name_diffuse) + "_diffuse.png") << "\n";
                    modelTextures.push_back(tga::loadTexture(PATH_to_Texture("checker.png"),
                        tga::Format::r32g32b32a32_sfloat,
                        tga::SamplerMode::nearest,
                        tga::AddressMode::repeat
                        , tgai));
                    auto [mat_data, mat_staging, mat_size] = stagingBufferOfType<float>(tgai);
                    mat_data = 0.5;
                    auto mat_buffer = tgai.createBuffer({ tga::BufferUsage::uniform, mat_size, mat_staging });
                    materialRoughnessBuffers.push_back(mat_buffer);
                }

                modelIndiceCounts.push_back(static_cast<uint32_t>(scene._meshes.at(i).indexBuffer.size()));
                modelBufferHandles.push_back({ vsBuffer, idxBuffer });
            }
            catch (const std::exception& e) {
                std::cerr << "Model could not be loaded: " << e.what() << '\n';
            }
        }
    }

    if (modelBufferHandles.empty()) {
        std::cerr
            << "No input provided, please specify [model.obj model_texture.png/jpg] for each model you want to show\n";
        return -1;
    }

    
    std::vector<tga::Buffer> modelMatrixBuffers;

    auto [grenade_data, grenade_staging, grenade_size] = stagingBufferOfType<glm::mat4>(tgai);

    for (size_t i = 0; i < modelBufferHandles.size(); i++) {
        auto [mdlData, mdlStaging, mdlSize] = stagingBufferOfType<glm::mat4>(tgai);
        
        mdlData = scene._toWorlds.at(i);

        //std::cout << modelNames[i] << ":\n" << glmMatrixToString(mdlData.toWorld) << "\n\n";
        
        modelMatrixBuffers.push_back(tgai.createBuffer({tga::BufferUsage::uniform, mdlSize, mdlStaging}));
    }

    //  Load shader code from file
    VoxelWorld<voxel_value_type> voxelWorld(60, 10, 60, 0.5);
    if (BOOL_READ_FROM_VXL) {
        if (!voxelWorld.load_from_vxl()) {
            return -1;
        }
    } else {
        voxelWorld.calculate_voxel_world(scene._meshes, scene._toWorlds, scene._names);
    }
    
    
    
    //  Window with the resolution of your screen
    //auto [wWidth, wHeight] = tgai.screenResolution();
    auto win = tgai.createWindow({1920, 1080, tga::PresentMode::immediate});

    // The essential stuff needed for a camera
    struct CameraData {
        glm::mat4 toWorld;
        glm::mat4 view;
        glm::mat4 projection;
    };

    auto [camData, camStaging, camSize] = stagingBufferOfType<CameraData>(tgai);
    camData.view = glm::lookAt
    (glm::vec3(0, 0, 50), 
        glm::vec3(0), 
        glm::vec3(0,1,0));
    camData.toWorld = glm::inverse(camData.view);
    // Note that fov expects radians
    camData.projection = glm::perspective_vk(glm::radians(60.f), static_cast<float>(1920) / 1080, 0.1f, 1000.f);
    auto camBuffer = tgai.createBuffer({tga::BufferUsage::uniform, camSize, camStaging});

    struct LightData {
        glm::vec3 colors[51];
        glm::vec3 positions[51];
    };

    auto [lightData, lightStaging, lightSize] = stagingBufferOfType<LightData>(tgai);
    
    for (size_t i = 0; i < scene._lights.size(); i++) {
        lightData.colors[i] = scene._lights.at(i).color;
        lightData.positions[i] = scene._lights.at(i).position;
    }
    
    auto lightBuffer = tgai.createBuffer({tga::BufferUsage::storage, lightSize, lightStaging});

    std::cout << "\n\n          Creating Render Passes\n\n\n";

    // _________ Pass 1: ________
    // INPUT: Models, Lights, Camera, VoxelWorld
    // Vertex Shader brings Models into clip space,
    // Fragment shader writes material data to gBuffer.
    // OUTPUT: gBuffer containing Surface Color, Normal Vectors and View Space Position

    auto pass1_vs = tga::loadShader(PATH_to_Shader("deferredpass1_vert"), tga::ShaderType::vertex, tgai);
    auto pass1_fs = tga::loadShader(PATH_to_Shader("deferredpass1_frag"), tga::ShaderType::fragment, tgai);
    
    tga::Texture gBufferColorDepth      = getGBuffer(tgai, tga::Format::r32g32b32a32_sfloat);
    tga::Texture gBufferNormal          = getGBuffer(tgai, tga::Format::r32g32b32a32_sfloat);
    tga::Texture gBufferRoughness       = getGBuffer(tgai, tga::Format::r32_sfloat);
    tga::Texture gBufferViewSpacePos    = getGBuffer(tgai, tga::Format::r32g32b32a32_sfloat);
    tga::Texture gBufferBrightColors    = getGBuffer(tgai, tga::Format::r32g32b32a32_sfloat);
    tga::Texture gBufferBloom           = getGBuffer(tgai, tga::Format::r32g32b32a32_sfloat);
    tga::Texture gBufferBlinnPhong      = getGBuffer(tgai, tga::Format::r32g32b32a32_sfloat);

    // Render pass to write results to gBuffer

    auto renderPass1 = tgai.createRenderPass(
        tga::RenderPassInfo{pass1_vs, pass1_fs, 
            
            std::vector<tga::Texture>{gBufferColorDepth, gBufferNormal, gBufferViewSpacePos, gBufferRoughness}}
            
            .setClearOperations(tga::ClearOperation::all)
            // Enable the depth test, compare with less or lessEqual
            .setPerPixelOperations(tga::PerPixelOperations{}.setDepthCompareOp(tga::CompareOperation::lessEqual))
            // Use the provided layout by tga_utils
            .setVertexLayout(tga::Vertex::layout())
            // Set 0: LightData, CameraData
            // Set 1: ModelData, ModelTexture
            .setInputLayout({
            tga::SetLayout{tga::BindingType::uniformBuffer},    // CameraData
            tga::SetLayout{tga::BindingType::uniformBuffer, 
                           tga::BindingType::sampler,
                           tga::BindingType::uniformBuffer}}));
        

    // Input sets in pass 1 da pass2 nur gBuffer verwendet.
    // One input set for things uniform to all meshes
    auto lightCameraInputSet = tgai.createInputSet({renderPass1, 
        {tga::Binding{camBuffer, 0}}, 0});

    // An input set for each model
    std::vector<tga::InputSet> modelInputSets;
    for (size_t i = 0; i < modelMatrixBuffers.size(); ++i) {
        auto& modelBuffer = modelMatrixBuffers[i];
        auto& texture = modelTextures[i];
        auto& mat = materialRoughnessBuffers.at(i);

        modelInputSets.push_back(tgai.createInputSet(
            {renderPass1, 
            {tga::Binding{modelBuffer, 0}, 
            tga::Binding{texture, 1},
            tga::Binding{mat, 2},
            }, 1}));
    }

    // ________ Pass 2:_________
    // INPUT: gBuffer containing Surface Color, Normal Vectors and View Space Position
    // Vertex Shader projects everything on a giant triangle
    // Fragment Shader computes Lighting and outputs surface color

    auto pass2_fs = tga::loadShader(PATH_to_Shader("deferredpass2BS_frag"), tga::ShaderType::fragment, tgai);
    auto pass2_vs = tga::loadShader(PATH_to_Shader("deferredpass2BS_vert"), tga::ShaderType::vertex, tgai);

    auto renderPass2 = tgai.createRenderPass(
        tga::RenderPassInfo{pass2_vs, pass2_fs, 
            
            std::vector<tga::Texture>{gBufferBlinnPhong, gBufferBrightColors}}
            .setClearOperations(tga::ClearOperation::all)
            // Enable the depth test, compare with less or lessEqual            
            .setPerPixelOperations(tga::PerPixelOperations{}.setDepthCompareOp(tga::CompareOperation::lessEqual))
            // Use the provided layout by tga_utils
            //.setVertexLayout(tga::Vertex::layout())      
            .setInputLayout({
                tga::SetLayout{ // Set 0
                    tga::BindingType::storageBuffer,    // [B0] Light Data
                    tga::BindingType::uniformBuffer},   // [B1] CameraData
                tga::SetLayout{ // Set 1
                    tga::BindingType::sampler,     // [B0] Color
                    tga::BindingType::sampler,     // [B1] Normal
                    tga::BindingType::sampler,    // [B3] ViewSpacePos
                    tga::BindingType::sampler},    // [B4] Roughness
                }));
      
    
    auto secondLightCameraInputSet = tgai.createInputSet({renderPass2, 
        {tga::Binding{lightBuffer, 0}, 
         tga::Binding{camBuffer, 1}}, 0});
    auto gBuffer = tgai.createInputSet({renderPass2, 
        {tga::Binding{gBufferColorDepth, 0}, 
         tga::Binding{gBufferNormal, 1}, 
         tga::Binding{gBufferViewSpacePos, 2},
         tga::Binding{gBufferRoughness, 3},
        }, 1});
    
 
    // ________ Pass 3 ________
    // Bloom: Takes brighness values and blurs them

    auto pass3_fs = tga::loadShader(PATH_to_Shader("gaussian_blur_frag"), tga::ShaderType::fragment, tgai);
    auto pass3_vs = tga::loadShader(PATH_to_Shader("giant_triangle_vert"), tga::ShaderType::vertex, tgai);

    /* Takes brightness values and blurs them.Takes gaussianBlurInputSet { horizBoolenBuffer, gBufferBrightColors } 
    * and outputs gBufferBloom with blurred values */
    auto renderPass3 = tgai.createRenderPass(
        tga::RenderPassInfo{pass3_vs, pass3_fs, gBufferBloom}
            .setClearOperations(tga::ClearOperation::all)
            // Enable the depth test, compare with less or lessEqual
            .setPerPixelOperations(tga::PerPixelOperations{}.setDepthCompareOp(tga::CompareOperation::lessEqual))
            // Use the provided layout by tga_utils
            .setInputLayout(
                {tga::SetLayout{tga::BindingType::sampler}}));  // [B0] BlinnPhong [B1] BrightColors

    auto gaussianBlurInputSet = tgai.createInputSet(
        {renderPass3,
         {tga::Binding{gBufferBrightColors, 0}}, 0});


    // ________ Pass 4 ________ Combining Blinn Phong and Blurred bright spots

    auto pass4_fs = tga::loadShader(PATH_to_Shader("deferredpass4_frag"), tga::ShaderType::fragment, tgai);
    auto pass4_vs = tga::loadShader(PATH_to_Shader("giant_triangle_vert"), tga::ShaderType::vertex, tgai);

    auto scene_render_result = getGBuffer(tgai);

    /* Takes blurred brightness values and blinn phong image and combines them. 
     * Takes combineInputSet { gBufferblinnPhong, gBufferBloom }
     * and outputs to window */
    auto renderPass4 = tgai.createRenderPass(
        tga::RenderPassInfo{pass4_vs, pass4_fs, scene_render_result}
            .setClearOperations(tga::ClearOperation::all)
            // Enable the depth test, compare with less or lessEqual
            .setPerPixelOperations(tga::PerPixelOperations{}.setDepthCompareOp(tga::CompareOperation::lessEqual))
            // Use the provided layout by tga_utils
            //.setVertexLayout(tga::Vertex::layout())
            .setInputLayout({tga::SetLayout{tga::BindingType::sampler,
                                            tga::BindingType::sampler}}));  // [B0] BlinnPhong [B1] BrightColors

    auto combineInputSet = tgai.createInputSet(
        {renderPass4, {tga::Binding{gBufferBlinnPhong, 0}, tga::Binding{gBufferBloom, 1}}, 0});

    // ________ Worley Noise Generator ________

    
    auto worley_noise_buffer = get_3D_worley_noise_buffer(&tgai);

    // ________ Path Tracing Pass ________ Compute Pass, produces a Path traced representation of the voxel grid
    
    auto path_tracing_cs = tga::loadShader(PATH_to_Shader("pathtracing_comp"), tga::ShaderType::compute, tgai);

    auto path_tracing_render_result = tgai.createTexture(
        { 1920 / 4, 1080 / 4,tga::Format::r32g32b32a32_sfloat, tga::SamplerMode::linear });

    auto path_tracing_compute_pass = tgai.createComputePass(
        tga::ComputePassInfo{ path_tracing_cs }
        .setInputLayout(
            { tga::SetLayout{
                tga::BindingType::uniformBuffer, // 0 Camera Data
                tga::BindingType::uniformBuffer, // 1 Voxel Info
                tga::BindingType::storageBuffer, // 2 Voxel Data
                tga::BindingType::sampler,       // 3 Scene Render
                tga::BindingType::storageBuffer, // 4 3D Worley Noise
                tga::BindingType::uniformBuffer, // 5 Info, contains timer
                tga::BindingType::storageImage,  // 6 out Bild mit Voxel Cloud
                tga::BindingType::storageBuffer,  // 7 Light Data
                tga::BindingType::uniformBuffer, // 8 Bullet Holes
                tga::BindingType::sampler // 9 Bullet Hole texture
                                            } }));

    auto [voxelInfoData, voxelInfoStaging, voxelInfoSize] = stagingBufferOfType<VoxelInfo>(tgai);
    voxelInfoData.voxel_count = voxelWorld.get_voxel_counts();
    voxelInfoData.voxel_size = glm::vec3(voxelWorld.get_voxel_size());
    voxelInfoData.world_measurements = glm::vec3(voxelWorld.get_world_measurements());
    voxelInfoData.smoke_radius = { 5,5,5 };
    voxelInfoData.smoke_center = { 30,1,30 };
    voxelInfoData.swipe = { 300,0,0 };
    
    auto voxelInfoBuffer = tgai.createBuffer({tga::BufferUsage::uniform, voxelInfoSize, voxelInfoStaging});

    auto [voxelData, voxelStaging, voxelSize] = stagingBufferOfType<voxel_value_type[120*120*20]>(tgai);
    for (uint32_t i = 0; i < voxelWorld.get_voxel_count(); i++) {
        voxelData[i] = voxelWorld.get_value_at_index(i);
    }
    auto voxelBuffer = tgai.createBuffer({ tga::BufferUsage::storage, voxelSize, voxelStaging});

    struct GeneralInfo {
        float timer;
    };

    auto [gen_info_data, gen_info_staging, gen_info_size] = stagingBufferOfType<GeneralInfo>(tgai);
    gen_info_data.timer = 0;
    auto info_buffer = tgai.createBuffer({ tga::BufferUsage::uniform, gen_info_size, gen_info_staging });

    struct Bulletholes {
        glm::vec3 origins[30];
        glm::vec3 directions[30];
        glm::vec3 timers[30];
        uint32_t count;
    };

    tga::Texture bullethole_tex;
    try {
        bullethole_tex = tga::loadTexture(PATH_to_UI("Bullethole"), tga::Format::r32_sfloat, tga::SamplerMode::linear, tga::AddressMode::clampBorder, tgai);
    }
    catch (std::exception ex) {
        std::cout << "[main.cpp]    Could not load \"" << PATH_to_UI("Bullethole") << "\" Check paths.hpp for correct resource path.\n";
        return -1;
    }

    auto [bullet_data, bullet_staging, bullet_size] = stagingBufferOfType<Bulletholes>(tgai);
    auto bullet_buffer = tgai.createBuffer({ tga::BufferUsage::uniform, bullet_size, bullet_staging });

    auto path_tracing_inputSet = tgai.createInputSet({ path_tracing_compute_pass, 
        {tga::Binding{camBuffer, 0},
         tga::Binding{voxelInfoBuffer, 1},
         tga::Binding{voxelBuffer, 2},
         tga::Binding{scene_render_result, 3},
         tga::Binding{worley_noise_buffer, 4},
         tga::Binding{info_buffer, 5},
         tga::Binding{path_tracing_render_result, 6},
         tga::Binding{lightBuffer, 7},
         tga::Binding{bullet_buffer, 8},
         tga::Binding{bullethole_tex, 9}
        }, 0 });

    // ________ Voxel Pass ________

    auto voxel_pass_vs = tga::loadShader(PATH_to_Shader("voxel_pass_vert"), tga::ShaderType::vertex, tgai);
    auto voxel_pass_fs = tga::loadShader(PATH_to_Shader("voxel_pass_frag"), tga::ShaderType::fragment, tgai);

    auto voxel_render_result = getGBuffer(tgai);

    auto voxel_pass = tgai.createRenderPass(
        tga::RenderPassInfo{voxel_pass_vs, voxel_pass_fs}
            .setRenderTarget(voxel_render_result)
            .setClearOperations(tga::ClearOperation::all)
            .setPerPixelOperations(tga::PerPixelOperations{}.setDepthCompareOp(tga::CompareOperation::lessEqual))
            .setVertexLayout(tga::Vertex::layout())
        .setInputLayout({ tga::SetLayout{
            tga::BindingType::uniformBuffer, // Camera Data
            tga::BindingType::storageBuffer, // Voxel Data
            tga::BindingType::sampler        // Voxel Texture
                                            }}));
    
    tga::Obj single_voxel = tga::loadObj(PATH_to_Mesh("Single_Voxel"));

    auto makeBuffer = [&](tga::BufferUsage usage, auto& vec) {
                auto size = vec.size() * sizeof(vec[0]);
                auto staging = tgai.createStagingBuffer({size, tga::memoryAccess(vec)});
                auto buffer = tgai.createBuffer({usage, size, staging});
                //tgai.free(staging);
                return buffer;
            };

    tga::Texture sv_texture = tga::loadTexture(
        PATH_to_Texture("Single_Voxel_diffuse.png"), 
        tga::Format::r32g32b32a32_sfloat, 
        tga::SamplerMode::nearest, 
        tgai);

    auto sv_vsBuffer = makeBuffer(tga::BufferUsage::vertex, single_voxel.vertexBuffer);
    auto sv_idxBuffer = makeBuffer(tga::BufferUsage::index, single_voxel.indexBuffer);
 
    auto sv_input_set = tgai.createInputSet({ voxel_pass, {
        tga::Binding{camBuffer, 0}, 
        tga::Binding{voxelBuffer, 1}, 
        tga::Binding{sv_texture, 2}}, 0 });

    

    // ________ Combination Pass ________ 

    auto combination_pass_vs = tga::loadShader(PATH_to_Shader("output_combination_vert"), tga::ShaderType::vertex, tgai);
    auto combination_pass_fs = tga::loadShader(PATH_to_Shader("output_combination_frag"), tga::ShaderType::fragment, tgai);

    auto combination_pass = tgai.createRenderPass(tga::RenderPassInfo{ combination_pass_vs, combination_pass_fs, win }
        .setInputLayout({ tga::SetLayout{
            tga::BindingType::sampler,  // Scene Render
            tga::BindingType::sampler,  // Path Tracing
            tga::BindingType::sampler,   // Voxel Render
            tga::BindingType::sampler   // Crosshair
            } }));

    tga::Texture crosshair_tex;
    try {
        crosshair_tex = tga::loadTexture(
            PATH_to_UI("Crosshair"),
            tga::Format::r32g32b32a32_sfloat,
            tga::SamplerMode::linear,
            tgai);
    }
    catch (std::runtime_error ex) {
        std::cout << "[main.cpp]    Could not load \"" << PATH_to_UI("Crosshair") << "\" Check paths.hpp for correct resource path.\n";
        return -1;
    }

    auto combinationInputSet = tgai.createInputSet({ combination_pass,
        {tga::Binding{scene_render_result, 0},
         tga::Binding{path_tracing_render_result, 1},
         tga::Binding{voxel_render_result, 2},
         tga::Binding{crosshair_tex, 3}
        }, 0});


    std::cout << "[Setup Complete] Models Loaded: " << modelInputSets.size() << std::endl;
    //
    //
    //          Rendering
    //
    //
    std::cout << "\n\n          Rendering\n\n\n";
    print_controls();

    // Command buffers are recorded once and then reused
    std::vector<tga::CommandBuffer> cmdBuffers(tgai.backbufferCount(win), tga::CommandBuffer{});
    //tga::CommandBuffer cmd;

    //  Record and present until signal to 
    
    glm::vec3 camPosition{ 30, CAMERA_HEIGHT, 30 };
    glm::vec3 camRotation{ 0, 0, 0 };
    camData.toWorld = glm::translate(camData.toWorld, camPosition);

    float moveSpeed = 10.f;
    float rotSpeed = 0.2f;

    auto prevTime = std::chrono::steady_clock::now();

    auto [pos_x, pos_y] = tgai.mousePosition(win);
    glm::vec3 old_mouse_position{ pos_x, pos_y, 0};

    FloodFill flood_fill{5,0.5f, &voxelWorld };
    
    float seconds_passed;
    auto time_start_of_simulation = std::chrono::steady_clock::now();
    glm::vec3 flood_fill_origin = voxelInfoData.smoke_center;

    bool godmode = true;
    float bullet_cooldown = 0.5f;
    float godmode_cooldown = 0.5f;
    float grenade_cooldown = 0.5f;
    while (!tgai.windowShouldClose(win)) {
        // ________________________________ FrameIndex ____________________________________________
        auto frameIndex = tgai.nextFrame(win);

        auto time_start_of_frame = std::chrono::steady_clock::now();
        
        // ________________________________ Time Management _______________________________________
        float dt = std::chrono::duration<float>(time_start_of_frame - prevTime).count();
        seconds_passed = std::chrono::duration<float>(time_start_of_frame - time_start_of_simulation).count();
        
        // ________________________________ Flood Fill ____________________________________________
        if (!flood_fill.is_done()) {
            // Fill new Available Voxels with smoke
            flood_fill.update(dt);

            // update the voxels that have changed for the staging buffer
            while (!flood_fill.interesting_voxels.empty()) {
                auto& [interesting_voxel, intersting_value] = flood_fill.interesting_voxels.front();
                flood_fill.interesting_voxels.pop();
                uint32_t index = voxelWorld.convert_voxel_coordinates_to_index(interesting_voxel).value();
                voxelData[index] = intersting_value;
            }
        }
        voxelInfoData.smoke_radius.x = flood_fill.get_current_radius();

        // Bounding box of the smoke to reduce Resources
        voxelInfoData.smoke_min_coord = voxelWorld.convert_voxel_to_world_coordinate(flood_fill.get_smoke_bounds().first).value() - glm::vec3(voxelWorld.get_voxel_size());
        voxelInfoData.smoke_max_coord = voxelWorld.convert_voxel_to_world_coordinate(flood_fill.get_smoke_bounds().second).value() + glm::vec3(voxelWorld.get_voxel_size());
        
        // ________________________________ Camera Rotation _______________________________________
        auto [posx, posy] = tgai.mousePosition(win);
        glm::vec3 new_mouse_position{ posx, posy, 0 };
        glm::vec3 mouse_movement = old_mouse_position - new_mouse_position;
        old_mouse_position = new_mouse_position;

        if (!tgai.keyDown(win, tga::Key::MouseMiddle)) {
            camRotation += glm::vec3(mouse_movement.y * dt * rotSpeed, mouse_movement.x * dt * rotSpeed, 0);
        }
        glm::mat4 rotationMatrix = Transform::getRotationMatrix(camRotation);
        
        // ________________________________ Camera Movement _______________________________________
        float forwardMovement = 0;
        float strafing = 0;
        float flying = 0;

        if (tgai.keyDown(win, tga::Key::W)) forwardMovement -= 1;
        if (tgai.keyDown(win, tga::Key::S)) forwardMovement += 1;
        if (tgai.keyDown(win, tga::Key::D)) strafing += 1;
        if (tgai.keyDown(win, tga::Key::A)) strafing -= 1;
        if (godmode) {
            if (tgai.keyDown(win, tga::Key::Shift_Left)) flying -= 1;
            if (tgai.keyDown(win, tga::Key::Space)) flying += 1;
        }

        glm::vec3 cameraMovement = { strafing, flying, forwardMovement };
        cameraMovement *= moveSpeed * dt;

        glm::vec3 orientedCameraMovement = glm::vec3(glm::vec4(cameraMovement, 0) * rotationMatrix);

        camPosition += orientedCameraMovement;
        if (!godmode) camPosition.y = CAMERA_HEIGHT;

        glm::vec3 newForward = glm::vec3(glm::vec4(0, 0, -1, 0) * rotationMatrix);
        glm::vec3 newUp = glm::vec3(glm::vec4(0, 1, 0, 0) * rotationMatrix);

        camData.view = glm::lookAt(camPosition, camPosition + newForward, newUp);
        camData.toWorld = glm::inverse(camData.view);

        // ________________________________ Godmode (Toggle Flight) _______________________________
        if (godmode_cooldown == 0.0f && tgai.keyDown(win, tga::Key::G)) {
            godmode_cooldown = 0.5f;
            godmode = !godmode;
        }
        godmode_cooldown = std::max(godmode_cooldown - dt, 0.f);

        // ________________________________ Bulletholes ___________________________________________
        if (bullet_cooldown == 0 && tgai.keyDown(win, tga::Key::MouseRight)) {
            bullet_cooldown = 0.1f;
            scene._bulletholes.push_back(Bullethole{ camPosition, newForward });
            /*std::cout << "New bullethole from " << std::to_string(camPosition.x) << " " << std::to_string(camPosition.y) << " " << std::to_string(camPosition.z) << " towards "
                << std::to_string(newForward.x) << " " << std::to_string(newForward.y) << " " << std::to_string(newForward.z) << "\n";*/
        }
        bullet_cooldown = std::max(0.f, bullet_cooldown - dt);
        
        scene.update_bulletholes(dt);
        
        bullet_data.count = static_cast<uint32_t>(scene._bulletholes.size());
        for (int i = 0; i < scene._bulletholes.size(); i++) {
            bullet_data.origins[i] = scene._bulletholes.at(i).origin;
            bullet_data.directions[i] = scene._bulletholes.at(i).direction;
            bullet_data.timers[i] = glm::vec3(scene._bulletholes.at(i).timer);
        }

        // ________________________________ Grenade Throw _________________________________________
        if (grenade_cooldown == 0.0f && tgai.keyDown(win, tga::Key::MouseLeft)) {
            RaycastHit hit = scene.raycast(camPosition, newForward);
            if (hit.hit) {
                scene.start_grenade(camPosition -newForward + newUp, hit.world_pos);
            }
            grenade_cooldown = 0.5f;
        }
        grenade_cooldown = std::max(0.0f, grenade_cooldown - dt);
        if (!scene.grenade_arrived()) {
            scene.update_grenade(dt);
        }
        else {
            flood_fill_origin = scene.get_grenade_pos();// +(glm::normalize(newForward) * glm::vec3(-0.5, -0.5, -0.5));
            flood_fill.reset(flood_fill_origin);
            voxelInfoData.smoke_center = flood_fill_origin;
        }
        grenade_data = scene._toWorlds.at(scene.grenade_index);

        // ________________________________ General Things ________________________________________
        gen_info_data.timer = seconds_passed;

        tgai.setWindowTitle(win,
            std::to_string(seconds_passed).substr(0, 1 + (seconds_passed>9? 1 : 0) + (seconds_passed>99? 1 : 0)) + " seconds passed. " +
            "Running at " + std::to_string(1 / dt).substr(0, (1/dt)< 100? 2 : 3) + " FPS. " +
            "Active Smoke Voxels: " + std::to_string(flood_fill.get_smoke_count()) + ". "
        );

        // ________________________________ Rendering _____________________________________________

        // Record a command Buffer only once per distinct window frame buffer
        if (!cmdBuffers[frameIndex]) {
            auto recorder = tga::CommandRecorder{tgai};

            recorder.bufferUpload(grenade_staging, modelMatrixBuffers[scene.grenade_index], grenade_size); // Position and rotation of the grenade

            recorder.bufferUpload(camStaging, camBuffer, camSize);  // Position and Rotation of the Camera
            
            recorder.bufferUpload(voxelStaging, voxelBuffer, voxelSize);    // Voxel grid 

            recorder.bufferUpload(voxelInfoStaging, voxelInfoBuffer, voxelInfoSize);    // Voxel info like radius and Voxel size

            recorder.bufferUpload(gen_info_staging, info_buffer, gen_info_size);    // Timer

            recorder.bufferUpload(bullet_staging, bullet_buffer, bullet_size);  // Bulletholes

            // ________ Path Tracing Pass, visualizes voxels using path tracing ___________________
            recorder.barrier(tga::PipelineStage::Transfer, tga::PipelineStage::ComputeShader);

            recorder.setComputePass(path_tracing_compute_pass)
                .bindInputSet(path_tracing_inputSet)
                .dispatch(1920 / 8, 1080 / 8, 1);

            recorder.barrier(tga::PipelineStage::ComputeShader, tga::PipelineStage::VertexShader);
            
            // ________ Deferred Pass 1, Bring Mesh Data onto Planes ______________________________
            recorder.setRenderPass(renderPass1, frameIndex, {0.37f, 0.57f, 0.2f, 0.f})
                .bindInputSet(lightCameraInputSet);
            
            for (size_t i = 0; i < modelInputSets.size(); ++i) {
                auto& [vertexBuffer, indexBuffer] = modelBufferHandles[i];
                auto& numIndices = modelIndiceCounts[i];
                auto& inputSet = modelInputSets[i];
                recorder
                    .bindVertexBuffer(vertexBuffer)
                    .bindIndexBuffer(indexBuffer)
                    .bindInputSet(inputSet)
                    .drawIndexed(numIndices, 0, 0);
            }

            // ________ Deferred Pass 2, Calculate Lighting, seperate bright spots for bloom ______
            recorder.setRenderPass(renderPass2, frameIndex, {0.37f, 0.57f, 0.2f, 0.f})
                .bindInputSet(secondLightCameraInputSet)
                .bindInputSet(gBuffer)
                .draw(6, 0);

            
            // ________ Deferred Pass 3, Blur the bright spots ____________________________________
            recorder.setRenderPass(renderPass3, frameIndex, {0.37f, 0.57f, 0.2f, 0.f})
                .bindInputSet(gaussianBlurInputSet)
                .draw(6, 0);

            // ________ Deferred Pass 4, Combining blurred brightness and geometry ________________
            recorder.setRenderPass(renderPass4, frameIndex, {0.37f, 0.57f, 0.2f, 0.f})
                .bindInputSet(combineInputSet)
                .draw(6, 0);

            // ________ Voxel Pass, uses forward rendering to visualize voxels in 3D (Debug Only) _
            if (BOOL_SHOW_FOUR_SCREENS) {
                recorder.setRenderPass(voxel_pass, frameIndex, { 0.37f, 0.57f, 0.2f, 0.f })
                    .bindVertexBuffer(sv_vsBuffer)
                    .bindIndexBuffer(sv_idxBuffer)
                    .bindInputSet(sv_input_set)
                    .drawIndexed(static_cast<uint32_t>(single_voxel.indexBuffer.size()), 0, 0, voxelWorld.get_voxel_count(), 0);
            }

            // ________ Combination Pass, adds all render results into a final output _____________
            recorder.setRenderPass(combination_pass, frameIndex, {0.37f, 0.57f, 0.2f, 0.f})
                .bindInputSet(combinationInputSet)
                .draw(6, 0);

            cmdBuffers[frameIndex] = recorder.endRecording();
        } else {
            // Need to reset the command buffer before re-using it
            tgai.waitForCompletion(cmdBuffers[frameIndex]);
        }
        tgai.execute(cmdBuffers[frameIndex]);
        
        tgai.present(win, frameIndex);

        prevTime = time_start_of_frame;
    }

    std::cout << "\n\n          Application Closed\n\n\n";

    return 0;
}


