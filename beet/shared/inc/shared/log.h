#ifndef BEETROOT_LOG_H
#define BEETROOT_LOG_H

#include <cstdio>

#define MSG_VERBOSE     0
#define MSG_INFO        1
#define MSG_DEBUG       2
#define MSG_WARNING     3
#define MSG_ERROR       4
#define MSG_CRITICAL    5

#define MSG_MIN_WARNING_LEVEL MSG_INFO

#if BEET_DEBUG
#define log_verbose(...){                               \
    beet_log(MSG_VERBOSE,"[verbose] : "  __VA_ARGS__)   \
}

#define log_info(...){                                  \
    beet_log(MSG_INFO,"[info] : "  __VA_ARGS__)         \
}

#define log_debug(...){                                 \
    beet_log(MSG_DEBUG,"[debug] : " __VA_ARGS__)        \
}

#define log_warning(...){                               \
    beet_log(MSG_WARNING,"[warning] : " __VA_ARGS__)    \
}

#define log_error(...){                                 \
    beet_log(MSG_ERROR,"[error] : "  __VA_ARGS__)   \
}

#define log_critical(...){                              \
    beet_log(MSG_CRITICAL,"[critical] : "  __VA_ARGS__) \
}

#define beet_log(level, ...){                           \
    if(level >= MSG_MIN_WARNING_LEVEL){                 \
        printf("[" __TIME__ "]" __VA_ARGS__);           \
    }                                                   \
}
#else
#define log_verbose(...){}
#define log_info(...){}
#define log_debug(...){}
#define log_warning(...){}
#define log_error(...){}
#define log_critical(...){}

#define beet_log(level, ...){}
#endif

#endif //BEETROOT_LOG_H
