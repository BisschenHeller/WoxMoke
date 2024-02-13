#pragma once


#define PATH_res ("../../../../res/")

#define PATH_to_Mesh(a) (std::string(PATH_res).append("Meshes/").append(a).append(".obj"))
#define PATH_to_Texture(a) (std::string(PATH_res).append("Textures/").append(a))
#define PATH_to_Shader(a) (std::string("../shaders/").append(a).append(".spv"))
#define PATH_to_UI(a) (std::string(PATH_res).append("UI/").append(a).append(".png"))
#define PATH_to_VXL (std::string(PATH_res).append("Voxels.vxl"))
#define PATH_to_Scene (std::string(PATH_res).append("Scene.txt"))

/*      paths.hpp
* 
*       A bunch of macros helping to keep all directory calls in one place. The best solution to the problem?
*		Far from it.
*		But it works.
*/
