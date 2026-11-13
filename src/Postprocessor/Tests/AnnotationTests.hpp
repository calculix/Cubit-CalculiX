#ifndef POSTPROCESSOR_ANNOTATIONTESTS_HPP
#define POSTPROCESSOR_ANNOTATIONTESTS_HPP
#include <memory>
class PostprocessorApplicationInterface;
// Uses the selection fixture's original-node, element and IP identities.
int annotation_cases(const std::shared_ptr<PostprocessorApplicationInterface>& application);
#endif
