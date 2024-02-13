#pragma once

#include <cstdlib>
#include <stdlib.h>
#include <tuple>
#include "tga/tga_utils.hpp"

/*      helpers.hpp
* 
*       This File and the corresponding .cpp file contain a few helper functions used in the main() file.
*/

// Returns a pseudo-random Float between 0 and 1
static float getRandomFloat() {
    return static_cast<float>(rand()) / (static_cast<float>(RAND_MAX));
}

// Ripped from a sample solution
// A little helper function to create a staging buffer that acts like a specific type
template <typename T>
std::tuple<T&, tga::StagingBuffer, size_t> stagingBufferOfType(tga::Interface& tgai)
{
    auto stagingBuff = tgai.createStagingBuffer({ sizeof(T) });
    return { *static_cast<T*>(tgai.getMapping(stagingBuff)), stagingBuff, sizeof(T) };
}

// Outputs the controls to the command line
static void print_controls() {
    std::cout << "\n";
    std::cout << "         ######                                                          ######\n";
    std::cout << "         ##              CONTROLS                                            ##\n";
    std::cout << "         ##                                                                  ##\n";
    std::cout << "         ##    [WASD] .  .  .  .  .   Movement                               ##\n";
    std::cout << "         ##    [Mouse] .  .  .  .  .  Look around                            ##\n";
    std::cout << "         ##    [Mouse_Middle] (Hold)  Stop turning                           ##\n";
    std::cout << "         ##    [Space] .  .  .  .  .  Fly Up                                 ##\n";
    std::cout << "         ##    [Shift] .  .  .  .  .  Fly Down                               ##\n";
    std::cout << "         ##    [Mouse_L] .  .  .  .   Throw Smoke (Only on geometry)         ##\n";
    std::cout << "         ##    [Mouse_R] .  .  .  .   Shoot (Only Visible in Smoke cloud)    ##\n";
    std::cout << "         ##    [G]  .  .  .  .  .  .  Toggle Flight                          ##\n";
    std::cout << "         ##                                                                  ##\n";
    std::cout << "         ######                                                          ######\n";
    std::cout << "\n";
}

std::string glmMatrixToString(glm::mat4 matrix);

/*
Returns a new tga::Texture with the specified format.
    Size:         1920 x 1080
    SamplerMode:  nearest
    AddressMode:  repeatMirror
    TextureType:  _2D
 */
tga::Texture getGBuffer(tga::Interface& tgai, tga::Format format);

/*
Returns a new tga::Texture.
    Size:         1920 x 1080
    Format:       r32g32b32a32_sfloat
    SamplerMode:  nearest
    AddressMode:  repeatMirror
    TextureType:  _2D
*/
tga::Texture getGBuffer(tga::Interface& tgai);

/*
Returns a new tga::Texture with the specified size.
    Format:      r32g32b32a32_sfloat
    SamplerMode: linear
    AddressMode: repeatMirror
    TextureType: _2D
 */
tga::Texture getGBuffer(tga::Interface& tgai, uint32_t screen_x, uint32_t screen_y);
