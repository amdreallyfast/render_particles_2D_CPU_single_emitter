// the OpenGL version include also includes all previous versions
// Build note: Due to a minefield of preprocessor build flags, the gl_load.hpp must come after 
// the version include.
// Build note: Do NOT mistakenly include _int_gl_4_4.h.  That one doesn't define OpenGL stuff 
// first.
// Build note: Also need to link opengl32.lib (unknown directory, but VS seems to know where it 
// is, so don't put in an "Additional Library Directories" entry for it).
// Build note: Also need to link glload/lib/glloadD.lib.
#include "glload/include/glload/gl_4_4.h"
#include "glload/include/glload/gl_load.hpp"

// Build note: Must be included after OpenGL code (in this case, glload).
// Build note: Also need to link freeglut/lib/freeglutD.lib.  However, the linker will try to 
// find "freeglut.lib" (note the lack of "D") instead unless the following preprocessor 
// directives are set either here or in the source-building command line (VS has a
// "Preprocessor" section under "C/C++" for preprocessor definitions).
// Build note: Also need to link winmm.lib (VS seems to know where it is, so don't put in an 
// "Additional Library Directories" entry).
#define FREEGLUT_STATIC
#define _LIB
#define FREEGLUT_LIB_PRAGMAS 0
#include "freeglut/include/GL/freeglut.h"

// this linking approach is very useful for portable, crude, barebones demo code, but it is 
// better to link through the project building properties
#pragma comment(lib, "glload/lib/glloadD.lib")
#pragma comment(lib, "opengl32.lib")            // needed for glload::LoadFunctions()
#pragma comment(lib, "freeglut/lib/freeglutD.lib")
#ifdef WIN32
#pragma comment(lib, "winmm.lib")               // Windows-specific; freeglut needs it
#endif

// for printf(...)
#include <stdio.h>

// for basic OpenGL stuff
#include "OpenGlErrorHandling.h"
//#include "GenerateShader.h"
#include "ShaderStorage.h"

// for drawing shapes
#include "GeometryData.h"
#include "PrimitiveGeneration.h"

// for particles
#include "ParticleRegionCircle.h"
#include "ParticleRegionPolygon.h"
#include "ParticleEmitterPoint.h"
#include "ParticleEmitterBar.h"
#include "ParticleStorage.h"
#include "ParticleUpdater.h"

// for moving the shapes around in window space
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// in a bigger program, ??where would this be stored??
ShaderStorage gShaderStorage;
// in a bigger program, this would be stored in some kind of shader storage
//GLuint gProgramId;

// in a bigger program, uniform locations would probably be stored in the same place as the 
// shader programs
GLint gUnifMatrixTransform;

// in a bigger program, geometry data would be stored in some kind of "scene" or in a renderer
// or behind door number 3 so that collision boxes could get at the vertex data
GeometryData gCircleGeometry;
GeometryData gPolygonGeometry;

// in a bigger program, this would somehow be encapsulated and associated with both the circle
// geometry and the circle particle region, and ditto for the polygon
glm::mat4 gCircleTranslateMatrix;
glm::mat4 gPolygonTranslateMatrix;

// in a bigger program, ??where would particle stuff be stored??
// Note: I made the updater only use one region and one emitter.  I suggest that there only ever
// be 1 region per updater, but my code can be adapted to use multiple emitters.
IParticleRegion *gpParticleRegionCircle;
IParticleEmitter *gpParticleEmitterForCircle;
ParticleUpdater gParticleUpdaterForCircle;

IParticleRegion *gpParticleRegionPolygon;
IParticleEmitter *gpParticleEmitterForPolygon;
ParticleUpdater gParticleUpdaterForPolygon;

// divide between the circle and the polygon regions
const unsigned int MAX_PARTICLE_COUNT = 10000;
ParticleStorage gParticleStorage;



