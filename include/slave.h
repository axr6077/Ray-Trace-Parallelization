#ifndef __SLAVE_PROCESS_H__
#define __SLAVE_PROCESS_H__

#include "RayTrace.h"

void slaveMain( ConfigData *data );
void staticCyclesHorizontal(ConfigData *data);
void slaveStaticStripsVertical(ConfigData* data);
void slaveStaticBlocks(ConfigData *data);
void slaveDynamicPartition(ConfigData *data);
#endif
