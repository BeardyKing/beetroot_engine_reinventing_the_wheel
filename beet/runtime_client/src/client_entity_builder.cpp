#include <client/client_entity_builder.h>

#include <gfx/gfx_fallback.h>
#include <gfx/gfx_resource_db.h>

void build_primary_camera_entity() {
    Camera camera{};
    Transform transform{};
    transform.position = vec3f{0.0f, 0.0f, -1.0f};

    CameraEntity cameraEntity{};
    cameraEntity.transformIndex = gfx_db_add_transform(transform);
    cameraEntity.cameraIndex = gfx_db_add_camera(camera);
    gfx_db_add_camera_entity(cameraEntity);
}

void build_lit_entities() {
    uint32_t defaultMesh{};
    uint32_t defaultMaterial{};
    {
        GfxTexture fallbackTexture{};
        gfx_create_texture_immediate("../res/textures/UV_Grid/UV_Grid_test.dds", fallbackTexture);

        VkDescriptorSet descriptorSet;
        gfx_fallback_update_material_descriptor(descriptorSet, fallbackTexture);

        LitMaterial material{};
        material.descriptorSetIndex = gfx_db_add_descriptor_set(descriptorSet);
        material.albedoIndex = gfx_db_add_texture(fallbackTexture);

        Transform transform{};
        transform.position.y = -2;
        transform.position.z = -8;

        GfxMesh mesh{};
        gfx_create_fallback_mesh(mesh);

        defaultMesh = gfx_db_add_mesh(mesh);
        defaultMaterial = gfx_db_add_lit_material(material);

        LitEntity defaultCube{};
        defaultCube.transformIndex = gfx_db_add_transform(transform);
        defaultCube.meshIndex = defaultMesh;
        defaultCube.materialIndex = defaultMaterial;
        gfx_db_add_lit_entity(defaultCube);
    }
    {
        Transform transform{};
        transform.position.x = -2;
        transform.position.y = 1;
        transform.position.z = -12;

        LitEntity defaultCube{};
        defaultCube.transformIndex = gfx_db_add_transform(transform);
        defaultCube.meshIndex = defaultMesh;
        defaultCube.materialIndex = defaultMaterial;
        gfx_db_add_lit_entity(defaultCube);
    }
}

void client_build_entities() {
    build_primary_camera_entity();
    build_lit_entities();
}

