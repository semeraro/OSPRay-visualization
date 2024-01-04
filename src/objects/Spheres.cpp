// Create and render some Spheres.  
// Simple 8 sphere cube viewed from above.
// scivis renderer, ambient light, 
// First the include files. Including some windows defines.
//
// The basics
//
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#ifdef _WIN32
#define NOMINMAX
#include <conio.h>
#include <malloc.h>
#include <windows.h>
#else
#include <alloca.h>
#endif
#include <vector>
// Ospray stuff c++ flavor
#include "ospray/ospray_cpp.h"
#include "ospray/ospray_cpp/ext/rkcommon.h"
#include "rkcommon/utility/SaveImage.h"
using namespace rkcommon::math;
int main(int argc, const char** argv) {
    // set up.
    // image size
    vec2i imgSize;
    imgSize.x = 1024; // width
    imgSize.y = 768; // height

    // camera
    vec3f cam_pos{0.f, 0.f, 10.f};
    vec3f cam_up{0.f, 1.f, 0.f};
    vec3f cam_view{0.0f, 0.f, -1.f};

    // sphere data
    // the x,y,z of each sphere position.
    // Let's arrange 8 spheres in a cube 
    // centered at the origin
    std::vector<vec3f> position = {
        vec3f(1.0f, 1.0f, 1.0f),
        vec3f(1.0f, 1.0f, -1.0f),
        vec3f(1.0f,-1.0f, 1.0f),
        vec3f(1.0f,-1.0f, -1.0f),
        vec3f(-1.0f, 1.0f, 1.0f),
        vec3f(-1.0f, 1.0f, -1.0f),
        vec3f(-1.0f,-1.0f, 1.0f),
        vec3f(-1.0f,-1.0f, -1.0f)};
    // Default sphere radius
    float radius = 0.55f;
    // or set radius of individual spheres
    std::vector<float> radi = {
        0.1,0.12,0.13,0.14,0.15,0.16,0.17,0.18
    };
#ifdef _WIN32
    bool waitForKey = false;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        // detect standalone console: cursor at (0,0)?
        waitForKey = csbi.dwCursorPosition.X == 0 && csbi.dwCursorPosition.Y == 0;
    }
#endif

    OSPError init_error = ospInit(&argc, argv);
    if (init_error != OSP_NO_ERROR)
        return init_error;
    // ospray related calls go in here. The first bunch of calls creates 
    // various objects, cameras, lights, geometry etc. Some things, geometry and lights,
    // are placed in a world. The objects are "commited" once they are complete.
    // when all the stuff is in the world that too is committed. 
    {
        // create and setup camera
        ospray::cpp::Camera camera("perspective");
        camera.setParam("aspect", imgSize.x / (float)imgSize.y);
        camera.setParam("position", cam_pos);
        camera.setParam("direction", cam_view);
        camera.setParam("up", cam_up);
        camera.commit(); // commit each object to indicate modifications are done

        // create and setup model and shperes
        // create a geometry object to contain the spheres. As yet no sphere count.
        ospray::cpp::Geometry Spheres("sphere");
        // set the position of the spheres
        Spheres.setParam("sphere.position", ospray::cpp::CopiedData(position));
        // set the default radius of the spheres
        // all the spheres will be this radius if no individual
        // radius is specified.
        Spheres.setParam("radius", radius);
        // if uncommented the next line sets the individual radius of 
        // each sphere. Overrides the default radius.
        Spheres.setParam("sphere.radius", ospray::cpp::CopiedData(radi));
        Spheres.commit();

        // put the spheres into a model
        ospray::cpp::GeometricModel model(Spheres);
        model.commit();

        // put the model into a group (collection of models)
        ospray::cpp::Group group;
        group.setParam("geometry", ospray::cpp::CopiedData(model));
        group.commit();

        // put the group into an instance (give the group a world transform)
        ospray::cpp::Instance instance(group);
        instance.commit();

        // put the instance in the world
        ospray::cpp::World world;
        world.setParam("instance", ospray::cpp::CopiedData(instance));
        // create and setup light. No light no image. 
        ospray::cpp::Light light("ambient");
        light.commit();

        world.setParam("light", ospray::cpp::CopiedData(light));
        world.commit();

         // create renderer, choose Scientific Visualization renderer
        ospray::cpp::Renderer renderer("scivis");

        // complete setup of renderer, place setParam calls here
        // to change rendering.
        // Change the background color as an example. Spec rgb as vec3f.
        vec3f lightgray{0.1f,0.1f,0.1f};
        renderer.setParam("backgroundColor", lightgray); // 
        renderer.commit();

        // create and setup framebuffer
        ospray::cpp::FrameBuffer framebuffer(
            imgSize.x, imgSize.y, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
        framebuffer.clear();

        // render one frame
        framebuffer.renderFrame(renderer, camera, world);

        // access framebuffer and write its content as PPM file
        uint32_t *fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
        rkcommon::utility::writePPM("firstSphereFrameCpp.ppm", imgSize.x, imgSize.y, fb);
        framebuffer.unmap(fb);
        std::cout << "rendering initial frame to firstSphereFrameCpp.ppm" << std::endl;

        // render 10 more frames, which are accumulated to result in a better
        // converged image
        for (int frames = 0; frames < 10; frames++)
          framebuffer.renderFrame(renderer, camera, world);

        fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
        rkcommon::utility::writePPM(
            "accumulatedSphereFrameCpp.ppm", imgSize.x, imgSize.y, fb);
        framebuffer.unmap(fb);
        std::cout << "rendering 10 accumulated frames to accumulatedSphereFrameCpp.ppm"
                  << std::endl;
    }
    // turn it off
    ospShutdown();
}
