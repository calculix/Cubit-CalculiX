#include "CalculiXReportTemplateProvider.hpp"
#include "CalculiXReportImageSettings.hpp"
#include <utility>
#include "Framework/Reporting/ReportTemplate.hpp"

CalculiXReportTemplateProvider::CalculiXReportTemplateProvider() = default;
CalculiXReportTemplateProvider::~CalculiXReportTemplateProvider() = default;

std::string CalculiXReportTemplateProvider::command_prefix() const
{
  return "ccx";
}

bool CalculiXReportTemplateProvider::describe(ReportTemplate& output, std::string& error) const
{
  ReportTemplate candidate;
  candidate.id = "calculix.standard";
  candidate.workspace_id = "calculix";
  candidate.revision = 3;
  candidate.content_groups = {
    {"ccx.content.model", "Model"},
    {"ccx.content.steps", "Analysis steps", "", "ccx.steps"},
    {"ccx.content.outputs", "Requested outputs", "ccx.content.steps"},
    {"ccx.content.images", "Model and setup images"},
    {"ccx.content.step_images", "Per-step images", "ccx.content.images"},
    {"ccx.content.post", "Postprocessor"}};
  const std::vector<std::pair<std::string, std::string>> categories =
  {
    {"ccx.model.overview", "Model overview"}, {"ccx.model.materials", "Materials"},
    {"ccx.model.sections", "Sections and orientations"}, {"ccx.model.blocks", "Blocks"},
    {"ccx.model.mesh", "Mesh"}, {"ccx.model.sets", "Sets and surfaces"},
    {"ccx.model.interactions", "Interactions and constraints"},
    {"ccx.model.initial_conditions", "Initial conditions"},
    {"ccx.steps", "Analysis steps"}, {"ccx.other", "Additional input definitions"},
    {"post.results", "Postprocessor results"}, {"post.images", "Postprocessor result images"}
  };
  for (const auto& entry : categories)
  {
    ReportSection section;
    section.id = entry.first;
    section.title = entry.second;
    candidate.sections.push_back(section);
    ReportOption option;
    option.key = entry.first;
    option.label = entry.second;
    option.kind = "boolean";
    option.default_value = "true";
    candidate.options.push_back(option);
    candidate.defaults.values[option.key] = option.default_value;
  }
  for (const auto& entry : std::vector<std::pair<std::string, std::string>>
    {{"parameters", "Step procedure and parameters"}, {"boundary_conditions", "Step boundary conditions"},
     {"loads", "Step loads"}, {"amplitudes", "Amplitude definitions and references"},
     {"model_changes", "Step model changes"}, {"field_output", "Requested field outputs"},
     {"history_output", "Requested history outputs"}})
  {
    ReportOption option;
    option.key = "ccx.steps." + entry.first;
    option.label = entry.second;
    option.kind = "boolean";
    option.default_value = "true";
    candidate.options.push_back(option);
    candidate.defaults.values[option.key] = option.default_value;
  }
  ReportOption steps;
  steps.key = "steps";
  steps.label = "Steps (all or comma-separated ordinals)";
  steps.kind = "steps";
  steps.default_value = "all";
  candidate.options.push_back(steps);
  candidate.defaults.values[steps.key] = steps.default_value;
  CalculiXReportImageSettings::add_options(candidate);
  for (auto& option : candidate.options)
  {
    if (option.key.rfind("ccx.model.", 0) == 0 || option.key == "ccx.other")
    {
      option.group_id = "ccx.content.model";
    }
    else if (option.key.rfind("ccx.steps.", 0) == 0)
    {
      option.group_id = "ccx.content.steps";
      if (option.key == "ccx.steps.field_output" || option.key == "ccx.steps.history_output")
      {
        option.group_id = "ccx.content.outputs";
      }
    }
    else if (option.key.rfind("ccx.images.", 0) == 0)
    {
      option.group_id = "ccx.content.images";
      if (option.key == "ccx.images.setup" || option.key == "ccx.images.loads" || option.key == "ccx.images.bcs")
      {
        option.group_id = "ccx.content.step_images";
      }
    }
    else if (option.key.rfind("post.", 0) == 0)
    {
      option.group_id = "ccx.content.post";
    }
  }
  output = std::move(candidate);
  error.clear();
  return true;
}
