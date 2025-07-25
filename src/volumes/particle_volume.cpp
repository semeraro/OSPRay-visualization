//
// Create a particle based volume dataset. 
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
#include <random>
#include "spherical_data.h"
#include "rkcommon/utility/SaveImage.h"
int main(int argc, const char** argv) {
    vec2i imgSize;
    imgSize.x = 1024; // width
    imgSize.y = 768; // height
    // camera
    vec3f cam_pos{0.f, 0.f, 3.f};
    vec3f cam_up{0.f, 1.f, 0.f};
    vec3f cam_view{0.0f, 0.f, -1.f};
    // build a particle dataset on the unit cube 
    // 
    long num_particles;
    vec3i dimensions(32,32,32);
    vec3f center(0.0f,0.0f,0.0f);
    float rmax = 0.4f;  // points farther than rmax from the center have value 0. 
    float maxq = 1.0f; // scale factor
#ifdef _WIN32
    bool waitForKey = false;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        // detect standalone console: cursor at (0,0)?
        waitForKey = csbi.dwCursorPosition.X == 0 && csbi.dwCursorPosition.Y == 0;
    }
#endif
    // build the ospray volume

    OSPError init_error = ospInit(&argc, argv);
    if (init_error != OSP_NO_ERROR)
        return init_error;
{
    // create and setup camera
        ospray::cpp::Camera camera("perspective");
        camera.setParam("aspect", imgSize.x / (float)imgSize.y);
        camera.setParam("position", cam_pos);
        camera.setParam("direction", cam_view);
        camera.setParam("up", cam_up);
        camera.commit();
    float *field = spherical_data(dimensions,center,rmax,maxq);
    //for(int i = 0; i < dimensions.long_product(); i++) {
    //    std::cout << field[i] << std::endl;
    //}
    ospray::cpp::Volume Vol("structuredRegular");
    
    ospray::cpp::SharedData voldata(field,OSP_FLOAT,dimensions);
    voldata.commit();
    Vol.setParam("data",voldata);
    Vol.setParam("dimensions",dimensions);
    vec3f origin(-0.5f,-0.5f,-0.5f);
    vec3f spacing(vec3f(1.0f,1.0f,1.0f)/(dimensions -1));
    std::cout << "origin: " << origin << " spacing " << spacing << std::endl;
    Vol.setParam("gridOrigin",origin);
    Vol.setParam("gridSpacing",spacing);
    Vol.commit();
    std::vector<box3f> boxes;
    boxes.emplace_back(origin,origin+vec3f(1.0f));
    box3f volbounds(origin,origin+vec3f(1.0f));
    ospray::cpp::Geometry domain("box");
    domain.setParam("box",ospray::cpp::SharedData(volbounds));
    domain.commit();
    ospray::cpp::GeometricModel domainmodel(domain);
    domainmodel.commit();

    // transfer function
    ospray::cpp::TransferFunction tf("piecewiseLinear");
    std::vector<vec3f> colors = {
        vec3f(0.0f,0.0f,0.0f),
        vec3f(1.0f,1.0f,1.0f)
    };
    std::vector<float> opacity = {0.0f,1.0f};
    range1f valueRange = range1f(0.0f,11.0f);
    tf.setParam("color",ospray::cpp::CopiedData(colors));
    tf.setParam("opacity",ospray::cpp::SharedData(opacity));
    tf.setParam("value",valueRange);
    tf.commit();
    // model
    ospray::cpp::VolumetricModel volmod(Vol);
    volmod.setParam("transferFunction",tf);
    volmod.setParam("samplingRate",1.0f);
    volmod.commit();
    // put the model into a group (collection of models)
        ospray::cpp::Group group;
        group.setParam("volume", ospray::cpp::SharedData(volmod));
        //group.setParam("geometry",ospray::cpp::CopiedData(domainmodel));
        group.commit();

        // put the group into an instance (give the group a world transform)
        ospray::cpp::Instance instance(group);
        instance.commit();
        ospray::cpp::Light aolight("ambient");
        aolight.commit();
        // put the instance in the world
        ospray::cpp::World world;
        //world.setParam("light", ospray::cpp::CopiedData(aolight));
        world.setParam("instance", ospray::cpp::SharedData(instance));
        world.commit();
     // create renderer, choose Scientific Visualization renderer
        ospray::cpp::Renderer renderer("scivis");
        // complete setup of renderer, place setParam calls here
        // to change rendering.
        // Change the background color as an example. Spec rgb as vec3f.
        vec3f lightgray{0.1f,0.1f,0.1f};
        //renderer.setParam("volumeSamplingRate",1.0f);
        //renderer.setParam("backgroundColor", lightgray); //
        renderer.setParam("pixelSamples",8); 
        renderer.commit();
        // create and setup framebuffer
        ospray::cpp::FrameBuffer framebuffer(
            imgSize.x, imgSize.y, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
        framebuffer.clear();

        // render one frame
        framebuffer.renderFrame(renderer, camera, world);

        // access framebuffer and write its content as PPM file
        uint32_t *fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
        rkcommon::utility::writePPM("volumerender.ppm", imgSize.x, imgSize.y, fb);
        framebuffer.unmap(fb);
        std::cout << "rendering initial frame to volumerender.ppm" << std::endl;
        // render 10 more frames, which are accumulated to result in a better
        // converged image
        for (int frames = 0; frames < 10; frames++)
          std::cout << "frame " << frames << std::endl;
          framebuffer.renderFrame(renderer, camera, world);

        fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
        rkcommon::utility::writePPM(
            "accumulatedvolumerender.ppm", imgSize.x, imgSize.y, fb);
        framebuffer.unmap(fb);
        std::cout << "rendering 10 accumulated frames to accumulatedvolumerender.ppm"
                  << std::endl;
}
// turn off
    ospShutdown();
#ifdef _WIN32
    if (waitForKey) {
    printf("\n\tpress any key to exit");
    _getch();
    }
#endif
    return 0;
}