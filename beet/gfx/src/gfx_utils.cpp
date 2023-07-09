#include <filesystem>
#include <iostream>
#include <fstream>

#include <shared/log.h>

void gfx_load_shader_binary(const char *path, char **out_data, size_t &out_fileSize) {
    std::ifstream file{path, std::ios::ate | std::ios::binary};

    if (!file.is_open()) {
        log_warning("failed to find path: %s\n", path);
        out_fileSize = 0;
    }

    out_fileSize = static_cast<size_t>(file.tellg());

    *out_data = new char[out_fileSize]();

    file.seekg(0);
    file.read(*out_data, out_fileSize);
    file.close();
}