// TODO: header
// a batch of hard coding to clean up Init()
void CreateCircularParticleThing(const glm::mat4 &translateMatrix, const GLuint programId)
{
    GenerateCircle(&gCircleGeometry);   // hard coded radius of 0.25f
    gCircleGeometry.Init(programId);

    // the circle starts centered on the origin and the translate matrix will move it
    // Note: The 1.0f makes it translatable.
    glm::vec4 circleCenter(+0.0f, +0.0f, +0.0f, 1.0f);  
    gCircleTranslateMatrix = glm::translate(glm::mat4(), glm::vec3(+0.3f, +0.3f, 0.0f));
    circleCenter = gCircleTranslateMatrix * circleCenter;
    gpParticleRegionCircle = new ParticleRegionCircle(
        glm::vec2(circleCenter.x, circleCenter.y), 0.25f);

    // stick the emitter bar on the left side of the circle, have it emit right, and make the 
    // particles slow compared to the point emitter 
    // Note: Have to use vec4s instead of vec2s because glm::translate(...) only spits out mat4.
    glm::vec4 barP1 = translateMatrix * glm::vec4(-0.2f, +0.1f, 0.0f, 1.0f);
    glm::vec4 barP2 = translateMatrix * glm::vec4(-0.2f, -0.1f, 0.0f, 1.0f);
    glm::vec2 emitDirection(+1.0f, 0.0f);
    float minVel = 0.1f;
    float maxVel = 0.3f;
    gpParticleEmitterForCircle = new ParticleEmitterBar(glm::vec2(barP1.x, barP1.y), 
        glm::vec2(barP2.x, barP2.y), emitDirection, minVel, maxVel);
}

// TODO: header
// a batch of hard coding to clean up Init()
void CreatePolygonParticleThing(const glm::mat4 &translateMatrix, const GLuint programId)
{
    std::vector<glm::vec2> polygonCorners;
    polygonCorners.push_back(glm::vec2(-0.25f, -0.5f));
    polygonCorners.push_back(glm::vec2(+0.25f, -0.5f));
    polygonCorners.push_back(glm::vec2(+0.5f, +0.25f));
    polygonCorners.push_back(glm::vec2(-0.5f, +0.25f));
    gPolygonTranslateMatrix = glm::translate(glm::mat4(), glm::vec3(-0.3f, -0.3f, 0.0f));
    for (size_t i = 0; i < polygonCorners.size(); i++)
    {
        // translate the polygon's corners
        // Note: glm::translate only spits out a mat4, so the vec2 has to be made into a 
        // translatable vec4 (the 1.0f at the end makes it translatable), transformed, then 
        // turned back into a vec2;
        glm::vec4 v4 = glm::vec4(polygonCorners[i], 0.0f, 1.0f);
        v4 = gPolygonTranslateMatrix * v4;
        polygonCorners[i] = glm::vec2(v4.x, v4.y);
    }

    GeneratePolygonWireframe(&gPolygonGeometry, polygonCorners, false);
    gPolygonGeometry.Init(programId);

    gpParticleRegionPolygon = new ParticleRegionPolygon(polygonCorners);

    // stick the emitter point in the upper right corner of the polygon, and make the particles
    // fast compared to the bar emitter
    glm::vec4 emitterPoint = translateMatrix * glm::vec4(+0.4f, +0.2f, 0.0f, 1.0f);
    float minVel = 0.3f;
    float maxVel = 0.5f;
    gpParticleEmitterForPolygon = new ParticleEmitterPoint(glm::vec2(emitterPoint.x, emitterPoint.y), 
        minVel, maxVel);
}

