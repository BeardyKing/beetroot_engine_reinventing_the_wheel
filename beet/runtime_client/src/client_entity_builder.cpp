#include <client/client_entity_builder.h>

#include <gfx/gfx_lit.h>
#include <gfx/gfx_font.h>
#include <gfx/gfx_texture.h>
#include <gfx/gfx_mesh.h>
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
        GfxTexture uvTestTexture{};
        gfx_create_texture_immediate("../res/textures/UV_Grid/UV_Grid_test.dds", uvTestTexture);

        VkDescriptorSet descriptorSet;
        gfx_lit_update_material_descriptor(descriptorSet, uvTestTexture);

        LitMaterial material{};
        material.descriptorSetIndex = gfx_db_add_descriptor_set(descriptorSet);
        material.albedoIndex = gfx_db_add_texture(uvTestTexture);

        Transform transform{};
        transform.position.y = -2;
        transform.position.z = -8;

        GfxMesh mesh{};
        gfx_create_cube_immediate(mesh);

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

void build_font_entities(){
    {
        GfxTexture fontAtlasTexture{};
        gfx_create_texture_immediate("../res/fonts/JetBrainsMono/JetBrainsMono-Regular.dds", fontAtlasTexture);

        VkDescriptorSet descriptorSet;
        gfx_font_update_material_descriptor(descriptorSet, fontAtlasTexture);

        FontMaterial material{}; // TODO Update with font material
        material.descriptorSetIndex = gfx_db_add_descriptor_set(descriptorSet); // TODO Update with font update set
        material.atlasIndex = gfx_db_add_texture(fontAtlasTexture);

        UiTransform transform{};
        transform.position.x = -.5;
        transform.position.y = 0;
        transform.scale.x = 1.0f;
        transform.scale.y = 1.0f;
        transform.size.x = 400;
        transform.size.y = 400;

        GfxMesh mesh{};
        gfx_create_plane_immediate(mesh);

        uint32_t planeMeshIdx = gfx_db_add_mesh(mesh);
        uint32_t fontMaterialIdx = gfx_db_add_font_material(material); // TODO Update with font material

        FontEntity defaultCube{};
        defaultCube.uiTransformIndex = gfx_db_add_ui_transform(transform);
        defaultCube.meshIndex = planeMeshIdx;
        defaultCube.materialIndex = fontMaterialIdx;
        gfx_db_add_font_entity(defaultCube);
    }
}

void client_build_entities() {
    build_primary_camera_entity();
    build_lit_entities();
    build_font_entities();
}

