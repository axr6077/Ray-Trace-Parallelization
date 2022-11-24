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

StaticBlock::StaticBlock(const ConfigData *data) {
	sqrtProcessors = isPerfectSquare(data -> mpi_procs);
	if (sqrtProcessors == 0) {return;}
	rowsMax = data -> height;
	colsMax = data -> width;
	blockID = data -> mpi_rank;
	rowsN = rowsMax / sqrtProcessors;
	rowsE = rowsN + 1;
	rowsR = rowsMax % sqrtProcessors;
	rowsRS = rowsMax - rowsR;
	colsN = colsMax / sqrtProcessors;
	colsE = colsN + 1;
	colsR = colsMax % sqrtProcessors;
	colsRS = colsMax - colsR;
	updateStaticBlockData(blockID);
}

void StaticBlock::updateStaticBlockData(int blockID) {
	this -> blockID = blockID;
	blockRow = blockID / sqrtProcessors;
	blockCol = blockID % sqrtProcessors;
	if (blockRow >= rowsRS) {
		rowsToCalc = rowsE;
		rowStart = rowsRS * rowsN + (blockRow - rowsRS) * rowsE;
	}
	else {
		rowsToCalc = rowsN;
		rowStart = blockRow * rowsN;
	}
	rowEnd = rowStart + rowsToCalc;
	if (blockCol >= colsRS) {
		colsToCalc = colsE;
		colStart = colsRS * colsN + (blockCol - colsRS) * colsE;
	}
	else {
		colsToCalc = colsN;
		colStart = blockCol * colsN;
	}
	colEnd = colStart + colsToCalc;
}

int StaticBlock::getNumOfPixels() {
	return 3 * (rowsToCalc + 1) * colsToCalc;
}

int StaticBlock::getSize(){
	return getNumOfPixels() + 1;
}

int StaticBlock::getIndex(int row, int col) {
	return 3 * (row * colsToCalc + col);
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
