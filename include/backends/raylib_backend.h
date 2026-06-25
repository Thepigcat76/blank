#pragma once

#include "../blank.h"

void raylib_backend_set_loglevel(u32 log_level);

void raylib_backend_init(Blank_Backend *backend,
                         Blank_BackendInitStage init_stage);