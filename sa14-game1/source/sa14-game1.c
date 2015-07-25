/*------------------------------------------------------------------------------
 * File: sa14-game1.c
 * Created: June 7, 2015
 * Last changed: July 14, 2015
 *
 * Author(s): Philip Arvidsson (philip@philiparvidsson.com)
 *
 * Description:
 *   Main program file.
 *----------------------------------------------------------------------------*/

/*------------------------------------------------
 * INCLUDES
 *----------------------------------------------*/

#include "resources.h"

#include "base/array.h"
#include "base/common.h"
#include "base/pak.h"
#include "base/time.h"
#include "engine/game.h"
#include "graphics/graphics.h"
#include "graphics/shader.h"
#include "graphics/text.h"
#include "graphics/texture.h"
#include "graphics/trimesh.h"
#include "graphics/io/3ds.h"

/*------------------------------------------------
 * FUNCTIONS
 *----------------------------------------------*/

/*--------------------------------------
 * Function: printIntroMessage()
 *
 * Description:
 *   Prints an introductory message.
 *
 * Usage:
 *   printIntroMessage();
 *------------------------------------*/
static void printIntroMessage(void) {
    printf("sa14-game1 v%s (build %d) by %s\n\n\n", ProgramVersion, 0, ProgramAuthors);
    printf("Some time in the not too distant future, this program will be a\n"
           "really cool game instead of this crap that does nothing...\n\n");

    printf("Full source available @ github.com/philiparvidsson\n\n");
}

/*--------------------------------------
 * Function: showSplashScreen(tex, secs)
 * Parameters:
 *   tex   The logo texture.
 *   secs  The time to display the logo, in seconds.
 *
 * Description:
 *   Shows a splash screen for the specfied amount of time.
 *
 * Usage:
 *   showSplashScreen(my_tex, 3.0f);
 *------------------------------------*/
static void showSplashScreen(textureT* splash_tex, float secs) {
    shaderT* splash_shader = gameResource("shader:splashscreen", ResShader);
    triMeshT* quad = createQuad(2.0f, 2.0f);

    useShader (splash_shader);
    useTexture(splash_tex, 0);

    timeT time = getTime();
    while (windowIsOpen()) {
        float elapsed = elapsedSecsSince(time);

        if (elapsed >= secs)
            break;

        // Default is fade=1.0, which gives a final value of 0.0, which is
        // 100% texture color and no fade.
        float fade = 1.0f;

        // Fade in the first second...
        if (elapsed < 1.0f)
            fade = sin(0.5f*elapsed*3.141592653);

        // ...and fade out the last 0.5 seconds.
        float time_left = secs - elapsed;
        if (time_left < 0.5f)
            fade = sin(time_left*3.141592653);

        // We multiply the fade value by itself to get sin(x)^2.
        fade *= fade;

        // Invert it to get the correct value.
        fade = 1.0f - fade;

        clearDisplay  (1.0f, 0.0f, 1.0f);
        setShaderParam("Fade", &fade);
        drawMesh      (quad);
        updateDisplay ();
    }

    freeMesh    (quad);
    useTexture  (NULL, 0);
    useShader   (NULL);
    deleteShader(splash_shader);
}

static arrayT* prealloc;
static int knull;

static float lol;
void frameFunc(float dt) {
    if (!prealloc) {
        prealloc = arrayNew(sizeof(gameEntityT*));
        for (int i = 0; i < 1000; i++) {
            gameEntityT* e = newAsteroidEntity();
            arrayAdd(prealloc, &e);
        }
    }

    //if (lol < 0)
    //    return;
    lol += dt;
    while (lol >= 0.5f) {
        //addEntityToGame(newAsteroidEntity());
        if (knull < 1000) {
            gameEntityT* e = *(gameEntityT**)arrayGet(prealloc, knull);
            addEntityToGame(e);
            knull++;
        }
        lol -= 0.1f;
        //lol = -1;
        //break;
    }
}

/*--------------------------------------
 * Function: main()
 *
 * Description:
 *   Program entry point.
 *
 * Usage:
 *   Don't call this function. :-)
 *------------------------------------*/
int main(void) {
    initGame("Asteroids", 1280, 720);

    loadResources();

    printIntroMessage();

    addGamePak(pakOpenArchive("data.pak", PakPassword));

    char* font_data = readGamePakFile("fonts/sector_034.ttf");
    int   num_bytes = gamePakFileSize("fonts/sector_034.ttf");
    loadFontFromMemory(font_data, num_bytes);
    free(font_data);

    showWindow();

#ifndef _DEBUG
    textureT* splash_tex = gameResource("texture:splashscreen0", ResTexture);
    showSplashScreen(splash_tex, 3.0f);
    freeTexture(splash_tex);
#endif // !_DEBUG

    addSubsystemToGame(newPhysicsSubsystem());
    addSubsystemToGame(newGraphicsSubsystem());

    gameEntityT* player_entity = newPlayerEntity();
    addEntityToGame(player_entity);

    gameMain(frameFunc);
}
