/*------------------------------------------------------------------------------
 * File: sa14-game1.c
 * Created: June 7, 2015
 * Last changed: June 9, 2015
 *
 * Author(s): Philip Arvidsson (philip@philiparvidsson.com)
 *
 * Description:
 *   Huvudprogram f�r spelet. Vad nu det inneb�r. Det f�r tiden utvisa. :-)
 *
 *----------------------------------------------------------------------------*/

/*------------------------------------------------
 * INCLUDES
 *----------------------------------------------*/

#include "core/common.h"
#include "core/linmath.h"

#include "graphics.h"

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

/*------------------------------------------------
 * FUNCTIONS
 *----------------------------------------------*/

/*--------------------------------------
 * Function: printIntroMessage()
 *
 * Description:
 *   Skriver ut introduktionsmeddelandet.
 *------------------------------------*/
static void printIntroMessage(void) {
    printf("sa14-game1 v%s by %s\n\n\n", ProgramVersion, ProgramAuthors);
    printf("Some time in the not too distant future, this program will be a\n"
           "really cool game instead of this crap that does nothing...\n");
}

/*--------------------------------------
 * Function: main()
 *
 * Description:
 *   Programmets huvudfunktion.
 *------------------------------------*/
int main(void) {
    printIntroMessage();

    initGraphics("Main Window", 640, 640);
    setFrameRate(60.0f);

    shaderProgramADT shader_program = createShaderProgram();

    compileVertexShader(shader_program, readFile("shaders/test_shader.vert"));
    compileFragmentShader(shader_program, readFile("shaders/test_shader.frag"));

    geometryT *box1 = createBox(0.1f, 0.1f*1.6f, 0.1f*1.6f*1.6f);
    geometryT *box2 = createBox(0.2f, 0.2f, 0.2f);

    mat4x4 proj = mat4x4_perspective(-1.0f, 1.0f, -1.0f, 1.0f, -0.1f, -3.0f);
    mat4x4 view = mat4x4_lookAt(
        (vec3) { 0.0f, 0.5f, 0.0f },
        (vec3) { 0.0f, 0.0f, -1.0f },
        (vec3) { 0.0f, 1.0f, 0.0f }
    );


    float ff = 0.0f;
    while (windowIsOpen()) {
        box1->transform = mat4x4_mul(mat4x4_translateZ(-1.0f), mat4x4_rotateY(ff));
        box2->transform =
            mat4x4_mul(
       
                    mat4x4_translateZ(-1.0f),
                mat4x4_mul(
                    mat4x4_rotateX(ff*0.5),
                    mat4x4_rotateY(ff*0.3f)
                )
            );

        clearDisplay(0.0f, 0.0f, 0.4f);
        ff += 0.75 / 60.0f;

        useShaderProgram(shader_program);
        setShaderUniform("View", Matrix4Uniform, &view);
        setShaderUniform("Proj", Matrix4Uniform, &proj);

        setShaderUniform("Model", Matrix4Uniform, &box1->transform);
        drawGeometry(box1);

        //setShaderUniform("Model", Matrix4Uniform, &box2->transform);
        //drawGeometry(box2);
        
        updateDisplay();
    }

    deleteGeometry(box1);
    deleteGeometry(box2);
    deleteShaderProgram(shader_program);

    exitGraphics();
}
