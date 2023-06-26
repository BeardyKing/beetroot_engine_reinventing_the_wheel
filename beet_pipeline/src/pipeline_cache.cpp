#include <pipeline/pipeline_cache.h>

#include <shared/log.h>

#include <sys/stat.h>

bool pipeline_cache_should_convert(const std::string &toPath, const std::string &fromPath) {
    struct stat toFileStat{};
    struct stat fromFileStat{};

    stat(toPath.c_str(), &toFileStat);
    stat(fromPath.c_str(), &fromFileStat);

    if (toFileStat.st_mtime > fromFileStat.st_mtime) {
        log_info("cached skipping convert: %s \n", toPath.c_str())
        return false;
    }
    return true;
}