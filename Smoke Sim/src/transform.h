#include "tga/tga.hpp"
#include "tga/tga_utils.hpp"

/*      transform.h
* 
*       This file serves as a container for constructing transformation matrices.
* 
*       Written by Tobias Heller
*/

using glm::vec4;
using glm::vec3;
using glm::mat4;

struct Transform {
public:
    static glm::mat4 getTranslationMatrix(glm::vec3 translation)
    {
        return glm::translate(mat4(1), translation);
    }

    static glm::mat4 getRotationMatrix(glm::vec3 euler_angles_deg) {
        glm::mat4 combinedRotation = 
            getPartialRotationMatrix_X(euler_angles_deg.x) * 
            getPartialRotationMatrix_Y(euler_angles_deg.y) * 
            getPartialRotationMatrix_Z(euler_angles_deg.z);

        return combinedRotation;
    }

    static glm::mat4 getScaleMatrix(glm::vec3 scale)
    {
        return glm::scale(mat4(1), scale);
    }

private:
    static glm::mat4 getPartialRotationMatrix_X(float x)
    {
        return glm::mat4(1, 0,            0,           0, 
                         0, glm::cos(x), -glm::sin(x), 0, 
                         0, glm::sin(x),  glm::cos(x), 0, 
                         0, 0,            0,           1);
    }

    static glm::mat4 getPartialRotationMatrix_Y(float y)
    {
        return glm::mat4( glm::cos(y), 0,   glm::sin(y),   0, 
                          0,           1,   0,             0,
                         -glm::sin(y), 0,   glm::cos(y),   0,
                          0,           0,   0,             1);
    }

    static glm::mat4 getPartialRotationMatrix_Z(float z)
    {
        return glm::mat4(glm::cos(z), -glm::sin(z), 0,   0,
                         glm::sin(z),  glm::cos(z), 0,   0,
                         0,            0,           1,   0,
                         0,            0,           0,   1);
    }
};