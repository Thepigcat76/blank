#pragma once

#include "internal/blank_backend.h"
#include "internal/blank_init.h"
#include "internal/blank_render.h"

void blank_start(Blank_InitState state,
               void (*backend_init_func)(Blank_Backend *backend, Blank_BackendInitStage stage),
               void (*app_run_func)(Blank_UiState *state));

bool blank_window_closed(Blank_UiState *state);

bool blank_window_resized(Blank_UiState *state);
