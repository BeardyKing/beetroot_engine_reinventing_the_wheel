#include <core/time.h>

#include <windows.h>

//===internal structs========
struct Time {
    double timeOnStartUp;
    double lastTime;
    double currentTime;
    double timeScaleDelta;
    double timeScale;
    double frequency;
    uint32_t frameCount;
    double deltaTime;
};

Time *g_time;

//===internal functions======
//===api=====================
double time_delta() {
    return g_time->deltaTime;
}

double time_current() {
    return g_time->currentTime - g_time->timeOnStartUp;
}

uint32_t time_frame_count() {
    return g_time->frameCount;
}

void time_tick() {
    LARGE_INTEGER timeNow;

    QueryPerformanceCounter(&timeNow);

    g_time->frameCount += 1;
    g_time->lastTime = g_time->currentTime;
    g_time->currentTime = (double) timeNow.QuadPart / g_time->frequency;
    g_time->deltaTime = (g_time->currentTime - g_time->lastTime) *
                        ((g_time->timeScale * g_time->timeScaleDelta) / g_time->frequency);
}

//===init & shutdown=========
void time_create() {
    LARGE_INTEGER now;
    LARGE_INTEGER frequency;

    QueryPerformanceCounter(&now);
    QueryPerformanceFrequency(&frequency);

    g_time = new Time{
            (double) now.QuadPart / (double) frequency.QuadPart,
            (double) 0,
            (double) 0,
            1.0,
            1000.0,
            (double) frequency.QuadPart,
            0
    };
}

void time_cleanup() {
    delete g_time;
    g_time = nullptr;
}

