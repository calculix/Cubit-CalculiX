#pragma once
#include <memory>
class PostprocessorApplicationInterface;
int capture_cases(const std::shared_ptr<PostprocessorApplicationInterface>& application);
