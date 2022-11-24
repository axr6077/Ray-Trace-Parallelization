/**
 * Parallel implementation of Ray Tracing
 * Partition Modes: 
 * 	STATIC_CYCLICAL_ROW_STRIPS: Status - Completed
 * 	DYNAMIC_BLOCKS: Status - Completed
 * 	STATIC_VERTICAL: Status - Not implemented yet
 *	STATIC_BLOCKS: Status - Not implemented yet
 * @author: Ayush Rout
 * @version: 1.7
 * @date: 11/22/2022
 * */

//This file contains the code that the master process will execute.

#include <iostream>
#include <mpi.h>
#include <cstring>

#include "RayTrace.h"
#include "master.h"
#include "utils.h"

void masterMain(ConfigData* data)
{
    //Depending on the partitioning scheme, different things will happen.
    //You should have a different function for each of the required 
    //schemes that returns some values that you need to handle.
    
    //Allocate space for the image on the master.
    float* pixels = new float[3 * data->width * data->height];
    
    //Execution time will be defined as how long it takes
    //for the given function to execute based on partitioning
    //type.
    double renderTime = 0.0, startTime, stopTime;
	//Add the required partitioning methods here in the case statement.
	//You do not need to handle all cases; the default will catch any
	//statements that are not specified. This switch/case statement is the
	//only place that you should be adding code in this function. Make sure
	//that you update the header files with the new functions that will be
	//called.
	//It is suggested that you use the same parameters to your functions as shown
	//in the sequential example below.
    
    switch (data->partitioningMode)
    {
        case PART_MODE_NONE:
            //Call the function that will handle this.
            startTime = MPI_Wtime();
            masterSequential(data, pixels);
            stopTime = MPI_Wtime();
            break;
	case PART_MODE_STATIC_CYCLES_HORIZONTAL:
	    startTime = MPI_Wtime();
	    staticCyclesHorizontal(data, pixels);
	    stopTime = MPI_Wtime();
	    break;
	case PART_MODE_STATIC_BLOCKS:
	    startTime = MPI_Wtime();
	    masterStaticBlocks(data, pixels);
	    stopTime = MPI_Wtime();
	    break;
	case PART_MODE_DYNAMIC:
	    startTime = MPI_Wtime();
	    masterDynamicPartition(data, pixels);
	    stopTime = MPI_Wtime();
	    break;		
        default:
            std::cout << "This mode (" << data->partitioningMode;
            std::cout << ") is not currently implemented." << std::endl;
            break;
    }

    renderTime = stopTime - startTime;
    std::cout << "Execution Time: " << renderTime << " seconds" << std::endl << std::endl;

    //After this gets done, save the image.
    std::cout << "Image will be saved to: ";
    std::string file = generateFileName(data);
    std::cout << file << std::endl;
    savePixels(file, pixels, data);

    //Delete the pixel data.
    delete[] pixels; 
}

void masterSequential(ConfigData* data, float* pixels)
{
    //Start the computation time timer.
    double computationStart = MPI_Wtime();

    //Render the scene.
    for( int i = 0; i < data->height; ++i )
    {
        for( int j = 0; j < data->width; ++j )
        {
            int row = i;
            int column = j;

            //Calculate the index into the array.
            int baseIndex = 3 * ( row * data->width + column );

            //Call the function to shade the pixel.
            shadePixel(&(pixels[baseIndex]),row,j,data);
        }

    }

    //Stop the comp. timer
    double computationStop = MPI_Wtime();
    double computationTime = computationStop - computationStart;

    //After receiving from all processes, the communication time will
    //be obtained.
    double communicationTime = 0.0;

    //Print the times and the c-to-c ratio
	//This section of printing, IN THIS ORDER, needs to be included in all of the
	//functions that you write at the end of the function.
    std::cout << "Total Computation Time: " << computationTime << " seconds" << std::endl;
    std::cout << "Total Communication Time: " << communicationTime << " seconds" << std::endl;
    double c2cRatio = communicationTime / computationTime;
    std::cout << "C-to-C Ratio: " << c2cRatio << std::endl;
}

int getIndex(const ConfigData* data, int row, int col) {
	return 3 * (row * data -> width + col);
}

