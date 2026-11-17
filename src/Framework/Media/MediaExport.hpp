#pragma once
#if defined(_WIN32)
#if defined(CUBITSIM_MEDIA_BUILD)
#define CUBITSIM_MEDIA_EXPORT __declspec(dllexport)
#else
#define CUBITSIM_MEDIA_EXPORT __declspec(dllimport)
#endif
#else
#define CUBITSIM_MEDIA_EXPORT
#endif
