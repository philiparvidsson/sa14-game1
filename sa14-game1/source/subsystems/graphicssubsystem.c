/*------------------------------------------------
 * DEFINES
 *----------------------------------------------*/

#define VECTOR_RGB

/*------------------------------------------------
 * INCLUDES
 *----------------------------------------------*/

#include "graphicssubsystem.h"

#include "base/common.h"
#include "components/graphicscomponent.h"
#include "components/physicscomponent.h"
#include "engine/game.h"
#include "engine/subsystem.h"
#include "graphics/graphics.h"
#include "graphics/material.h"
#include "graphics/shader.h"
#include "graphics/rendertarget.h"
#include "graphics/texture.h"
#include "math/matrix.h"
#include "math/vector.h"

#include <stdlib.h>

#include <GL/glew.h>

/*------------------------------------------------
 * TYPES
 *----------------------------------------------*/

typedef struct {
    vec3 clear_color;
    float aspect_ratio;

    shaderT*  default_shader;
    textureT* default_texture;

    renderTargetT* render_target;

#ifdef DRAW_TRI_NORMALS
    shaderT* normal_shader;
#endif // DRAW_TRI_NORMALS

    shaderT* noise_shader;
    float    noise_intensity;
    int      noise_seed;

    shaderT*       mblur_shader0;
    shaderT*       mblur_shader1;
    renderTargetT* mblur_rt;

    mat4x4 view_proj;
} graphicsSubsystemDataT;

/*------------------------------------------------
 * FUNCTIONS
 *----------------------------------------------*/
static void drawComponents(graphicsSubsystemDataT* gfx_data, arrayT* components);

static void loadDefaultShader(graphicsSubsystemDataT* gfx_data) {
    gfx_data->default_shader = createShader();

    string* vert_src = readGamePakFile("default.vert");
    string* frag_src = readGamePakFile("default.frag");
    
    compileVertexShader  (gfx_data->default_shader, vert_src);
    compileFragmentShader(gfx_data->default_shader, frag_src);
    
    free(vert_src);
    free(frag_src);
}


#ifdef DRAW_TRI_NORMALS
static void loadNormalShader(graphicsSubsystemDataT* gfx_data) {
    gfx_data->normal_shader = createShader();

    string* vert_src = readGamePakFile("normals.vert");
    string* geom_src = readGamePakFile("normals.geom");
    string* frag_src = readGamePakFile("normals.frag");
    
    compileVertexShader  (gfx_data->normal_shader, vert_src);
    compileGeometryShader(gfx_data->normal_shader, geom_src);
    compileFragmentShader(gfx_data->normal_shader, frag_src);
    
    free(vert_src);
    free(geom_src);
    free(frag_src);
}
#endif // DRAW_TRI_NORMALS

static void initPostFX(graphicsSubsystemDataT* gfx_data) {
    // Motion Blur -------------------------------
    
    string* vert_src = readGamePakFile("motionblur.vert");
    string* frag_src = readGamePakFile("motionblur0.frag");

    gfx_data->mblur_shader0 = createShader();
    
    compileVertexShader  (gfx_data->mblur_shader0, vert_src);
    compileFragmentShader(gfx_data->mblur_shader0, frag_src);
    
    free(vert_src);
    free(frag_src);
    
    vert_src = readGamePakFile("discard_z.vert");
    frag_src = readGamePakFile("motionblur1.frag");
    
    gfx_data->mblur_shader1 = createShader();
    
    compileVertexShader  (gfx_data->mblur_shader1, vert_src);
    compileFragmentShader(gfx_data->mblur_shader1, frag_src);

    free(vert_src);
    free(frag_src);
    
    gfx_data->mblur_rt = createRenderTarget(screenWidth(), screenHeight());
    
    // Noise -------------------------------------

    gfx_data->noise_intensity = 0.09f;
    gfx_data->noise_seed      = 0;
    
    vert_src = readGamePakFile("discard_z.vert");
    frag_src = readGamePakFile("noise.frag");
    
    gfx_data->noise_shader = createShader();
    
    compileVertexShader  (gfx_data->noise_shader, vert_src);
    compileFragmentShader(gfx_data->noise_shader, frag_src);
    
    free(vert_src);
    free(frag_src);
}

static void applyPostFX(gameSubsystemT* subsystem) {
    graphicsSubsystemDataT* gfx_data = subsystem->data;
    
    //--------------------------------------------
    // Motion Blur
    //--------------------------------------------

    // 1. Render velocity texture.

    renderTargetT* old_rt = useRenderTarget(gfx_data->mblur_rt);
    useShader(gfx_data->mblur_shader0);
    clearDisplay(0.0f, 0.0f, 0.0f);
    drawComponents(subsystem->data, subsystem->components);
    useRenderTarget(old_rt);

    // 2. Render to screen.

    useShader(gfx_data->mblur_shader1);
    textureT* old_tex = useTexture(getRenderTargetColorTexture(gfx_data->mblur_rt), 1);
    shaderPostProcess(NULL);
    useTexture(old_tex, 1);

    //--------------------------------------------
    // Noise
    //--------------------------------------------

    useShader(gfx_data->noise_shader);
    setShaderParam("Intensity", &gfx_data->noise_intensity);
    setShaderParam("Seed"     , &gfx_data->noise_seed);
    shaderPostProcess(NULL);

    gfx_data->noise_seed++;

    //--------------------------------------------
}

