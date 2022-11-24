//This file contains the code that the master process will execute.

#include <iostream>
#include <mpi.h>
#include "RayTrace.h"
#include "slave.h"
#include "utils.h"
#include "master.h"

void slaveMain(ConfigData* data)
{
    //Depending on the partitioning scheme, different things will happen.
    //You should have a different function for each of the required 
    //schemes that returns some values that you need to handle.
    switch (data->partitioningMode)
    {
        case PART_MODE_NONE:
            //The slave will do nothing since this means sequential operation.
            break;
	case PART_MODE_STATIC_CYCLES_HORIZONTAL:
	    staticCyclesHorizontal(data);
	    break;
	case PART_MODE_STATIC_BLOCKS:
	    slaveStaticBlocks(data);
	    break;
	case PART_MODE_DYNAMIC:
	    slaveDynamicPartition(data);
	    break;
        default:
            std::cout << "This mode (" << data->partitioningMode;
            std::cout << ") is not currently implemented." << std::endl;
            break;
    }
}


void staticCyclesHorizontal(ConfigData* data) {
	double computationStart, computationStop, computationTime;
	int max_rows = data -> height;
	int max_columns = data -> width;
	int rows_per_proc = max_rows / data -> mpi_procs + data -> cycleSize;

	int numPix = getIndex(data, rows_per_proc, 0);
	int size = numPix + 1;
	float *pixels = new float[size];
	computationStart = MPI_Wtime();

	int M_row = 0;
	for (int row = 0; row < max_rows; ++row) {
		if ((row / data -> cycleSize) % data -> mpi_procs == data -> mpi_rank) {
			for (int col = 0; col < max_columns; ++col) {
				int baseIdx = getIndex(data, M_row, col);
				shadePixel(&(pixels[baseIdx]), row, col, data);
			}
			M_row++;
		}
	}
	computationStop = MPI_Wtime();
	computationTime = computationStop - computationStart;
	MPI_Barrier(MPI_COMM_WORLD);
	pixels[numPix] = computationTime;	
	MPI_Send(pixels, size, MPI_FLOAT, 0, 8, MPI_COMM_WORLD);
	delete[] pixels;
}

void slaveDynamicPartition(ConfigData* data) {
	MPI_Status status;
	int blockID = data -> mpi_rank - 1;
	DynamicBlock dynamicBlock = DynamicBlock(data);
	dynamicBlock.updateDynamicBlockData(data, 0);
	int size = dynamicBlock.getSize();
	float* packet = new float[size];
	
	double computationStart, computationStop, computationTime;
	computationTime = 0.0;
	
	while (blockID != -1) {
		dynamicBlock.updateDynamicBlockData(data, blockID);
		computationStart = MPI_Wtime();
		for(int row = 0; row < dynamicBlock.blockRowNum; ++row) {
			for (int col = 0; col < dynamicBlock.blockColNum; ++col) {
				int baseIdx = dynamicBlock.getIndex(row, col);
				shadePixel(&(packet[baseIdx]), row + dynamicBlock.blockRowStart, col + dynamicBlock.blockColStart, data);
			}
		}
		computationStop = MPI_Wtime();
		computationTime += computationStop - computationStart;
		packet[DynamicBlock::DYNAMIC_PACKET::DYNAMIC_PACKET_BLOCK_ID] = blockID;
		packet[DynamicBlock::DYNAMIC_PACKET::DYNAMIC_PACKET_SLAVE] = data -> mpi_rank;
		packet[DynamicBlock::DYNAMIC_PACKET::DYNAMIC_PACKET_COMPUTATION_TIME] = computationTime;
		
		MPI_Send(packet, size, MPI_FLOAT, 0, 8, MPI_COMM_WORLD);

		MPI_Recv(&blockID, 1, MPI_INT, 0, MPI_TAG_DYNAMIC, MPI_COMM_WORLD, &status);
	}	
}

