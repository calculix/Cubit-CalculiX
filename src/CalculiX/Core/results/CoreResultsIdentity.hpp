#ifndef CORERESULTSIDENTITY_HPP
#define CORERESULTSIDENTITY_HPP
#include <string>
class CoreResultsFrd;
class CoreResultsDat;
struct CoreResultsVtkPlan;
// Versioned SHA-256 over retained numeric/text content and resolved mesh/link
// semantics. No pointers, paths, thread counts, clocks or struct padding enter
// the digest. Integer lengths and IEEE-754 doubles are encoded little endian;
// NaNs and signed zero are canonicalized. Result identity is not a job name.
std::string retained_result_identity(const CoreResultsFrd& frd, const CoreResultsDat& dat, const CoreResultsVtkPlan& plan);
#endif
