# OSPRay-visualization
This repo contains a collection of example programs that exercises the OSPRay API. The collection is a result of my exploration of the
OSPRay API. It is an archive of my experiments. I started experimenting in order to discover how OSPRay works and become
more familiar with the various controls and parameters available through the API. Most of my previous experience with 
OSPRay has been through use of the renderer in Paraview. While this approach proved adequate for most purposes I suspected
I could do more interesting things with the software by directly utilizing the API. 

> This readme and the source code in the repo are meant to be used as notes and reminders to myself about how the OSPRay API works and how to use it. It is not meant to be a comprehensive guide or to promote the use of OSPRay for any particular purpose. The text herein may contain errors due to my misinterpretation of how OSPRay works. 

My experiments go from basic to more advanced. I initially use the default parameter values for all of the OSPRay objects except where changes 
are necessary to produce an image. The process I followed is to first exercise the instantiation of the geometries available to OSPRay. 
Once I can create all of the geometries I move on to experimenting with other aspects of the API like materials, lights and alternate renderers. 
After the geometries have been explored I move on to volumes and experiment with the functionality for volume rendering. The plan is then to combine the
techniques used in the basic experiments to see what kind of visualizations I can produce with OSPRay.  

The repo has a src directory that contains all the code. The organization of src
sort of follows the API documentation. Each subdirectory of src contains code related to the subsection fo the API documentation,... almost. The organization of this readme follows that of the source directory. Subsections of this doc contain notes about the code in the corresponding source directoy. The code itself contains more detailed comments.  

## Experiments
### Hello
This C++ example is the osptutorial example from the OSPRay distro. I used this 
example to set up my development environment.
### Objects
#### Spheres
This example creates a set of 8 spheres arranged in a cube pattern.  