/*-----------------------------------------------------------------------------------------------
Description:
    Governs window creation, the initial OpenGL configuration (face culling, depth mask, even
    though this is a 2D demo and that stuff won't be of concern), the creation of geometry, and
    the creation of a texture.
Parameters:
    argc    (From main(...)) The number of char * items in argv.  For glut's initialization.
    argv    (From main(...)) A collection of argument strings.  For glut's initialization.
Returns:
    False if something went wrong during initialization, otherwise true;
Exception:  Safe
Creator:    John Cox (3-7-2016)
-----------------------------------------------------------------------------------------------*/
void Init()
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);

    //gProgramId = GenerateShaderProgram();
    gShaderStorage.NewShader("particles");
    gShaderStorage.AddShaderFile("particles", "shaderParticle.vert", GL_VERTEX_SHADER);
    gShaderStorage.AddShaderFile("particles", "shaderParticle.frag", GL_FRAGMENT_SHADER);
    gShaderStorage.LinkShader("particles");
    GLuint particleProgramId = gShaderStorage.GetShaderProgram("particles");

    //gUnifMatrixTransform = glGetUniformLocation(gProgramId, "translateMatrixWindowSpace");

    gCircleTranslateMatrix = glm::translate(glm::mat4(), glm::vec3(+0.3f, +0.3f, 0.0f));
    CreateCircularParticleThing(gCircleTranslateMatrix, particleProgramId);

    gPolygonTranslateMatrix = glm::translate(glm::mat4(), glm::vec3(-0.3f, -0.3f, 0.0f));
    CreatePolygonParticleThing(gPolygonTranslateMatrix, particleProgramId);

    //// make a circle and a bar particle emitter inside it
    //// - circle geometry for drawing a circle
    //// - circle particle region of same size
    //// 
    //GenerateCircle(&gCircleGeometry);   // hard coded radius of 0.25f
    //gCircleGeometry.Init(gProgramId);
    //glm::vec4 circleCenter(+0.0f, +0.0f, +0.0f, 1.0f);  // 1.0f makes it translatable
    //gCircleTranslateMatrix = glm::translate(glm::mat4(), glm::vec3(+0.3f, +0.3f, 0.0f));
    //circleCenter = gCircleTranslateMatrix * circleCenter;
    //gpParticleRegion1 = new ParticleRegionCircle(
    //    glm::vec2(circleCenter.x, circleCenter.y), 0.25f);
    //glm::vec2 barP1 = 
    //gpParticleEmitter1 = new ParticleEmitterBar

        //GenerateCircle(&gCircle);

    // this is a hack, but it correctly moves the particle region's boundaries to match where 
    // the shape will be drawn 
    //std::vector<glm::vec2> polygonCorners;
    //polygonCorners.push_back(glm::vec2(-0.25f, -0.5f));
    //polygonCorners.push_back(glm::vec2(+0.25f, -0.5f));
    //polygonCorners.push_back(glm::vec2(+0.5f, +0.25f));
    //polygonCorners.push_back(glm::vec2(-0.5f, +0.25f));
    //gPolygonTranslateMatrix = glm::translate(glm::mat4(), glm::vec3(-0.3f, -0.3f, 0.0f));
    //for (size_t i = 0; i < polygonCorners.size(); i++)
    //{
    //    // translate the polygon's corners
    //    // Note: glm::translate only spits out a mat4, so the vec2 has to be made into a 
    //    // translatable vec4 (the 1.0f at the end makes it translatable), transformed, then 
    //    // turned back into a vec2;
    //    glm::vec4 v4 = glm::vec4(polygonCorners[i], 0.0f, 1.0f);
    //    v4 = gPolygonTranslateMatrix * v4;
    //    polygonCorners[i] = glm::vec2(v4.x, v4.y);
    //}
    //gpParticleRegion = new ParticleRegionPolygon(polygonCorners);


    ////GeneratePolygonWireframe(&gPolygon, polygonCorners, false);

    ////InitializeGeometry(gProgramId, &gPolygon);
    ////InitializeGeometry(gProgramId, &gCircle);

    //// make a point emitter at the center of a circular region of the same location and size as 
    //// the circle primitive (currently (7-4-2016) hard-coded as 0.25 radius)

    //gpParticleEmitter = new ParticleEmitterPoint(glm::vec2(+0.0f, +0.0f), 0.1f, 0.5f);
    //gpParticleEmitter = new ParticleEmitterBar(glm::vec2(-0.2f, -0.25f), glm::vec2(+0.15, +0.2f), glm::vec2(-0.1f, -0.6f), 0.1f, 0.5f);


    gParticleStorage.Init(particleProgramId, MAX_PARTICLE_COUNT);
    gParticleUpdaterForCircle.SetRegion(gpParticleRegionCircle);
    gParticleUpdaterForCircle.SetEmitter(gpParticleEmitterForCircle, 10);

    // start all particles at the emitter's orign
    for (size_t particleCount = 0; particleCount < MAX_PARTICLE_COUNT; particleCount++)
    {
        gpParticleEmitterForCircle->ResetParticle(&(gParticleStorage._allParticles[particleCount]));
    }
}

