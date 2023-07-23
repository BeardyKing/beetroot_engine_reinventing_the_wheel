#ifndef BEETROOT_TEXTURE_COMPRESSION_H
#define BEETROOT_TEXTURE_COMPRESSION_H

#include <string>
#include <shared/texture_formats.h>

void pipeline_build_compressed_textures(const std::string &readPath, const std::string &writePath, TextureFormat format, bool generateMipsMaps);

#endif //BEETROOT_TEXTURE_COMPRESSION_H
