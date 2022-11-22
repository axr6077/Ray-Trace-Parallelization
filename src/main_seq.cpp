#include <ctime>
#include <iostream>
#include <ctime>
#include <string>
#include <sys/stat.h>
#include <errno.h>
using namespace std;

#include "RayTrace.h"

int main( int argc, char* argv[] ) 
{
    ConfigData data;
    
    //Create the output directory where all of the renders will be saved.
    struct stat stat_buf;
    string rd("renders");
    stat(rd.c_str(), &stat_buf);
    if(!S_ISDIR(stat_buf.st_mode)) 
    {
        if(mkdir("renders", 0700) != 0)
        {
            cerr << "Could not create the 'renders' directory!" << endl;
            cerr << "Don't know where to save the rendered images!" << endl;
            return 1;
        }
    }
    
    //Try to initialize the scene.
    bool result = initialize(&argc, &argv, &data);
    //Make sure that the initialization was completed.	
    if( result )
    {
        return 1;
    }

    //Fill in the MPI related data
    data.mpi_rank = 0;
    data.mpi_procs = 1;

    //Print a summary of the number of processes, width, height, and partitioning scheme.
    std::cout << "Scene: " << data.sceneID << std::endl;
    std::cout << "Width x Height: " << data.width << " x " << data.height << std::endl;
    std::cout << "Partitioning scheme: " << data.partitioningMode << std::endl;
    std::cout << "Number of Processes: " << 1 << std::endl;

    //Allocate enough space.
    float* pixels = new float[ 3 * data.width * data.height ];
    clock_t start = clock();

    //Render the scene.
    for( int i = 0; i < data.height; ++i )
    {
        for( int j = 0; j < data.width; ++j )
        {
            int row = i;
            int column = j;

            //Calculate the index into the array.
            int baseIndex = 3 * ( row * data.width + column );

            //Call the function to shade the pixel.
            shadePixel(&(pixels[baseIndex]),row,j,&data);
        }
    }

    //Stop the timing.
    clock_t stop = clock();

    //Figure out how much time was taken.
    float time = (float)(stop - start) / (float)CLOCKS_PER_SEC;
    std::cout << "Execution Time: " << time << " seconds" << std::endl << std::endl;

    //Now save the image.
    std::cout << "Image will be save to: ";
    std::string file = generateFileName(&data);
    std::cout << file << std::endl;
    savePixels(file, pixels, &data);
    
    //Clean up the scene and other data.
    shutdown(&data);

    //Delete the pixels.
    delete[] pixels;

    return 0;
}