void staticCyclesHorizontal(ConfigData* data, float* pixels) {
	MPI_Status status;
	double compStart, compStop, compTime;
	double commStart, commStop, commTime;
	int max_rows = data -> height;
	int max_columns = data -> width;
	int rows_per_proc = (max_rows/data -> mpi_procs) + data -> cycleSize;
	int numPix = getIndex(data, rows_per_proc, 0);
	int size = numPix + 1;
	float* p_pixel = new float[size];
	compStart = MPI_Wtime();
	int M_row = 0;
	for (int row = 0; row < max_rows; ++row) {
		if ((row / data -> cycleSize) % data -> mpi_procs == data -> mpi_rank) {
			for (int col = 0; col < max_columns; ++col) {
				int baseIdx = getIndex(data, row, col);
				shadePixel(&(pixels[baseIdx]), row, col, data);
			}
			M_row++;
		}
	}
	compStop = MPI_Wtime();
	compTime = compStop - compStart;
	MPI_Barrier(MPI_COMM_WORLD);
	int pRow = 0; 
	int mappedRow = 0;
	commStart = MPI_Wtime();
	for (int slave = 1; slave < data -> mpi_procs; slave++) {
		pRow = slave * data -> cycleSize;
		mappedRow = 0;

		MPI_Recv(p_pixel, size, MPI_FLOAT, slave, 8, MPI_COMM_WORLD, &status);
		if (p_pixel[numPix] > compTime) {
			compTime = p_pixel[numPix];
		}

		while(pRow < max_rows) {
			for (int i = 0; i < data -> cycleSize && pRow < max_rows; i++) {
				memcpy(&(pixels[getIndex(data, pRow, 0)]), &(p_pixel[getIndex(data, mappedRow, 0)]), sizeof(float) * 3 * data -> width);
				mappedRow++;
				pRow++;
			}
			pRow += (data -> mpi_procs - 1) * data -> cycleSize;
		}
	}
	commStop = MPI_Wtime();
	commTime = commStop - commStart;
	std::cout << "Total Computation Time: " << compTime << " seconds" << std::endl;
   	std::cout << "Total Communication Time: " << commTime << " seconds" << std::endl;
    	double c2cRatio = commTime / compTime;
    	std::cout << "C-to-C Ratio: " << c2cRatio << std::endl;
	delete[] p_pixel;	
}

void masterDynamicPartition(ConfigData* data, float *pixels) {
	DynamicBlock dynamicBlock = DynamicBlock(data);
	MPI_Status status;
	double computationStart, computationStop, computationTime;
	computationStart = MPI_Wtime();
	int size = dynamicBlock.getSize();
	int numBlocks = dynamicBlock.numBlocksWide * dynamicBlock.numBlocksTall;
	int packetIndex, pixelIndex, slave, slaveBlockID;
	float *packet = new float[size];
	
	int blockID;
	double communicationStart1, communicationStop1, communicationTime1;
	communicationStart1 = MPI_Wtime();
	for (blockID = data -> mpi_procs - 1; blockID < numBlocks; ++blockID) {
		MPI_Recv(packet, size, MPI_FLOAT, MPI_ANY_SOURCE, 8, MPI_COMM_WORLD, &status);
		slave = packet[DynamicBlock::DYNAMIC_PACKET::DYNAMIC_PACKET_SLAVE];
		slaveBlockID = packet[DynamicBlock::DYNAMIC_PACKET::DYNAMIC_PACKET_BLOCK_ID];
		dynamicBlock.updateDynamicBlockData(data, slaveBlockID);
		MPI_Send(&blockID, 1, MPI_INT, slave, MPI_TAG_DYNAMIC, MPI_COMM_WORLD);
		for (int row = 0; row < dynamicBlock.blockRowNum; row++) {
			for (int col = 0; col < dynamicBlock.blockColNum; col++) {
				pixelIndex = getIndex(data, dynamicBlock.blockRowStart + row, dynamicBlock.blockColStart + col);
				
				packetIndex = dynamicBlock.getIndex(row, col);
				pixels[pixelIndex] = packet[packetIndex];
				pixels[pixelIndex + 1] = packet[packetIndex + 1];
				pixels[pixelIndex + 2] = packet[packetIndex + 2];
			}
		}			
	}
	communicationStop1 = MPI_Wtime();
	communicationTime1 = communicationStop1 - communicationStart1;

	double communicationStart2, communicationStop2, communicationTime2;
	communicationStart2 = MPI_Wtime();
	blockID = -1;
	for (int i = 1; i < data -> mpi_procs; ++i) {
		MPI_Recv(packet, size, MPI_FLOAT, MPI_ANY_SOURCE, 8, MPI_COMM_WORLD, &status);
		slave = packet[DynamicBlock::DYNAMIC_PACKET::DYNAMIC_PACKET_SLAVE];
		slaveBlockID = packet[DynamicBlock::DYNAMIC_PACKET::DYNAMIC_PACKET_BLOCK_ID];
		if(packet[DynamicBlock::DYNAMIC_PACKET::DYNAMIC_PACKET_COMPUTATION_TIME] > computationTime) {
			computationTime = packet[DynamicBlock::DYNAMIC_PACKET::DYNAMIC_PACKET_COMPUTATION_TIME];
		}
		dynamicBlock.updateDynamicBlockData(data, slaveBlockID);
		MPI_Send(&blockID, 1, MPI_INT, slave, MPI_TAG_DYNAMIC, MPI_COMM_WORLD);
		for (int row = 0; row < dynamicBlock.blockRowNum; row++) {
			for (int col = 0; col < dynamicBlock.blockColNum; col++) {
				pixelIndex = getIndex(data, dynamicBlock.blockRowStart + row, dynamicBlock.blockColStart + col);
				packetIndex = dynamicBlock.getIndex(row, col);
				pixels[pixelIndex] = packet[packetIndex];
                                pixels[pixelIndex + 1] = packet[packetIndex + 1];
                                pixels[pixelIndex + 2] = packet[packetIndex + 2];
			}
		}
	}
	communicationStop2 = MPI_Wtime();
	communicationTime2 = communicationStop2 = communicationStart2;
	double communicationTime = communicationTime1 + communicationTime2;
	computationStop = MPI_Wtime();
	computationTime = computationStop - computationStart;
	std::cout << "Total Computation Time: " << computationTime << " seconds" << std::endl;
    	std::cout << "Total Communication Time: " << communicationTime << " seconds" << std::endl;
    	double c2cRatio = communicationTime / computationTime;
    	std::cout << "C-to-C Ratio: " << c2cRatio << std::endl;
	delete[] packet;
}

