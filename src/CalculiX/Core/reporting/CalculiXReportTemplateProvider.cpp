#include "CalculiXReportTemplateProvider.hpp"
#include "Framework/MeshQuality/MeshQualityDefaults.hpp"
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
  candidate.revision = 6;
  candidate.content_groups = {
    {"ccx.content.model", "Model"},
    {"ccx.content.steps", "Analysis steps", "", "ccx.steps"},
    {"ccx.content.outputs", "Requested outputs", "ccx.content.steps"},
    {"ccx.content.images", "Model and setup images"},
    {"ccx.content.step_images", "Per-step images", "ccx.content.images"},
    {"ccx.content.post", "Results", "", "post.results"},
    {"ccx.content.post_images", "Result images", "ccx.content.post", "post.images"},
    {"ccx.content.set_details", "Detailed set membership", "ccx.content.model"},
    {"ccx.content.mesh_details", "Detailed mesh listings", "ccx.content.model"},
    {"ccx.content.result_details", "Detailed result listings", "ccx.content.post"}};
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
  const auto addBoolean = [&](const std::string& key, const std::string& label,
                              const std::string& group, bool enabled)
  {
    ReportOption option;
    option.key = key;
    option.label = label;
    option.group_id = group;
    option.kind = "boolean";
    option.default_value = "false";
    if (enabled)
    {
      option.default_value = "true";
    }
    candidate.defaults.values[key] = option.default_value;
    candidate.options.push_back(std::move(option));
  };
  addBoolean("ccx.model.sets.nodes", "Node membership", "ccx.content.set_details", false);
  addBoolean("ccx.model.sets.elements", "Element membership", "ccx.content.set_details", false);
  addBoolean("ccx.model.sets.faces", "Face membership", "ccx.content.set_details", false);
  addBoolean("ccx.model.mesh.nodes", "Node coordinates", "ccx.content.mesh_details", false);
  addBoolean("ccx.model.mesh.elements", "Element connectivity", "ccx.content.mesh_details", false);
  addBoolean("post.results.extrema", "Block minima and maxima", "ccx.content.post", true);
  addBoolean("post.results.nodal_values", "Nodal values", "ccx.content.result_details", false);
  addBoolean("post.results.element_values", "Element and integration-point values", "ccx.content.result_details", false);
  addBoolean("post.results.tables", "Additional source tables", "ccx.content.result_details", false);
  ReportOption frames;
  frames.key = "post.results.frames";
  frames.label = "Frame selection";
  frames.group_id = "ccx.content.post";
  frames.kind = "choice";
  frames.default_value = "last_per_step";
  frames.choices = {"last_per_step", "envelope", "all_frames"};
  candidate.defaults.values[frames.key] = frames.default_value;
  candidate.options.push_back(std::move(frames));
  const auto addChoice = [&](const std::string& key, const std::string& label,
                             const std::string& initial, const std::vector<std::string>& choices)
  {
    ReportOption option;
    option.key = key;
    option.label = label;
    option.group_id = "ccx.content.post_images";
    option.kind = "choice";
    option.default_value = initial;
    option.choices = choices;
    candidate.defaults.values[key] = initial;
    candidate.options.push_back(std::move(option));
  };
  addChoice("post.images.presentation", "Presentation", "inherited", {"inherited", "default"});
  addChoice("post.images.camera", "Camera", "preserve", {"preserve", "fit"});
  addChoice("post.images.background", "Background", "current", {"current", "white", "transparent"});
  ReportOption view;
  view.key = "post.images.view";
  view.label = "Postprocessor view (empty uses active view)";
  view.group_id = "ccx.content.post_images";
  view.kind = "text";
  candidate.defaults.values[view.key] = "";
  candidate.options.push_back(std::move(view));
  for (auto& option : candidate.options)
  {
    if (!option.group_id.empty())
    {
      continue;
    }
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
  MeshQualityDefaults::add_options(candidate);
  output = std::move(candidate);
  error.clear();
  return true;
}
