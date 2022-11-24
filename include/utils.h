#ifndef __UTILS_H__
#define __UTILS_H__

#include "RayTrace.h"

typedef enum {
	MPI_TAG_DYNAMIC
} MPIMessageTag;

typedef struct StaticBlock{
	int sqrtProcessors; // sqrt of the number of processors 
	int colsMax;
	int rowsMax;
	int blockID;
	int rowsN, rowsE, rowsR, rowsRS;
	int colsN, colsE, colsR, colsRS;
	int blockRow, blockCol;
	int rowStart, rowEnd, rowsToCalc;
	int colStart, colEnd, colsToCalc;
	StaticBlock(const ConfigData* data);
	void updateStaticBlockData(int blockID);
	int getNumOfPixels();
	int getSize();
	int getIndex(int row, int col);
} StaticBlock;

typedef struct DynamicBlock {
	int blockHeight, blockWidth, blockRowStart, blockColStart, \
		blockRowEnd, blockColEnd, blockRowNum, blockColNum, 
		numBlocksWide, numBlocksTall, blockID, xBlockID, yBlockID;
	
	DynamicBlock(const ConfigData* data);

	void updateDynamicBlockData(const ConfigData* data, int blockID);
	int getNumOfPixels();
	int getSize();
	int getIndex(int row, int col);
	
	typedef enum {
		DYNAMIC_PACKET_SLAVE = 0,
		DYNAMIC_PACKET_BLOCK_ID,
		DYNAMIC_PACKET_COMPUTATION_TIME,
		DYNAMIC_PACKET_MAX
	} DYNAMIC_PACKET;
} DynamicBlock;

int isPerfectSquare(int num);
inline int ceilFunc(int m, int n) {
	return (m + n - 1) / n;
}

#endif
