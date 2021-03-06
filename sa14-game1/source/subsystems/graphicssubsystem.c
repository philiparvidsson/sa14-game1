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
#include "graphics/text.h"
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

    renderTargetT* render_target;

#ifdef DRAW_TRI_NORMALS
    shaderT* normal_shader;
#endif // DRAW_TRI_NORMALS

    textureT* background_tex;
    textureT* screen_tex;

    shaderT* exposure_shader;

    shaderT* noise_shader;
    int      noise_seed;

    shaderT*       mblur_shader0;
    shaderT*       mblur_shader1;
    renderTargetT* mblur_rt;

    mat4x4 view_proj;
} graphicsSubsystemDataT;

/*------------------------------------------------
 * FUNCTIONS
 *----------------------------------------------*/

static void drawComponents(gameSubsystemT* subsystem, bool use_materials);

#ifdef DRAW_TRI_NORMALS
static void loadNormalShader(graphicsSubsystemDataT* gfx_data) {
    gfx_data->normal_shader = gameResource("shader:normals", ResShader);
}
#endif // DRAW_TRI_NORMALS

static void initPostFX(graphicsSubsystemDataT* gfx_data) {
    // Motion Blur -------------------------------

    gfx_data->mblur_shader0 = gameResource("shader:mblur0", ResShader);
    gfx_data->mblur_shader1 = gameResource("shader:mblur1", ResShader);
    gfx_data->mblur_rt      = createRenderTarget(screenWidth(), screenHeight());

    // Exposure -------------------------------------

    gfx_data->exposure_shader = gameResource("shader:exposure", ResShader);

    // Noise -------------------------------------

    gfx_data->noise_seed = 0;
    gfx_data->noise_shader = gameResource("shader:noise", ResShader);
}

bool postit = false;
int frame_counter;
static void applyPostFX(gameSubsystemT* subsystem) {
    graphicsSubsystemDataT* gfx_data = subsystem->data;

    frame_counter--;
    if (frame_counter < 0)
        frame_counter = 0;

    if (keyIsPressed('d') && frame_counter == 0) {
        postit = !postit;
        frame_counter = 10;
    }

    if (!postit)
        return;


    //--------------------------------------------
    // Motion Blur
    //--------------------------------------------

    // 1. Render velocity texture.

    renderTargetT* old_rt = useRenderTarget(gfx_data->mblur_rt);
    useShader      (gfx_data->mblur_shader0);
    clearDisplay   (0.0f, 0.0f, 0.0f);
    drawComponents (subsystem, false);
    useRenderTarget(old_rt);

    // 2. Apply motion blur.

    textureT* old_tex = useTexture(getRenderTargetColorTexture(gfx_data->mblur_rt), 1);
    useShader            (gfx_data->mblur_shader1);
    loadTextureFromScreen(gfx_data->screen_tex);
    shaderPostProcess    (gfx_data->screen_tex);
    useTexture           (old_tex, 1);

    //--------------------------------------------
    // Exposure
    //--------------------------------------------

    useShader            (gfx_data->exposure_shader);
    loadTextureFromScreen(gfx_data->screen_tex);
    shaderPostProcess    (gfx_data->screen_tex);

    //--------------------------------------------
    // Noise
    //--------------------------------------------

    useShader            (gfx_data->noise_shader);
    setShaderParam       ("Seed", &gfx_data->noise_seed);
    loadTextureFromScreen(gfx_data->screen_tex);
    shaderPostProcess    (gfx_data->screen_tex);

    gfx_data->noise_seed++;

    //--------------------------------------------
}

static void setupCamera(graphicsSubsystemDataT* gfx_data) {
    mat4x4 proj, view;

    // @To-do: Camera logic should be here.
    mat4x4_look_at(&(vec3) { 0.0f, 0.0f, 6.0f },
                   &(vec3) { 0.0f, 0.0f, 0.0f },
                   &(vec3) { 0.0f, 1.0f, 0.0f }, &view);

    float r = gfx_data->aspect_ratio;
    mat4x4_persp(-5.0f*r, 5.0f*r, -5.0f, 5.0f, -10.0f, -0.01f, &proj);

    mat4x4* vp = &gfx_data->view_proj;

    mat_identity(vp);
    mat_mul     (&view, vp, vp);
    mat_mul     (&proj, vp, vp);
}