void masterStaticBlocks(ConfigData *data, float *pixels) {
	MPI_Status status;
	double computationStart, computationStop, computationTime;
	computationStart = MPI_Wtime();
	StaticBlock staticBlock = StaticBlock(data);
	if (staticBlock.sqrtProcessors == 0) {
		std::cout << "Error: " << data -> mpi_procs << " is not a perfect square to implement blocks" << std::endl;
		return;
	}
	for (int row = staticBlock.rowStart; row < staticBlock.rowEnd; ++row) {
		for (int col = staticBlock.colStart; col < staticBlock.colEnd; ++col) {
			int baseIdx = getIndex(data, row, col);

			shadePixel(&(pixels[baseIdx]), row, col, data);
		}
	}
	computationStop = MPI_Wtime();
	computationTime = computationStop - computationStart;
	MPI_Barrier(MPI_COMM_WORLD);
	staticBlock.updateStaticBlockData(data -> mpi_procs - 1);
	int size = staticBlock.getSize();
	float *packet = new float[size];
	int packetIdx, pixelIdx;
	double communicationStart, communicationStop, communicationTime;
	communicationStart = MPI_Wtime();
	for (int slave = 1; slave < data -> mpi_procs; slave++) {
		staticBlock.updateStaticBlockData(slave);
		size = staticBlock.getSize();

		MPI_Recv(packet, size, MPI_FLOAT, slave, 8, MPI_COMM_WORLD, &status);
		if (packet[size - 1] > computationTime) {
			computationTime = packet[size - 1];
		}
		for (int row = 0; row < staticBlock.rowsToCalc; row++) {
			for (int col = 0; col < staticBlock.colsToCalc; col++) {
				pixelIdx = getIndex(data, staticBlock.rowStart + row, staticBlock.colStart + col);
				packetIdx = staticBlock.getIndex(row, col);
				pixels[pixelIdx] = packet[packetIdx];
				pixels[pixelIdx + 1] = packet[packetIdx + 1];
				pixels[pixelIdx + 2] = packet[packetIdx + 2];
			}
		}
	
	}
	communicationStop = MPI_Wtime();
	communicationTime = communicationStop - communicationStart;
	std::cout << "Total Computation Time: " << computationTime << " seconds" << std::endl;
    	std::cout << "Total Communication Time: " << communicationTime << " seconds" << std::endl;
    	double c2cRatio = communicationTime / computationTime;
    	std::cout << "C-to-C Ratio: " << c2cRatio << std::endl;
	delete[] packet;
}
