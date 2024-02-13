#include "helpers.hpp"

/*
* Returns a new tga::Texture with the specified format.
* Size:         1920 x 1080
* SamplerMode:  nearest
* AddressMode:  repeatMirror
* TextureType:  _2D
 */
tga::Texture getGBuffer(tga::Interface& tgai, tga::Format format) {
    return tgai.createTexture(tga::TextureInfo{ 1920, 1080,
        format,
        tga::SamplerMode::nearest,
        tga::AddressMode::repeatMirror,
        tga::TextureType::_2D });
};

tga::Texture getGBuffer(tga::Interface& tgai) {
    return tgai.createTexture(tga::TextureInfo{ 1920, 1080,
        tga::Format::r32g32b32a32_sfloat,
        tga::SamplerMode::nearest,
        tga::AddressMode::repeatMirror,
        tga::TextureType::_2D });
};


tga::Texture getGBuffer(tga::Interface& tgai, uint32_t screen_x, uint32_t screen_y) {
    return tgai.createTexture(tga::TextureInfo{ screen_x, screen_y,
        tga::Format::r32g32b32a32_sfloat,
        tga::SamplerMode::linear,
        tga::AddressMode::repeatMirror,
        tga::TextureType::_2D });
};

std::string glmMatrixToString(glm::mat4 matrix)
{
    return std::string("(" + std::to_string(matrix[0][0]) + " " + std::to_string(matrix[1][0]) +
        " " + std::to_string(matrix[2][0]) + " " + std::to_string(matrix[3][0]) + "\n" +
        std::to_string(matrix[0][1]) + " " + std::to_string(matrix[1][1]) + " " + std::to_string(matrix[2][1]) +
        " " + std::to_string(matrix[3][1]) + "\n" + std::to_string(matrix[0][2]) +
        " " + std::to_string(matrix[1][2]) + " " + std::to_string(matrix[2][2]) + " " + std::to_string(matrix[3][2]) +
        "\n" + std::to_string(matrix[0][3]) + " " + std::to_string(matrix[1][3]) +
        " " + std::to_string(matrix[2][3]) + " " + std::to_string(matrix[3][3]) + ")\n");
};
