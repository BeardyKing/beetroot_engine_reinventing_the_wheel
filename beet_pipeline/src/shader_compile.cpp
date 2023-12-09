#include <pipeline/shader_compile.h>
#include <pipeline/pipeline_defines.h>
#include <pipeline/pipeline_cache.h>

#include <shared/log.h>

#include <fmt/format.h>

void pipeline_build_shader_spv(const std::string &readPath, const std::string &writePath) {
    const std::string inPath = fmt::format("{}{}", PIPELINE_RES_DIR, readPath);
    const std::string outPath = fmt::format("{}{}", CLIENT_RUNTIME_RES_DIR, writePath);
    const std::string cmd = fmt::format("{} {} {} {}", GLSL_VALIDATOR_EXE_PATH, "-V -o", outPath, inPath);

    if (pipeline_cache_should_convert(outPath, inPath)) {
        log_info(MSG_PIPELINE, "shader: %s \n", outPath.c_str());
        system(cmd.c_str());
    }
}

void pipeline_shader_log() {
    log_info(MSG_PIPELINE, "\n")
    log_info(MSG_PIPELINE, "===========================\n")
    log_info(MSG_PIPELINE, "===BUILDING SHADERS========\n")
    log_info(MSG_PIPELINE, "===========================\n")
    log_info(MSG_PIPELINE, "\n")
}