/*-----------------------------------------------------------------------------------------------
Description:
    This is the rendering function.  It tells OpenGL to clear out some color and depth buffers,
    to set up the data to draw, to draw than stuff, and to report any errors that it came across.
    This is not a user-called function.

    This function is registered with glutDisplayFunc(...) during glut's initialization.
Parameters: None
Returns:    None
Exception:  Safe
Creator:    John Cox (2-13-2016)
-----------------------------------------------------------------------------------------------*/
void Display()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 translateMatrix;

    gParticleUpdaterForCircle.Update(gParticleStorage._allParticles, 0, 
        gParticleStorage._allParticles.size(), 0.01f);

    GLuint particleProgramId = gShaderStorage.GetShaderProgram("particles");
    glUseProgram(particleProgramId);

    //static unsigned int frameCounter = 0;
    //frameCounter++;
    //if (frameCounter % 100 == 0)
    //{
    //    printf("frame count = %d\n", frameCounter);
    //}

    //// put the circle up and to the right
    //// Note: Remember that this program is "barebones", so translation must be in window space 
    //// ([-1,+1] on X and Y).
    //translateMatrix = glm::translate(glm::mat4(), glm::vec3(+0.3f, +0.3f, 0.0f));
    //glUniformMatrix4fv(gUnifMatrixTransform, 1, GL_FALSE, glm::value_ptr(translateMatrix));
    //glBindVertexArray(gCircle._vaoId);
    //glDrawElements(gCircle._drawStyle, gCircle._indices.size(), GL_UNSIGNED_SHORT, 0);

    //// put the polygon down and to the left
    //translateMatrix = glm::translate(glm::mat4(), glm::vec3(-0.3f, -0.3f, 0.0f));
    //glUniformMatrix4fv(gUnifMatrixTransform, 1, GL_FALSE, glm::value_ptr(translateMatrix));
    //glBindVertexArray(gPolygon._vaoId);
    //glDrawElements(gPolygon._drawStyle, gPolygon._indices.size(), GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(gParticleStorage._vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, gParticleStorage._arrayBufferId);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gParticleStorage._sizeBytes, gParticleStorage._allParticles.data());
    glDrawArrays(gParticleStorage._drawStyle, 0, gParticleStorage._allParticles.size());



    // TODO: remove everything from this project that has to do with textures; they aren't used in this demo




    // clean up bindings
    // Note: Don't bother going through each texture unit and unbinding it.  While unbinding is
    // a good preactice, texture units are an exception.  Texture switching means that the 
    // texture cache is no longer valid, so whatever GPU processors are handling the texture 
    // will experience cache misses until the cache "warms up" again.  Lesson: Make as few 
    // texture switches as possible.
    glUseProgram(0);

    // tell the GPU to swap out the displayed buffer with the one that was just rendered
    glutSwapBuffers();

    // tell glut to call this display() function again on the next iteration of the main loop
    // Note: https://www.opengl.org/discussion_boards/showthread.php/168717-I-dont-understand-what-glutPostRedisplay()-does
    // Also Note: This display() function will also be registered to run if the window is moved
    // or if the viewport is resized.  If glutPostRedisplay() is not called, then as long as the
    // window stays put and doesn't resize, display() won't be called again (tested with 
    // debugging).
    // Also Also Note: It doesn't matter where this is called in this function.  It sets a flag
    // for glut's main loop and doesn't actually call the registered display function, but I 
    // got into the habbit of calling it at the end.
    glutPostRedisplay();
}

/*-----------------------------------------------------------------------------------------------
Description:
    Tell's OpenGL to resize the viewport based on the arguments provided.  This is an 
    opportunity to call glViewport or glScissor to keep up with the change in size.
    
    This is not a user-called function.  It is registered with glutReshapeFunc(...) during 
    glut's initialization.
Parameters:
    w   The width of the window in pixels.
    h   The height of the window in pixels.
Returns:    None
Exception:  Safe
Creator:    John Cox (2-13-2016)
-----------------------------------------------------------------------------------------------*/
void Reshape(int w, int h)
{
    glViewport(0, 0, w, h);
}

/*-----------------------------------------------------------------------------------------------
Description:
    Executes when the user presses a key on the keyboard.

    This is not a user-called function.  It is registered with glutKeyboardFunc(...) during
    glut's initialization.

    Note: Although the x and y arguments are for the mouse's current position, this function does
    not respond to mouse presses.
Parameters:
    key     The ASCII code of the key that was pressed (ex: ESC key is 27)
    x       The horizontal viewport coordinates of the mouse's current position.
    y       The vertical window coordinates of the mouse's current position
Returns:    None
Exception:  Safe
Creator:    John Cox (2-13-2016)
-----------------------------------------------------------------------------------------------*/
void Keyboard(unsigned char key, int x, int y)
{
    // this statement is mostly to get ride of an "unreferenced parameter" warning
    printf("keyboard: x = %d, y = %d\n", x, y);
    switch (key)
    {
    case 27:
    {
        // ESC key
        glutLeaveMainLoop();
        return;
    }
    default:
        break;
    }
}

