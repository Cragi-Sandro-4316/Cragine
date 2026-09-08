#include "TextureAtlas.h"
#include <stb_image.h>


namespace crg::renderer {

    unsigned char* TextureAtlas::loadTextureData(int& width, int& height, int& channels, std::filesystem::path& path) {
        return stbi_load(path.c_str(), &width, &height, &channels, 0);
    }

}
