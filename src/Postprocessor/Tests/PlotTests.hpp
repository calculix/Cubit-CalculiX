#ifndef POSTPROCESSOR_PLOTTESTS_HPP
#define POSTPROCESSOR_PLOTTESTS_HPP
#include <memory>
class PostprocessorApplicationInterface;
int plot_cases(const std::shared_ptr<PostprocessorApplicationInterface>& application);
#endif
