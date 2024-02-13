#pragma once

#include "tga/tga_utils.hpp"
#include <iostream>
#include <random>
#include <cstdint>
#include "tga/tga_utils.hpp"
#include "helpers.hpp"
#include "paths.hpp"

#define WORLEY_RESOLUTION (128)
#define WORLEY_POINTS (40)
#define WORLEY_CUTOFF (30)
#define WORLEY_INVERT (false)

/*      worley.hpp
* 
*       This file and the corresponding .cpp file compute worley noise through the dispatch of the 
*       "worley_storage.comp" compute Shader. Since only the usage of a 3D array of floats yielded 
*       desirable results, as of 4. of Feb .2024 the implementation utilizing 3D textures was
*       culled from the project.
* 
*       Written by Tobias Heller
*/

struct WorleyParams {
    uint32_t point_count;
    float resolution;
    float cutoff;
    bool invert;
};

struct WorleyInfo {
    float offset;
};

tga::Texture get_3D_worley_noise_texture(tga::Interface* tgai);

tga::Buffer get_3D_worley_noise_buffer(tga::Interface* tgai);