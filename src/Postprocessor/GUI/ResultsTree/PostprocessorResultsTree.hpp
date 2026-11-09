#ifndef POSTPROCESSORRESULTSTREE_HPP
#define POSTPROCESSORRESULTSTREE_HPP

#include "Postprocessor/Core/session/PostprocessorSession.hpp"
#include "Framework/Results/VtkResultsData.hpp"
#include "Postprocessor/Core/representations/ResultColoring.hpp"
#include <QTreeWidget>
#include <functional>
#include <map>

enum class ResultTreeItemKind
{
  Source,
  MeshGroup,
  Dataset,
  FieldsGroup,
  Field,
  ResultGroup
};

class PostprocessorResultsTree : public QTreeWidget
{
public:
  using Load = std::function<std::shared_ptr<const SimResults::VtkResultsData>(const std::string&)>;
  using Groups = std::function<std::vector<Postprocessor::ResultFieldGroup>(const std::string&,std::optional<std::size_t>)>;
  using HasMesh = std::function<bool(const std::string&, std::size_t)>;
  using Datasets = std::function<std::vector<SimResults::Dataset>(const std::string&)>;
  PostprocessorResultsTree(std::function<void(const QString&)> select,
    std::function<void(const QString&, bool)> visibility, QWidget* parent = nullptr);
  std::function<void(const QString&, std::size_t)> activate_field;
  std::function<void(const QString&,Postprocessor::DatasetVisibilityAction,std::optional<std::size_t>)> dataset_action;
  void refresh(const Postprocessor::PostprocessorSession& session, const Load& load, const HasMesh& has_mesh,
    const Groups& catalogue = {}, const Datasets& datasets = {});
private:
  std::string activeView;
  std::map<std::string,std::shared_ptr<const SimResults::VtkResultsData>> snapshots;
};

#endif
