// Create and render a couple of planes.  
// Create some planes. Same basic setup as spheres but use planes instead. 
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
    vec3f cam_pos{5.f, 5.f, 5.f};
    vec3f cam_up{-1.f, 1.f, -1.f};
    vec3f cam_view{-1.0f, -1.f, -1.f};

    // We are going to build the planes
    // inside a box. That is we define a bounding box
    // that will limit the size of the planes. 
    // Geometrically a plane has infinite extent. 
    // the bounding box. 
    vec3f lower{-2.f,-2.f,-2.f}; // the "lower" corner of the box
    vec3f upper{2.f,2.f,2.f};    // the opposite corner of the box
    const box3f bounding_box(lower,upper);
    // make three planes that intersect at the origin
    // each plane orthogonal to one coordinate axis.
    // a vector to hold the coefficients of three planes
    std::vector<vec4f> coefs;
    std::vector<box3f> bboxes;
    // plane orthogonal to the x direction through origin
    coefs.push_back(vec4f(1.f,0.f,0.f,0.f));
    bboxes.push_back(bounding_box);
    // plane orthogonal to the y direction through origin
    coefs.push_back(vec4f(0.f,1.f,0.f,0.f));
    bboxes.push_back(bounding_box);
    // plane orthogonal to the z direction through origin
    coefs.push_back(vec4f(0.f,0.f,1.f,0.f));
    bboxes.push_back(bounding_box);
    
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

        // make a plane geometry
        ospray::cpp::Geometry planes("plane");
        planes.setParam("plane.coefficients", ospray::cpp::CopiedData(coefs));
        planes.setParam("plane.bounds", ospray::cpp::CopiedData(bboxes));
        planes.commit();
        // put the spheres into a model
        ospray::cpp::GeometricModel model(planes);
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
        ospray::cpp::Renderer renderer("ao");

        // complete setup of renderer, place setParam calls here
        // to change rendering.
        // Change the background color as an example. Spec rgb as vec3f.
        vec3f lightgray{0.1f,0.1f,0.1f};
        renderer.setParam("backgroundColor", lightgray); // 
        renderer.setParam("aoSamples", 100);
        renderer.setParam("aoDistance", 2.5f);
        renderer.commit();

        // create and setup framebuffer
        ospray::cpp::FrameBuffer framebuffer(
            imgSize.x, imgSize.y, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
        framebuffer.clear();

        // render one frame
        framebuffer.renderFrame(renderer, camera, world);

        // access framebuffer and write its content as PPM file
        uint32_t *fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
        rkcommon::utility::writePPM("firstPlaneFrameCpp.ppm", imgSize.x, imgSize.y, fb);
        framebuffer.unmap(fb);
        std::cout << "rendering initial frame to firstPlaneFrameCpp.ppm" << std::endl;

        // render 10 more frames, which are accumulated to result in a better
        // converged image
        for (int frames = 0; frames < 10; frames++)
          framebuffer.renderFrame(renderer, camera, world);

        fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
        rkcommon::utility::writePPM(
            "accumulatedPlaneFrameCpp.ppm", imgSize.x, imgSize.y, fb);
        framebuffer.unmap(fb);
        std::cout << "rendering 10 accumulated frames to accumulatedPlaneFrameCpp.ppm"
                  << std::endl;
    }
    // turn it off
    ospShutdown();
}
