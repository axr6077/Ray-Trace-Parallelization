#ifndef __RAY_TRACE_H__
#define __RAY_TRACE_H__

#include <string>

//Declare the Camera and World as classes.
//This will eliminate the need for any explicit header files here.
//Do NOT worry about these class definitions. They are handled internally.
class Camera;
class World;

//Specify the partitioning types that can be used.
typedef enum{ 
    PART_MODE_NONE = 0,
    PART_MODE_STATIC_STRIPS_HORIZONTAL = 1,
    PART_MODE_STATIC_STRIPS_VERTICAL = 2,
    PART_MODE_STATIC_BLOCKS = 4,
    PART_MODE_STATIC_CYCLES_HORIZONTAL = 8,
    PART_MODE_STATIC_CYCLES_VERTICAL = 16,
    PART_MODE_DYNAMIC = 32
} PartType;

//Define a structure that will be used to hold all of the configuration data.
typedef struct
{
    //Image size
    int width;
    int height;

    //MPI values
    int mpi_rank;
    int mpi_procs;

    //Partitioning mode and associated properties
    PartType partitioningMode;
    int dynamicBlockWidth;
    int dynamicBlockHeight;
    int cycleSize;

    //Scene data
    //DO NOT TOUCH THESE!
    Camera* camera;
    World* world;
    std::string sceneID;

} ConfigData;

//This function will do all of the command line argument parsing along with
//some limited error checking on the argument. It will read the scene into
//the application and then populate all of the values in the ConfigData 
//struct. After this returns, you will have to set the mpi_rank and mpi_procs
//values by yourself. This is done to eliminate any dependencies on MPI 
//within the library.
//
//Inputs:
//    argc - The pointer to the number of input arguments
//    argv - The pointer to the input arguments
//    configuration - The pointer to the ConfigData struct that will be
//        used to hold the relevant information.
//
//Outputs:
//    true if there was an error in the processing; otherwise, false
bool initialize(int* argc, char** argv[], ConfigData* configuration);

//This function will handle the cleanup of the scene. Remember, since
//there are no MPI dependencies, this will NOT call any MPI functions.
//
//Inputs:
//    configuration - The pointer to the ConfigData struct with the scene.
void shutdown(ConfigData* configuration);

//This function will actually perform ray tracing on a given pixel.
//When called, the values for row and column should be within the 
//acceptable bounds of the image, that is, 0 <= row < height and
//0 <= column < width. If these conditions are not met, an error
//message will be displayed.
//
//Inputs:
//    color - a float array of 3 elements; this does not have to be a 
//        separate array of 3 elements, but this will write to color[0],
//        color[1], and color[2].
//    row - the row of the image to render
//    column - the column of the image to render
//    configuration - the pointer to the ConfigData struct that contains
//        the scene information.
void shadePixel(float* color, int row, int column, ConfigData* configuration);

//This function will save the image to disk based on the generated
//filename.
//
//Inputs:
//    filename - the name of the file to write. This should be created
//        by the generateFileName() function and then passed as a value.
//    pixels - the float pointer that contains all of the pixel data from
//        shading the scene.
//    data - The pointer to the ConfigData struct that contains the
//        scene information. 
bool savePixels(std::string filename, float* pixels, ConfigData* data);

//This function will generate a file name that is used to save the image.
//The file names will be unique down to the second.
//The format will be MMDDYY-hhmmss, where:
//    MM = month, DD = day, YY = year
//    hh = hour, mm = minute, ss = second
//
//Inputs: NONE
//
//Outputs:
//    A C++ string the represents the file name. 
std::string generateFileName(ConfigData* data);

#endif
