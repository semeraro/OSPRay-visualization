// a routine to generate sphere data on a regular grid.
// vec3i dimensions - number of vertices in each coordinate direction.
// vec3f center - coordinates of the center of the spherical domain.
// float rmax - physical size of the domain outside of which the value of
//           the scalar goes to zero. 
// float d0 - a scale factor for the field. data = d0/(location-center) where
//            location is the coordinate of the field point.
// float* - pointer to the data in row major order. 
//
// Data is generated at the vertices of the grid. The value of the density is
// d0 at the center of the domain and falls off linearly to zero at a distance
// rmax from the center. The physical extent of the domain itself is the unit
// cube centered on the origin. 
// First the include files. Including some windows defines.

// a bit of ospray stuff so we can use rkcommon
#include "ospray/ospray_cpp.h"
#include "ospray/ospray_cpp/ext/rkcommon.h"
using namespace rkcommon::math;
// todo make the loop here a parallel for to speed up intialization.
float* spherical_data(vec3i dims,vec3f center, float rmax, float d0) {
    long num_points = dims.long_product();
    float *data = new float[num_points];
    float min, max;
    long index;
    min = 1000.0f;
    max = -1000.0f;
    box3f bounding_box{vec3f{-0.5f,-0.5f,-0.5f},{0.5f,0.5f,0.5f}};
    vec3f spacing = (bounding_box.upper - bounding_box.lower)/(dims-1);
    for(int k = 0;k<dims.z;k++)
        for(int j = 0;j<dims.y;j++)
            for(int i = 0;i<dims.x;i++) {
                index = i + dims.x*j + dims.x*dims.y*k;
                vec3f location = vec3i{i,j,k}*spacing + bounding_box.lower; 
                data[index] = (length(location-center) > rmax) ? 0.0f : d0/length(location-center);
                min = data[index] < min ? data[index] : min;
                max = data[index] > max ? data[index] : max;
                
            }
    std::cout << "min max: " << vec2f(min,max) << std::endl;        
    return data;            
}