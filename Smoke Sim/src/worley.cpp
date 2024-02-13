#include "worley.hpp"

tga::Buffer get_3D_worley_noise_buffer(tga::Interface* tgai) {
    std::cout << "[Worley]    Generating 3D Worley noise at " << WORLEY_RESOLUTION << " x " << WORLEY_RESOLUTION << " x " << WORLEY_RESOLUTION << " pixels...\n";
    auto [paramsData, paramsStaging, paramsSize] = stagingBufferOfType<WorleyParams>(*tgai);
    paramsData.point_count = WORLEY_POINTS;
    paramsData.cutoff = WORLEY_CUTOFF;
    paramsData.invert = WORLEY_INVERT;
    paramsData.resolution = WORLEY_RESOLUTION;
    auto params_buffer = tgai->createBuffer({ tga::BufferUsage::uniform, paramsSize, paramsStaging });

    auto worley_noise_cs = tga::loadShader(PATH_to_Shader("worley_storage_comp"), tga::ShaderType::compute, *tgai);

    auto worley_noise_pass = tgai->createComputePass(
        tga::ComputePassInfo{ worley_noise_cs }.setInputLayout({ tga::SetLayout{
            tga::BindingType::uniformBuffer,
            tga::BindingType::storageBuffer,
            tga::BindingType::storageBuffer
            } }));

    auto [worley_info, worley_info_staging, worley_info_size] = stagingBufferOfType<WorleyInfo>(*tgai);
    worley_info.offset = 777;
    auto worley_info_buffer = tgai->createBuffer({ tga::BufferUsage::storage, worley_info_size, worley_info_staging });

    auto [worley_output, worley_output_staging, worley_output_size] = stagingBufferOfType<uint32_t[128][128][128]>(*tgai);
    tga::Buffer worley_noise_buffer = tgai->createBuffer({ tga::BufferUsage::storage, worley_output_size, worley_output_staging });

    auto [worley_data, worley_staging, worley_size] = stagingBufferOfType<glm::vec3[20]>(*tgai);
    for (int i = 0; i < 20; i++) {
        float rand_x = getRandomFloat() * WORLEY_RESOLUTION;
        float rand_y = getRandomFloat() * WORLEY_RESOLUTION;
        float rand_z = getRandomFloat() * WORLEY_RESOLUTION;
        worley_data[i] = glm::vec3(rand_x, rand_y, rand_z);
    }
    auto worley_points_buffer = tgai->createBuffer({ tga::BufferUsage::storage, worley_size, worley_staging });
    auto worley_input_set = tgai->createInputSet({ worley_noise_pass, {
        tga::Binding{params_buffer, 0},
        tga::Binding{worley_points_buffer, 1},
        tga::Binding{worley_noise_buffer, 2}
        }, 0 });

    std::cout << "[Worley]    Dispatching Shader...\n";

    auto getWorleyNoiseCmd = tga::CommandRecorder(*tgai).setComputePass(worley_noise_pass)
        .bindInputSet(worley_input_set)
        .dispatch(
            WORLEY_RESOLUTION / 4,
            WORLEY_RESOLUTION / 4,
            WORLEY_RESOLUTION / 4)
        .barrier(tga::PipelineStage::ComputeShader, tga::PipelineStage::Transfer)
        .bufferDownload(worley_noise_buffer, worley_output_staging, worley_output_size)
        .endRecording();
    tgai->execute(getWorleyNoiseCmd);
    tgai->waitForCompletion(getWorleyNoiseCmd);

    std::cout << "[Worley]    => Done.\n";

    return worley_noise_buffer;
}