static void setupCamera(graphicsSubsystemDataT* gfx_data) {
    mat4x4 proj, view;

    // @To-do: Camera logic should be here.
    mat4x4_look_at(&(vec3) { 0.0f, 0.0f, 1.0f },
                   &(vec3) { 0.0f, 0.0f, 0.0f },
                   &(vec3) { 0.0f, 1.0f, 0.0f }, &view);

    float r = gfx_data->aspect_ratio;
    mat4x4_persp(-0.5f*r, 0.5f*r, -0.5f, 0.5f, -1.5f, -0.01f, &proj);

    mat4x4* vp = &gfx_data->view_proj;

    mat_identity(vp);
    mat_mul     (&view, vp, vp);
    mat_mul     (&proj, vp, vp);
}

static void setupLights(graphicsSubsystemDataT* gfx_data) {
    int one = 2;
    setShaderParam("NumLights", &one);

    vec3 light_pos      = (vec3) { 0.0f, 1.0f, 0.0f };
    vec3 light_ambient  = (vec3) { 0.0f, 0.0f, 0.0f };
    vec3 light_diffuse  = (vec3) { 1.0f, 1.0f, 1.0f };
    vec3 light_specular = (vec3) { 1.0f, 1.0f, 1.0f };

    setShaderParam("Lights[0].pos"     , &light_pos);
    setShaderParam("Lights[0].ambient" , &light_ambient);
    setShaderParam("Lights[0].diffuse" , &light_specular);
    setShaderParam("Lights[0].specular", &light_diffuse);

    vec_scale(&light_diffuse, 0.3f, &light_diffuse);
    light_pos.y = -1.0f;

    setShaderParam("Lights[1].pos"     , &light_pos);
    setShaderParam("Lights[1].ambient" , &light_ambient);
    setShaderParam("Lights[1].diffuse" , &light_specular);
    setShaderParam("Lights[1].specular", &light_diffuse);
}

static void setupTransforms(gameSubsystemT* subsystem) {
    graphicsSubsystemDataT* gfx_data = subsystem->data;

    for (int i = 0; i < arrayLength(subsystem->components); i++) {
        gameComponentT* component = *(gameComponentT**)arrayGet(subsystem->components, i);
        graphicsComponentDataT* gfx_component = component->data;

        gameComponentT* phys_c = getComponent(component->entity, "physics");
        // The graphics component needs a physics component to pull the position
        // from.
        assert(phys_c != NULL);
        physicsComponentDataT*  phys_component = phys_c->data;

        mat4x4 model;
        mat_identity(&model);

        mat4x4 translation;
        vec3 pos = bodyGetPosition(phys_component->body);
        mat_transl_xyz(pos.x, pos.y, pos.z, &translation);

        mat_mul(&gfx_component->transform, &model, &model);
        mat_mul(&translation             , &model, &model);

        gfx_component->prev_model_view_proj = gfx_component->model_view_proj;

        mat4x4* mvp = &gfx_component->model_view_proj;
        mat_identity(mvp);
        mat_mul     (&model              , mvp, mvp);
        mat_mul     (&gfx_data->view_proj, mvp, mvp);
    }
}

static void drawComponent(graphicsSubsystemDataT* gfx_data, gameComponentT* component) {
    graphicsComponentDataT* gfx_component  = component->data;

    if (!gfx_component->mesh)
        return;

    setShaderParam("ProjViewModel"    , &gfx_component->model_view_proj);
    setShaderParam("PrevProjViewModel", &gfx_component->prev_model_view_proj);

    setShaderParam("NormalTransform"  , &gfx_component->normal_transform);

    materialT* material = gfx_component->material;
    setShaderParam("Material.ambient"  , &material->ambient  );
    setShaderParam("Material.diffuse"  , &material->diffuse  );
    setShaderParam("Material.specular" , &material->specular );
    setShaderParam("Material.shininess", &material->shininess);
    
    drawMesh(gfx_component->mesh);
}

static void drawComponents(graphicsSubsystemDataT* gfx_data, arrayT* components) {
    for (int i = 0; i < arrayLength(components); i++) {
        gameComponentT* component = *(gameComponentT**)arrayGet(components, i);
        drawComponent(gfx_data, component);
    }
}

static void drawEverything(gameSubsystemT* subsystem, float dt) {
    graphicsSubsystemDataT* gfx_data = subsystem->data;

    setupCamera(gfx_data);
    setupTransforms(subsystem);

    useRenderTarget(gfx_data->render_target);

    vec3* clear_color = &gfx_data->clear_color;
    clearDisplay(clear_color->r, clear_color->g, clear_color->b);

    useShader (gfx_data->default_shader);
    useTexture(gfx_data->default_texture, 0);

    setupLights(gfx_data);
    drawComponents(gfx_data, subsystem->components);

#ifdef DRAW_TRI_NORMALS
    useShader (gfx_data->normal_shader);
    useTexture(gfx_data->normal_shader, 0);

    setupCamera   (gfx_data);
    drawComponents(gfx_data, subsystem->components);
#endif // DRAW_TRI_NORMALS

    useRenderTarget(NULL);
    presentRenderTarget(gfx_data->render_target);

    applyPostFX(subsystem);
}

gameSubsystemT* newGraphicsSubsystem(void) {
    gameSubsystemT* subsystem = newSubsystem("graphics");
    graphicsSubsystemDataT* gfx_data = calloc(1, sizeof(graphicsSubsystemDataT));

    gfx_data->clear_color     = (vec3) { 1.0f, 1.0f, 1.0f };
    gfx_data->aspect_ratio    = screenWidth() / (float)screenHeight();
    gfx_data->default_texture = createWhiteTexture();
    gfx_data->render_target   = createMultisampledRenderTarget(screenWidth(), screenHeight(), 8);

    loadDefaultShader(gfx_data);

#ifdef DRAW_TRI_NORMALS
    loadNormalShader(gfx_data);
#endif // DRAW_TRI_NORMALS

    subsystem->data = gfx_data;
    subsystem->after_update_fn = drawEverything;

    initPostFX(gfx_data);

    return (subsystem);
}
