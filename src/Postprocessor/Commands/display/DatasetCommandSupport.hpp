#include <type_traits>
#ifndef POSTPROCESSOR_DATASETCOMMANDSUPPORT_HPP
#define POSTPROCESSOR_DATASETCOMMANDSUPPORT_HPP
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitCommandInterface.hpp"
#include <cmath>

namespace DatasetCommandSupport
{
  inline std::vector<std::string> syntax(bool show)
  {
    const std::string source = " [source <string:type='quoted',number='1',label='source_name'>]";
    const std::string prefix = [&]() -> std::string {
      if (show) {
        return "post show ";
      } else {
        return "post hide ";
      }
    }();
    std::vector<std::string> result = {prefix+"{blocks|nodesets|sidesets|integrationpoints}"+source};
    for (const auto& selector : {std::string("<value:label='dataset_id'>"),
        std::string("<string:type='quoted',number='1',label='dataset_name'>")})
    {
      result.push_back(prefix+"dataset "+selector+source);
      if (show)
      {
        result.push_back(prefix+"only dataset "+selector+source);
      }
    }
    if (show)
    {
      result.push_back("post show dataset all"+source);
    }
    return result;
  }

  inline bool execute(CubitCommandData& data, PostprocessorApplicationInterface& application,
    bool show, std::string& error)
  {
    std::string source;
    if (data.find_keyword("SOURCE") && (!data.get_string("source_name",source) || source.empty()))
    {
      error = "A nonempty quoted source name is required.";
      return false;
    }
    for (auto category : {Postprocessor::DatasetCategory::Blocks,Postprocessor::DatasetCategory::NodeSets,
        Postprocessor::DatasetCategory::SideSets,Postprocessor::DatasetCategory::IntegrationPoints})
    {
      if (data.find_keyword([&]() -> std::common_type_t<decltype("BLOCKS"), std::common_type_t<decltype("NODESETS"), const char*>> {
        if (category == Postprocessor::DatasetCategory::Blocks) {
          return "BLOCKS";
        } else {
          return [&]() -> std::common_type_t<decltype("NODESETS"), const char*> {
            if (category == Postprocessor::DatasetCategory::NodeSets) {
              return "NODESETS";
            } else {
              return [&]() -> const char* {
                if (category == Postprocessor::DatasetCategory::SideSets) {
                  return "SIDESETS";
                } else {
                  return "INTEGRATIONPOINTS";
                }
              }();
            }
          }();
        }
      }()))
      {
        return application.set_category_visible(source,category,show,error);
      }
    }
    using Action = Postprocessor::DatasetVisibilityAction;
    std::optional<PostprocessorApplicationInterface::DatasetSelector> selector;
    auto action = [&]() -> std::common_type_t<decltype(Action::Show), decltype(Action::Hide)> {
      if (show) {
        return Action::Show;
      } else {
        return Action::Hide;
      }
    }();
    std::string label;
    double number;
    // Read quoted names first: a dataset literally named "all" is not Show all.
    if (data.get_string("dataset_name",label))
    {
      selector = label;
    }
    else if (data.find_keyword("ALL") && show)
    {
      action = Action::ShowAll;
    }
    else if (data.get_value("dataset_id",number) && std::isfinite(number) && number >= 0 &&
        number == std::floor(number) && number <= 2147483647)
    {
      selector = static_cast<std::size_t>(number);
    }
    else
    {
      error = "Dataset requires an ID in [0,2147483647] or a quoted published name.";
      return false;
    }
    if (show && data.find_keyword("ONLY"))
    {
      action = Action::ShowOnly;
    }
    return application.set_dataset_visibility(source,action,selector,error);
  }
}
#endif
