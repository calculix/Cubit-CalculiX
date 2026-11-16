#include "ReportContentGroup.hpp"
#include <utility>

ReportContentGroup::ReportContentGroup() = default;

ReportContentGroup::ReportContentGroup(std::string id, std::string label, std::string parent, std::string gate)
  : id(std::move(id)), label(std::move(label)), parent_id(std::move(parent)), gate_option(std::move(gate))
{
}
