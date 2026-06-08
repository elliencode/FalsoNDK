#pragma once

#include "../android/ASensor.h"

void sensors_init(ASensorEventQueue * queue);

[[noreturn]] void * sensors_thread(void * arg);
void sensors_poll();
