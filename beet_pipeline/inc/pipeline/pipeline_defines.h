#ifndef BEETROOT_PIPELINE_DEFINES_H
#define BEETROOT_PIPELINE_DEFINES_H

//===project=================
#define PIPELINE_RES_DIR BEET_CMAKE_RES_DIR // defined in preprocess_pipeline/CmakeLists.txt
#define CLIENT_RUNTIME_RES_DIR BEET_CMAKE_CLIENT_RES_DIR

//===pipeline cache==========
#define PIPELINE_CACHE_ALWAYS_CONVERT false

//===shader compile==========
#define PIPELINE_SHADER_DIR PIPELINE_RES_DIR "shaders/"
#define CLIENT_RUNTIME_SHADER_DIR BEET_CMAKE_CLIENT_RES_DIR "shaders/"

//===font atlas==============
#define PIPELINE_FONT_DIR PIPELINE_RES_DIR "fonts/"
#define CLIENT_RUNTIME_FONT_DIR BEET_CMAKE_CLIENT_RES_DIR "fonts/"

//===textures================
#define PIPELINE_TEXTURE_DIR PIPELINE_RES_DIR "textures/"
#define CLIENT_RUNTIME_TEXTURE_DIR BEET_CMAKE_CLIENT_RES_DIR "textures/"

#endif //BEETROOT_PIPELINE_DEFINES_H
