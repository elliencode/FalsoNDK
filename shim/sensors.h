#pragma once

#include <psp2/kernel/threadmgr.h>
#include "android/ASensor.h"

void sensors_init(ASensorEventQueue * queue);

int sensors_thread(SceSize args, void * argp);
void sensors_poll();
