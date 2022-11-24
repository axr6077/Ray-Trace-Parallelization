/**
 * Parallel implementation of Ray Tracing
 * Partition Modes: 
 * 	STATIC_CYCLICAL_ROW_STRIPS: Status - Completed
 * 	DYNAMIC_BLOCKS: Status - Completed
 * 	STATIC_VERTICAL: Status - Not implemented yet
 *	STATIC_BLOCKS: Status - In progress
 * @author: Jason Lowden
 * @author: Ayush Rout
 * @version: 1.3
 * @date: 11/24/2022
 * */

#include <ctime>
#include <iostream>
#include <ctime>
#include <string>
#include <sys/stat.h>
#include <mpi.h>
using namespace std;

#include "RayTrace.h"
#include "master.h"
#include "slave.h"

int main( int argc, char* argv[] ) 
{
    //Keep the data that will be used for the scene.
    ConfigData data; 
    int init_flag = MPI_Init(&argc, &argv); 
    if (init_flag != MPI_SUCCESS) {
    	fprintf(stderr, "Cannot initialize MPI. \n");
	MPI_Abort(MPI_COMM_WORLD, init_flag);
    }
    
    //Try to initialize the scene.
    bool result = initialize(&argc, &argv, &data);
    //Make sure that the initialization was completed.	
    if( result )
    {
        MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
    }

    //Insert the MPI intialization code here.
    MPI_Comm_rank(MPI_COMM_WORLD, &data.mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &data.mpi_procs);

    if( data.mpi_rank == 0 )
    {
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
                MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER); 
            }
        }

        //Print a summary of the number of processes, width, height, and partitioning scheme.
        //DO NOT CHANGE ANYTHING IN THIS SECTION!!!
        std::cout << "Scene: " << data.sceneID << std::endl; 
        std::cout << "Width x Height: " << data.width << " x " << data.height << std::endl;
        std::cout << "Partitioning scheme: " << data.partitioningMode << std::endl;
        std::cout << "Number of Processes: " << data.mpi_procs << std::endl;
        //Print out the other properties as well
        std::cout << "Dynamic block size: " << data.dynamicBlockWidth << " x " << data.dynamicBlockHeight << std::endl;
        std::cout << "Cycle Size: " << data.cycleSize << std::endl; 

        //Start the main processing for the ray tracer.
        masterMain( &data );
    }
    else
    {
        slaveMain( &data );
    }

    //Clean up the scene and other data.
    shutdown(&data);
    MPI_Finalize();

    return 0;
}
