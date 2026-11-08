#ifndef POSTPROCESSOR_CAMERACOMMANDTEXT_HPP
#define POSTPROCESSOR_CAMERACOMMANDTEXT_HPP
#include "Postprocessor/Core/views/ViewDefinition.hpp"

// Full numeric state, classic locale and round-trip double precision. Shared by
// GUI gesture commits and tests; no encoded payload or private VTK state.
bool format_camera_command(const Postprocessor::CameraDefinition& camera, std::string& command, std::string& error);
#endif
