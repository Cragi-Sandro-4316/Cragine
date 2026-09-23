#include "ImageTexture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


namespace crg::renderer {

    unsigned char* ImageTexture::loadTextureData(int& width, int& height, int& channels, std::filesystem::path& path) {
        return stbi_load(path.c_str(), &width, &height, &channels, 0);
    }



}
