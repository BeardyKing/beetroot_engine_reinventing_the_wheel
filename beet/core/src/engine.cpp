//===defines=================
#include <core/engine.h>
#include <core/window.h>
#include <shared/assert.h>

#include <map>

//===internal structs========
struct Engine {
    std::map<uint32_t, void (*)()> createSystems;
    std::map<uint32_t, void (*)()> cleanupSystems;
    std::map<uint32_t, void (*)()> updateSystems;
};

Engine *g_engine;

//===internal functions======

//===api=====================
void engine_register_system_create(uint32_t priority, void(*ptr)()) {
    ASSERT_MSG(g_engine->createSystems.count(priority) == 0, "Err: create system already exists with this priority");
    g_engine->createSystems[priority] = ptr;
}

void engine_register_system_update(uint32_t priority, void(*ptr)()) {
    ASSERT_MSG(g_engine->updateSystems.count(priority) == 0, "Err: update system already exists with this priority");
    g_engine->updateSystems[priority] = ptr;
}

void engine_register_system_cleanup(uint32_t priority, void(*ptr)()) {
    ASSERT_MSG(g_engine->cleanupSystems.count(priority) == 0, "Err: cleanup system already exists with this priority");
    g_engine->cleanupSystems[priority] = ptr;
}

void engine_system_create() {
    for (const auto &sys: g_engine->createSystems) {
        sys.second();
    }
}

void engine_system_update() {
    for (const auto &sys: g_engine->updateSystems) {
        sys.second();
    }
}

void engine_system_cleanup() {
    // iterate over the cleanup systems in reverse.
    auto &cleanupSystems = g_engine->cleanupSystems;
    for (auto sys = cleanupSystems.rbegin(); sys != cleanupSystems.rend(); sys++) {
        sys->second();
    }
}

bool engine_is_open() {
    return window_is_open();
}

//===init & shutdown=========
void engine_create() {
    g_engine = new Engine;
}

void engine_cleanup() {
    delete g_engine;
    g_engine = nullptr;
}
