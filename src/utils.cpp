#include "utils.h"
#include <algorithm>

int isPerfectSquare(int num) {
	int i = 1;
	int sq = 1;
	while (sq <= num) {
		sq = i * i;
		if (sq == num) {
			return i;
		}
		++i;
	}
	return 0;
}

DynamicBlock::DynamicBlock(const ConfigData* data) {
	blockHeight = data -> dynamicBlockHeight;
	blockWidth = data -> dynamicBlockWidth;
	numBlocksWide = ceilFunc(data -> width, blockWidth);
	numBlocksTall = ceilFunc(data -> height, blockHeight);
	updateDynamicBlockData(data, data -> mpi_rank);
}

void DynamicBlock::updateDynamicBlockData(const ConfigData* data, int blockID) {
	this -> blockID = blockID;
	
	xBlockID = blockID % numBlocksWide;
	yBlockID = blockID / numBlocksWide;
	
	blockRowStart = yBlockID * blockHeight;
	blockColStart = xBlockID * blockWidth;
	
	blockRowEnd = std::min(blockRowStart + blockHeight, data -> height);
	blockColEnd = std::min(blockColStart + blockWidth, data -> width);
	
	blockRowNum = blockRowEnd - blockRowStart;
	blockColNum = blockColEnd - blockColStart;
}

int DynamicBlock::getNumOfPixels() { return 3 * blockRowNum * blockColNum;}

int DynamicBlock::getSize() {return getNumOfPixels() + DYNAMIC_PACKET_MAX;}

int DynamicBlock::getIndex(int row, int col) {
	return DYNAMIC_PACKET_MAX + 3 * (row * blockColNum + col);
}