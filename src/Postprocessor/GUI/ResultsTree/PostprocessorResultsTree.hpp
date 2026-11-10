#ifndef POSTPROCESSORRESULTSTREE_HPP
#define POSTPROCESSORRESULTSTREE_HPP

#include "Postprocessor/Core/session/PostprocessorSession.hpp"
#include "Framework/Results/VtkResultsData.hpp"
#include "Postprocessor/Core/representations/ResultColoring.hpp"
#include <QTreeWidget>
#include "Postprocessor/Core/tables/ResultTables.hpp"
#include <functional>
#include "Framework/Results/ResultJobEntry.hpp"
#include <map>

enum class ResultTreeItemKind
{
  Source,
  MeshGroup,
  Dataset,
  FieldsGroup,
  Field,
  ResultGroup,
  Table,
  Category
};

enum class SourceAction
{
  Show, Hide, ShowOnly, Refresh, Remove, RemoveAll
};

class PostprocessorResultsTree : public QTreeWidget
{
public:
  std::function<bool(std::vector<SimResults::ResultJobEntry>&,std::string&)> available_jobs;
  std::function<void(const SimResults::ResultJobEntry&)> add_job;
  std::function<void()> import_files;
  std::function<void(const QString&,SourceAction)> source_action;
  using Availability = std::function<std::string(const std::string&)>;
  using Tables = std::function<std::vector<Postprocessor::ResultTableDescriptor>(const std::string&)>;
  using Load = std::function<std::shared_ptr<const SimResults::VtkResultsData>(const std::string&)>;
  using Groups = std::function<std::vector<Postprocessor::ResultFieldGroup>(const std::string&,std::optional<std::size_t>)>;
  using HasMesh = std::function<bool(const std::string&, std::size_t)>;
  using Datasets = std::function<std::vector<SimResults::Dataset>(const std::string&)>;
  PostprocessorResultsTree(std::function<void(const QString&)> select,
    std::function<void(const QString&, bool)> visibility, QWidget* parent = nullptr);
  std::function<void(const QString&, std::size_t)> activate_field;
  std::function<void(const QString&,const QString&,bool)> open_table;
  std::function<void(const QString&,Postprocessor::DatasetVisibilityAction,std::optional<std::size_t>)> dataset_action;
  std::function<void(const QString&,Postprocessor::DatasetCategory,bool)> category_action;
  void refresh(const Postprocessor::PostprocessorSession& session, const Load& load, const HasMesh& has_mesh,
    const Groups& catalogue = {}, const Datasets& datasets = {}, const Tables& tables = {}, const Availability& availability = {});
private:
  std::string activeView;
  std::string activeSource;
  bool refreshing = false;
  std::map<std::string,std::shared_ptr<const SimResults::VtkResultsData>> snapshots;
};

#endif