static void setupLights(graphicsSubsystemDataT* gfx_data) {
    int two = 2;
    setShaderParam("NumLights", &two);

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
        vec2 pos = bodyPosition(phys_component->body);
        mat_transl_xyz(pos.x, pos.y, 0.0f, &translation);

        mat_mul(&gfx_component->transform, &model, &model);
        mat_mul(&translation             , &model, &model);

        gfx_component->prev_model_view_proj = gfx_component->model_view_proj;

        mat4x4* mvp = &gfx_component->model_view_proj;
        mat_identity(mvp);
        mat_mul     (&model              , mvp, mvp);
        mat_mul     (&gfx_data->view_proj, mvp, mvp);
    }
}

static void drawComponent(graphicsSubsystemDataT* gfx_data, gameComponentT* component, bool use_material) {
    graphicsComponentDataT* gfx_component  = component->data;

    if (!gfx_component->mesh)
        return;

    if (use_material) {
        useMaterial(gfx_component->material);
        setupLights(gfx_data);
    }

    setShaderParam("ModelViewProj"    , &gfx_component->model_view_proj);
    setShaderParam("PrevModelViewProj", &gfx_component->prev_model_view_proj);
    setShaderParam("NormalMatrix"     , &gfx_component->transform);

    drawMesh(gfx_component->mesh);
}

static void sortComponentsByMaterial(gameSubsystemT* subsystem) {
    for (int i = 0; i < arrayLength(subsystem->components); i++) {
        for (int j = (i+1); j < arrayLength(subsystem->components); j++) {
            gameComponentT** a_ptr = arrayGet(subsystem->components, i);
            gameComponentT** b_ptr = arrayGet(subsystem->components, j);

            graphicsComponentDataT* a = (*a_ptr)->data;
            graphicsComponentDataT* b = (*b_ptr)->data;

            if (a->material->sort_value > b->material->sort_value) {
                gameComponentT* tmp = *a_ptr;
                *a_ptr = *b_ptr;
                *b_ptr = tmp;
            }
        }
    }
}

static void drawComponents(gameSubsystemT* subsystem, bool use_materials) {
    if (use_materials)
        sortComponentsByMaterial(subsystem);

    for (int i = 0; i < arrayLength(subsystem->components); i++) {
        gameComponentT* component = *(gameComponentT**)arrayGet(subsystem->components, i);
        drawComponent(subsystem->data, component, use_materials);
    }

    if (use_materials)
        useMaterial(NULL);
}

static void drawEverything(gameSubsystemT* subsystem, float dt) {
    graphicsSubsystemDataT* gfx_data = subsystem->data;

    setupCamera(gfx_data);
    setupTransforms(subsystem);

    useRenderTarget(gfx_data->render_target);

    vec3* clear_color = &gfx_data->clear_color;
    clearDisplay(clear_color->r, clear_color->g, clear_color->b);

    GLint depth_mask;
    glGetIntegerv(GL_DEPTH_WRITEMASK, &depth_mask);
    glDepthMask(GL_FALSE);
    useShader(gfx_data->noise_shader);
    shaderPostProcess(gfx_data->background_tex);
    glDepthMask(depth_mask);


    drawComponents(subsystem, true);

#ifdef DRAW_TRI_NORMALS
    useShader(gfx_data->normal_shader);

    setupCamera   (gfx_data);
    drawComponents(subsystem, false);
#endif // DRAW_TRI_NORMALS

    useRenderTarget(NULL);
    presentRenderTarget(gfx_data->render_target);

    applyPostFX(subsystem);

    drawText("SCORE: 12345\nNOOB WARNING: HIGH", 10.0f, 10.0f, "Sector 034", 10);
}

gameSubsystemT* newGraphicsSubsystem(void) {
    gameSubsystemT* subsystem = newSubsystem("graphics");
    graphicsSubsystemDataT* gfx_data = calloc(1, sizeof(graphicsSubsystemDataT));

    gfx_data->aspect_ratio   = screenWidth() / (float)screenHeight();
    gfx_data->clear_color    = (vec3) { 1.0f, 1.0f, 1.0f };
    gfx_data->render_target  = createRenderTarget(screenWidth(), screenHeight());
    gfx_data->background_tex = gameResource("texture:background", ResTexture);
    gfx_data->screen_tex     = createTexture();

#ifdef DRAW_TRI_NORMALS
    loadNormalShader(gfx_data);
#endif // DRAW_TRI_NORMALS

    subsystem->data = gfx_data;
    subsystem->after_update_fn = drawEverything;

    initPostFX(gfx_data);

    return (subsystem);
}
