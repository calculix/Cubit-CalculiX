#pragma once

#if defined(_WIN32)
#if defined(CUBITSIM_MODEL_BUILD)
#define CUBITSIM_MODEL_EXPORT __declspec(dllexport)
#else
#define CUBITSIM_MODEL_EXPORT __declspec(dllimport)
#endif
#else
#define CUBITSIM_MODEL_EXPORT
#endif
