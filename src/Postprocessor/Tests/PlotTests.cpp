#include <type_traits>
#include "PlotTests.hpp"
#include "Postprocessor/Core/application/PostprocessorCore.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Core/plots/PlotCodec.hpp"
#include "Postprocessor/Core/tables/ResultTables.hpp"
#include "Postprocessor/GUI/Plots/PostprocessorPlotInteraction.hpp"
#include "Postprocessor/GUI/Plots/PostprocessorPlotLabels.hpp"
#include "Postprocessor/Commands/plots/PlotCommandText.hpp"
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QJsonObject>
#include <vtkSmartPointer.h>
#include <vtkTable.h>
#include <vtkDoubleArray.h>
#include <vtkTypeInt64Array.h>
#include <iostream>
#include <limits>
#include <cmath>
int plot_cases(const std::shared_ptr<PostprocessorApplicationInterface>& app)
{
  using namespace Postprocessor;
  using Selector = PostprocessorApplicationInterface::DatasetSelector;
  constexpr std::int64_t large = 9007199254740993LL;
  int failures = 0;
  const auto check = [&](bool value,const char* message)
  {
    if (!value)
    {
      ++failures;
      std::cerr << message << '\n';
    }
  };
  std::string error;
  const auto view = app->session().active_view_id;
  const std::string name = "History, \"A\"";
  check(app->create_plot(name,error),"Cannot create plot");
  check(!app->create_plot(name,error),"Duplicate plot name was accepted");
  check(app->add_plot_entity(name,"large node","Model",Selector{std::size_t{0}},EntityKind::Node,large,
    std::nullopt,"stress","SXX",false,PlotReduction::None,error),"Cannot create exact-ID node history");
  const auto id = app->session().active_plot_id;
  const auto revision = app->session().plots.at(id).data_revision;
  const auto meshRevision = app->display_revision();
  PlotAxes fixed;
  fixed.x = std::array<double,2>{0,2};
  fixed.y = std::array<double,2>{0,100};
  check(app->set_plot_axes(name,fixed,error) && app->session().plots.at(id).data_revision == revision &&
    app->display_revision() == meshRevision,"Axis changes invalidated curve data or the 3D scene");
  fixed.x = std::array<double,2>{1,0};
  check(!app->set_plot_axes(name,fixed,error),"Reversed axis bounds were accepted");
  check(app->set_plot_axes(name,PlotAxes{},error),"Cannot restore automatic axes");
  PlotData data;
  check(app->plot_data(id,data,error) && data.curves.size() == 1 && data.curves[0].samples.size() == 2 &&
    data.curves[0].samples[0].valid && data.curves[0].samples[0].y == 10 &&
    !data.curves[0].samples[1].valid,"Missing entity frame did not produce a gap");
  check(!app->add_plot_entity(name,"ambiguous element","Model",Selector{std::size_t{0}},EntityKind::Element,120,
    std::nullopt,"stress","SXX",false,PlotReduction::None,error),"Element nodal history silently averaged samples");
  check(app->add_plot_entity(name,"maximum","Model",Selector{std::size_t{0}},EntityKind::Element,120,
    std::nullopt,"stress","SXX",false,PlotReduction::Maximum,error),"Cannot create explicit element maximum");
  check(app->plot_data(id,data,error) && data.curves[1].samples[0].y == 40,"Element maximum used masked/incorrect samples");
  check(!app->add_plot_entity(name,"ip","Model",Selector{std::size_t{1}},EntityKind::IntegrationPoint,120,
    3,"ip_stress","SXX",false,PlotReduction::None,error),"Unknown raw fields were compared without a common unit basis");
  const std::string ipPlot = "Integration point history";
  check(app->create_plot(ipPlot,error),"Cannot create separate integration-point plot");
  check(app->add_plot_entity(ipPlot,"ip","Model",Selector{std::size_t{1}},EntityKind::IntegrationPoint,120,
    3,"ip_stress","SXX",false,PlotReduction::None,error),"Cannot create integration-point history");
  const auto ipPlotId = app->session().active_plot_id;
  check(app->plot_data(ipPlotId,data,error) && data.curves.size() == 1 &&
    data.curves[0].samples[0].y == 100 && !data.curves[0].samples[1].valid,
    "IP history did not preserve element/IP identity across frames");
  check(app->change_plot(name,"show","",error),"Cannot return to the nodal history plot");
  const auto labelRevision = app->session().plots.at(id).data_revision;
  check(app->set_plot_label(name,"title","Contact history","",error) &&
    app->set_plot_label(name,"xlabel","Time [s]","",error) && app->set_plot_label(name,"ylabel","Stress [MPa]","",error) &&
    app->set_plot_label(name,"curve","Selected node","large node",error),"Cannot edit plot presentation labels");
  check(app->session().plots.at(id).data_revision == labelRevision,"Label edits invalidated result samples");
  check(plot_label_command(name,"curve","Selected node","large node") ==
    QStringLiteral("post plot curve label \"Selected node\" curve \"large node\" plot ")+plot_quote(name),"Curve-label journal is not readable");
  check(!app->set_plot_label(name,"title",std::string(257,'x'),"",error),"Oversized plot label was accepted");
  const auto json = plots_to_json(app->session().plots);
  std::map<std::string,PlotDefinition> restored;
  check(plots_from_json(json,restored) &&
    std::get<PlotEntityReference>(restored.at(id).curves[0].input).entity.entity_id == large,
    "Plot persistence rounded a 64-bit ID");
  check(restored.at(id).curves[0].color == app->session().plots.at(id).curves[0].color,"Plot persistence lost curve color");
  check(restored.at(id).title == std::optional<std::string>("Contact history") &&
    restored.at(id).x_label == std::optional<std::string>("Time [s]") &&
    restored.at(id).curves[0].label == std::optional<std::string>("Selected node"),"Plot label persistence lost overrides");
  check(app->set_plot_label(name,"auto","","",error) && !app->session().plots.at(id).title &&
    !app->session().plots.at(id).curves[0].label,"Automatic labels did not reset overrides");
  auto duplicate = json;
  duplicate.append(json[0]);
  check(!plots_from_json(duplicate,restored),"Duplicate saved plots were accepted");
  check(plots_from_json(QJsonValue(QJsonValue::Undefined),restored) && restored.empty(),"Legacy project plot defaults failed");
  const auto source = app->session().active_source_id;
  auto unsorted = *app->result_data(source);
  unsorted.increments[0].time = 2;
  unsorted.increments[1].time = 1;
  PlotCurveData ordered;
  check(plot_entity_data(unsorted,std::get<PlotEntityReference>(app->session().plots.at(id).curves[0].input),ordered,error) &&
    ordered.samples[0].x == 2 && ordered.samples[1].x == 1,"History reordered frames by time");
  QTemporaryDir directory;
  const auto path = directory.filePath("plot.csv");
  check(app->export_plot(name,path.toStdString(),error),"Cannot export plot CSV");
  QFile csv(path);
  check(csv.open(QIODevice::ReadOnly),"Cannot read CSV fixture");
  const auto bytes = csv.readAll();
  csv.close();
  check(bytes.contains("9007199254740993") && bytes.contains("\"History, \"\"A\"\"\""),
    "CSV lost exact IDs or RFC4180 quote escaping");
  check(!app->export_plot(name,path.toStdString(),error),"CSV export overwrote an existing file");
  check(QDir(directory.path()).entryList({".cubitsim-plot-*.tmp"},QDir::Files|QDir::Hidden).isEmpty(),
    "CSV export left a temporary staging file");
  const auto fresh = directory.filePath(QString::fromUtf8("Desktop export ä"));
  check(QDir().mkpath(fresh),"Cannot create fresh CSV destination fixture");
  const auto freshPath = QDir(fresh).filePath("new plot.csv");
  check(app->export_plot(name,freshPath.toUtf8().toStdString(),error) && QFile::exists(freshPath),
    "CSV publication failed in a fresh directory with spaces/non-ASCII characters");
  check(QDir(fresh).entryList({".cubitsim-plot-*.tmp"},QDir::Files|QDir::Hidden).isEmpty(),
    "Fresh-directory CSV publication left temporary files");
  QFile unchanged(path);
  check(unchanged.open(QIODevice::ReadOnly) && unchanged.readAll() == bytes,
    "A rejected overwrite changed the existing CSV");
  check(app->change_plot_curve(name,"maximum","hide",error) &&
    app->session().plots.at(id).curves[1].visible == false,"Cannot hide a curve");
  check(app->change_plot(name,"close","",error) && !app->session().views.at(view).plot_id &&
    app->session().plots.count(id) == 1,"Closing deleted the plot definition");
  check(app->change_plot(name,"show","",error) && app->session().views.at(view).plot_id == id,"Cannot reopen saved plot");

  auto tables = std::make_shared<SimResults::VtkResultsData>();
  auto table = std::shared_ptr<vtkTable>(vtkTable::New(),[](vtkTable* value)
  {
    value->Delete();
  });
  auto ids = vtkSmartPointer<vtkTypeInt64Array>::New();
  ids->SetName("NodeId");
  ids->InsertNextValue(large);
  ids->InsertNextValue(large+1);
  auto x = vtkSmartPointer<vtkDoubleArray>::New();
  x->SetName("Time");
  x->InsertNextValue(0.1);
  x->InsertNextValue(0.2);
  auto y = vtkSmartPointer<vtkDoubleArray>::New();
  y->SetName("Force");
  y->InsertNextValue(7);
  y->InsertNextValue(9);
  table->AddColumn(ids);
  table->AddColumn(x);
  table->AddColumn(y);
  tables->tables.push_back(table);
  PlotTableReference reference{"s",result_tables(*tables,std::nullopt).front().key,"Time","Force","NodeId",std::to_string(large)};
  PlotCurveData filtered;
  check(plot_table_data(tables,reference,filtered,error) && filtered.samples.size() == 1 &&
    filtered.samples[0].y == 7 && !filtered.samples[0].frame,"Table filter rounded IDs or invented global frame membership");

  reference.filter_column.clear();
  reference.filter_value.clear();
  PlotCurveData mixed;
  check(!plot_table_data(tables,reference,mixed,error),"Global table silently connected different entity histories");

  TableFilter exactNode;
  exactNode.column = "NodeId";
  exactNode.value = std::to_string(large);
  TableFilter forceRange;
  forceRange.column = "Force";
  forceRange.mode = TableFilterMode::Range;
  forceRange.minimum = 7;
  forceRange.maximum = 7;
  reference.filters = {exactNode,forceRange};
  check(plot_table_data(tables,reference,filtered,error) && filtered.samples.size() == 1 && filtered.samples[0].y == 7,
    "Plot AND filters rounded identifiers or ignored inclusive bounds");
  PlotDefinition tablePlot;
  tablePlot.id = "filtered";
  tablePlot.name = "Filtered table";
  PlotCurveDefinition tableCurve;
  tableCurve.name = "History";
  tableCurve.input = reference;
  tablePlot.curves.push_back(tableCurve);
  std::map<std::string,PlotDefinition> roundTrip;
  check(plots_from_json(plots_to_json({{tablePlot.id,tablePlot}}),roundTrip) &&
    same_table_filters(std::get<PlotTableReference>(roundTrip.at("filtered").curves[0].input).filters,reference.filters),
    "Plot persistence lost multiple column filters");
  PlotAxes stateAxes, actual;
  actual.x = std::array<double,2>{0,10};
  actual.y = std::array<double,2>{0,100};
  std::uint64_t token = 1;
  int previews = 0, journaled = 0;
  QObject owner;
  PostprocessorPlotInteraction interaction([&]()
  {
    return actual;
  },[&]()
  {
    return stateAxes;
  },[]()
  {
    return QSizeF(500,300);
  },[&]()
  {
    return token;
  },[&](const PlotAxes& axes,bool journal)
  {
    stateAxes = axes;
    actual = axes;
    if (!actual.x)
    {
      actual.x = std::array<double,2>{0,10};
      actual.y = std::array<double,2>{0,100};
    }
    ++token;
    [&]() -> std::common_type_t<decltype(++journaled), decltype(++previews)> {
      if (journal) {
        return ++journaled;
      } else {
        return ++previews;
      }
    }();
    return true;
  },[]()
  {
  },&owner);
  QMouseEvent press(QEvent::MouseButtonPress,QPointF(0,0),QPointF(0,0),Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
  QMouseEvent move(QEvent::MouseMove,QPointF(100,30),QPointF(100,30),Qt::NoButton,Qt::LeftButton,Qt::NoModifier);
  QMouseEvent release(QEvent::MouseButtonRelease,QPointF(100,30),QPointF(100,30),Qt::LeftButton,Qt::NoButton,Qt::NoModifier);
  check(interaction.handle(&press) && interaction.handle(&move),"Plot pan was not intercepted");
  interaction.flush_preview();
  check(previews == 1 && journaled == 0 && *actual.x == std::array<double,2>{-2,8},
    "Plot pan did not use silent previews or correct data-space translation");
  interaction.handle(&release);
  check(journaled == 1,"Plot pan release did not journal exactly once");
  const auto baseline = stateAxes;
  interaction.handle(&press);
  interaction.handle(&move);
  interaction.flush_preview();
  QKeyEvent escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);
  interaction.handle(&escape);
  check(journaled == 1 && stateAxes.x == baseline.x && stateAxes.y == baseline.y,"Esc did not restore plot axes silently");
  check(app->change_plot(name,"remove","",error) && app->session().plots.empty(),"Cannot remove plot definition");
  PostprocessorCore standalone;
  check(standalone.create_plot("Before reset",error),"Cannot prepare reset fixture");
  const auto beforeReset = standalone.session().plot_revision;
  check(standalone.reset(error) && standalone.session().plots.empty() && standalone.session().plot_revision > beforeReset,
    "Reset reused old plot gesture/cache revisions");
  return failures;
}
