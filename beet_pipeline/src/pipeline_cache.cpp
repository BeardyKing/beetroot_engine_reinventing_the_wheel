#include <pipeline/pipeline_cache.h>
#include <pipeline/pipeline_commandlines.h>

#include <shared/log.h>

#include <sys/stat.h>

bool pipeline_cache_should_convert(const std::string &toPath, const std::string &fromPath) {
    if (commandline_get_arg(CLArgs::ignoreConvertCache).enabled) {
        return true;
    }

    struct stat toFileStat{};
    struct stat fromFileStat{};

    stat(toPath.c_str(), &toFileStat);
    stat(fromPath.c_str(), &fromFileStat);

    if (toFileStat.st_mtime > fromFileStat.st_mtime) {
        log_info(MSG_PIPELINE, "cached skipping convert: %s \n", toPath.c_str())
        return false;
    }
    return true;
}