// Experiments with the mesh object. 
// First create a flat 2d mesh in the xy plane
// specify triangles, vertex normals and vertex texture coords.
// The mesh covers a unit square. There are I points in x and J points in y
// direction. 
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
// Ospray stuff c++ flavor
#include "ospray/ospray_cpp.h"
#include "ospray/ospray_cpp/ext/rkcommon.h"
#include "rkcommon/utility/SaveImage.h"
#include "rkcommon/utility/random.h"
using namespace rkcommon::math;
int main(int argc, const char** argv) {
    int I = 10;
    int J = 10;
    int numverts = I*J;
    // two triangles per quad in the mesh
    int numtris = (I-1)*(J-1)*2;
    float dx = 1.0f/float(I-1);
    float dy = 1.0f/float(J-1);
    std::vector<vec3f> vertex;
    std::vector<vec3f> normal;
    std::vector<vec3ui> index;
    std::vector<vec2f> texcoords;
    float x,y,z;
    // insert vertices
    for(int j=0;j<J;j++) 
        for(int i=0;i<I;i++){
            x = 0.0f+i*dx;
            y = 0.0f+j*dy;
            z = 0.0;
            vertex.emplace_back(x,y,z); // vertex coords
            normal.emplace_back(0.f,0.f,-1.f); // normal at vertex
            texcoords.emplace_back(x,y); // texture coordinates
            if((i<(I-1)) && (j<(J-1))) { // insert triangles
                unsigned int k1=i+j*I;
                unsigned int k2=k1+1;
                unsigned int k3=k1+I;
                index.emplace_back(k1,k2,k3); // bottom left triangle
                k1=k2;
                k2=k1+I;
                k3=k2-1;
                index.emplace_back(k1,k2,k3); // top right triangle
            }
        }
    vec2i imgSize;
    imgSize.x = 1024; // width
    imgSize.y = 1024; // height
    // camera
    vec3f cam_pos{0.f, 0.f, 2.f};
    vec3f cam_up{0.f, 1.f, 0.f};
    vec3f cam_view{0.0f, 0.f,-1.f};
    // we have the required data now lets build a mesh object.
    // going to need to make some ospray calls.

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
    {
        // create and setup model and mesh
        // first the mesh
        ospray::cpp::Geometry mesh("mesh");
        mesh.setParam("vertex.position", ospray::cpp::CopiedData(vertex));
        mesh.setParam("vertex.normal", ospray::cpp::CopiedData(normal));
        mesh.setParam("index", ospray::cpp::CopiedData(index));
        mesh.setParam("vertex.texcoord",ospray::cpp::CopiedData(texcoords));
        mesh.commit();
        // geometric model
        ospray::cpp::GeometricModel geomodel(mesh);
        geomodel.commit();
        // group
        ospray::cpp::Group meshgroup;
        meshgroup.setParam("geometry", ospray::cpp::CopiedData(geomodel));
        meshgroup.commit();
        // instance
        ospray::cpp::Instance instance(meshgroup);
        affine3f trans = affine3f::translate(vec3f(-0.5f,-0.5f,0.0f)); // translate the square to center on the origin. 
        instance.setParam("transform",trans);
        instance.commit();
        // world
        ospray::cpp::World world;
        world.setParam("instance", ospray::cpp::CopiedData(instance));
        ospray::cpp::Light light("ambient");
        light.commit();
        world.setParam("light", ospray::cpp::CopiedData(light));
        world.commit();
        //
        // -------------------------------------------------------------------
        //
        // cameras lights etc...
        //
        ospray::cpp::Camera camera("perspective");
        camera.setParam("aspect", imgSize.x / (float)imgSize.y);
        camera.setParam("position", cam_pos);
        camera.setParam("direction", cam_view);
        camera.setParam("up", cam_up);
        camera.commit();
        ospray::cpp::Renderer renderer("scivis");
        // complete setup of renderer
        renderer.setParam("aoSamples", 1);
        renderer.setParam("backgroundColor", 0.5f); // white, transparent
        renderer.commit();
        //
        ospray::cpp::FrameBuffer framebuffer(
            imgSize.x, imgSize.y, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
        framebuffer.clear();

        // render one frame
        framebuffer.renderFrame(renderer, camera, world);

        // access framebuffer and write its content as PPM file
        uint32_t *fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
        rkcommon::utility::writePPM("planemesh.ppm", imgSize.x, imgSize.y, fb);
        framebuffer.unmap(fb);
        std::cout << "rendering initial frame to planemesh.ppm" << std::endl;
    }
    ospShutdown();
#ifdef _WIN32
    if (waitForKey) {
    printf("\n\tpress any key to exit");
    _getch();
    }
#endif
    return 0;
}