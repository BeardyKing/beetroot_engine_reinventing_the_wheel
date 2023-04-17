#include <beet/time.h>
#include <beet/types.h>

#include <windows.h>

struct Time {
    double lastTime;
    double currentTime;
    double timeScaleDelta;
    double timeScale;
    double frequency;
    uint32_t frameCount;
    double deltaTime;
};

Time *g_time;

void time_create() {
    LARGE_INTEGER now;
    LARGE_INTEGER frequency;

    QueryPerformanceCounter(&now);
    QueryPerformanceFrequency(&frequency);

    g_time = new Time{
            (double) now.QuadPart,
            (double) now.QuadPart,
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

double time_delta() {
    return g_time->deltaTime;
}

void time_tick() {
    LARGE_INTEGER timeNow;

    QueryPerformanceCounter(&timeNow);

    g_time->frameCount += 1;
    g_time->lastTime = g_time->currentTime;
    g_time->currentTime = (double) timeNow.QuadPart;
    g_time->deltaTime = (g_time->currentTime - g_time->lastTime) *
                        ((g_time->timeScale * g_time->timeScaleDelta) / g_time->frequency);
}