#ifndef BEETROOT_GFX_MESH_H
#define BEETROOT_GFX_MESH_H

#include <gfx/gfx_types.h>

struct RawMesh {
    uint32_t vertexSize;
    Vertex *vertexData;

    uint32_t indexSize;
    uint32_t *indexData;
};
void gfx_create_cube_immediate(GfxMesh &outMesh);
void gfx_create_plane_immediate(GfxMesh &outMesh);
void gfx_create_mesh_immediate(const RawMesh& rawMeshData, GfxMesh &outMesh);
void gfx_cleanup_mesh(GfxMesh &mesh);

#endif //BEETROOT_GFX_MESH_H