/*-----------------------------------------------------------------------------------------------
Description:
    I don't know what this does, but I've kept it around since early times, and this was the 
    comment given with it:
    
    "Called before FreeGLUT is initialized. It should return the FreeGLUT display mode flags 
    that you want to use. The initial value are the standard ones used by the framework. You can 
    modify it or just return you own set.  This function can also set the width/height of the 
    window. The initial value of these variables is the default, but you can change it."
Parameters:
    displayMode     ??
    width           ??
    height          ??
Returns:
    ??what??
Exception:  Safe
Creator:    John Cox (2-13-2016)
-----------------------------------------------------------------------------------------------*/
unsigned int Defaults(unsigned int displayMode, int &width, int &height) 
{
    // this statement is mostly to get ride of an "unreferenced parameter" warning
    printf("Defaults: width = %d, height = %d\n", width, height);
    return displayMode; 
}

/*-----------------------------------------------------------------------------------------------
Description:
    Cleans up GPU memory.  This might happen when the processes die, but be a good memory steward
    and clean up properly.

    Note: A big program would have the textures, program IDs, buffers, and other things 
    encapsulated somewhere, and each other those would do the cleanup, but in this barebones 
    demo, I can just clean up everything here.
Parameters: None
Returns:    None
Exception:  Safe
Creator:    John Cox (2-13-2016)
-----------------------------------------------------------------------------------------------*/
void CleanupAll()
{
    gShaderStorage.DeleteProgram("particles");

    // these deletion functions need the buffer ID, but they take a (void *) for the second 
    // argument in the event that the user has an array of IDs (legacy OpenGL stuff that isn't 
    // used much anymore, if at all), so pass in the buffer ID's address and tell it to delete 1
    // Note: The second argument is treated like an array, so if I were to pass in the address 
    // of a single GLuint and tell it to delete 2, then it will either (1) blow up or 
    // (2) silently delete something I didn't want.  Both are bad, so treat it nice.
    // Also Note: If I attempt to delete an ID that has already been deleted, that is ok.  OpenGL
    // will silently swallow that.
    glDeleteBuffers(1, &gCircleGeometry._arrayBufferId);
    glDeleteBuffers(1, &gCircleGeometry._elementBufferId);
    glDeleteVertexArrays(1, &gCircleGeometry._vaoId);
    glDeleteBuffers(1, &gPolygonGeometry._arrayBufferId);
    glDeleteBuffers(1, &gPolygonGeometry._elementBufferId);
    glDeleteVertexArrays(1, &gPolygonGeometry._vaoId);

    delete(gpParticleRegionCircle);
    delete(gpParticleRegionPolygon);
    delete(gpParticleEmitterForCircle);
    delete(gpParticleEmitterForPolygon);
}

/*-----------------------------------------------------------------------------------------------
Description:
    Program start and end.
Parameters:
    argc    The number of strings in argv.
    argv    A pointer to an array of null-terminated, C-style strings.
Returns:
    0 if program ended well, which it always does or it crashes outright, so returning 0 is fine
Exception:  Safe
Creator:    John Cox (2-13-2016)
-----------------------------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    glutInit(&argc, argv);

    int width = 500;
    int height = 500;
    unsigned int displayMode = GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH | GLUT_STENCIL;
    displayMode = Defaults(displayMode, width, height);

    glutInitDisplayMode(displayMode);
    glutInitContextVersion(4, 4);
    glutInitContextProfile(GLUT_CORE_PROFILE);

    // enable this for automatic message reporting (see OpenGlErrorHandling.cpp)
#define DEBUG
#ifdef DEBUG
    glutInitContextFlags(GLUT_DEBUG);
#endif

    glutInitWindowSize(width, height);
    glutInitWindowPosition(300, 200);
    int window = glutCreateWindow(argv[0]);

    glload::LoadTest glLoadGood = glload::LoadFunctions();
    // ??check return value??

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

    if (!glload::IsVersionGEQ(3, 3))
    {
        printf("Your OpenGL version is %i, %i. You must have at least OpenGL 3.3 to run this tutorial.\n",
            glload::GetMajorVersion(), glload::GetMinorVersion());
        glutDestroyWindow(window);
        return 0;
    }

    if (glext_ARB_debug_output)
    {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        glDebugMessageCallbackARB(DebugFunc, (void*)15);
    }

    Init();

    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutMainLoop();

    CleanupAll();

    return 0;
}