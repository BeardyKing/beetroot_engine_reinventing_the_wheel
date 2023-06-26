#include <pipeline/shader_compile.h>
#include <pipeline/pipeline_defines.h>
#include <pipeline/pipeline_cache.h>

#include <shared/log.h>

#include <format>

void pipeline_build_shader_spv(const std::string &readPath, const std::string &writePath) {
    const std::string inPath = std::format("{}{}", PIPELINE_SHADER_DIR, readPath);
    const std::string outPath = std::format("{}{}", CLIENT_RUNTIME_SHADER_DIR, writePath);
    const std::string cmd = std::format("{} {} {} {}", GLSL_VALIDATOR_EXE_PATH, "-V -o", outPath, inPath);

    if (pipeline_cache_should_convert(outPath, inPath)) {
        log_info("shader: %s \n", outPath.c_str());
        system(cmd.c_str());
    }
}

void pipeline_shader_log() {
    log_info("\n")
    log_info("===========================\n")
    log_info("===BUILDING SHADERS========\n")
    log_info("===========================\n")
    log_info("\n")
}