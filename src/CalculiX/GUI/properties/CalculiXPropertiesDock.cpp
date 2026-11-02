#include "CalculiXPropertiesDock.hpp"
#include "PropertyTreeWidget.hpp"
#include "CubitMessage.hpp"
#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>
#include "GUI/loads/LoadsForcesPropertiesWidget.hpp"
#include "GUI/loads/LoadsPressuresPropertiesWidget.hpp"
#include "GUI/loads/LoadsHeatfluxesPropertiesWidget.hpp"
#include "GUI/loads/LoadsSurfaceTractionPropertiesWidget.hpp"
#include "GUI/loads/LoadsGravityPropertiesWidget.hpp"
#include "GUI/loads/LoadsCentrifugalPropertiesWidget.hpp"
#include "GUI/loads/LoadsFilmPropertiesWidget.hpp"
#include "GUI/loads/LoadsRadiationPropertiesWidget.hpp"
#include "GUI/loads/LoadsBodyHeatfluxPropertiesWidget.hpp"
#include "GUI/bcs/BCsTemperaturesPropertiesWidget.hpp"
#include "GUI/bcs/BCsDisplacementsPropertiesWidget.hpp"
#include "GUI/sections/SectionsPropertiesWidget.hpp"
#include "GUI/sections/OrientationsPropertiesWidget.hpp"
#include "GUI/interactions/ConstraintsPropertiesWidget.hpp"
#include "GUI/interactions/ContactPairsPropertiesWidget.hpp"
#include "GUI/amplitudes/AmplitudesPropertiesWidget.hpp"
#include "GUI/export/CustomLinesPropertiesWidget.hpp"
#include "GUI/steps/StepsPropertiesWidget.hpp"
#include "GUI/steps/HistoryOutputsPropertiesWidget.hpp"
#include "GUI/steps/FieldOutputsPropertiesWidget.hpp"
#include "GUI/steps/InitialConditionsPropertiesWidget.hpp"
#include "GUI/jobs/JobsPropertiesWidget.hpp"
#include "GUI/steps/DampingPropertiesWidget.hpp"
#include "GUI/steps/PhysicalConstantsPropertiesWidget.hpp"
#include "GUI/interactions/SurfaceInteractionsPropertiesWidget.hpp"
#include "GUI/loads/LoadsTrajectoryPropertiesWidget.hpp"
#include "GUI/mesh/BlocksPropertiesWidget.hpp"
#include "GUI/mesh/NodesetPropertiesWidget.hpp"
#include "GUI/mesh/SidesetPropertiesWidget.hpp"
#include "GUI/materials/MaterialPropertiesWidget.hpp"
#include "GUI/bcs/HBCsPropertiesWidget.hpp"
#include "GUI/topology/TopologyPropertiesWidget.hpp"
#include "GUI/topology/TopologyIterationPropertiesWidget.hpp"

CalculiXPropertiesDock::CalculiXPropertiesDock(QWidget* parent)
  : QDockWidget(tr("CalculiX Properties"), parent)
{
  setObjectName("CalculiX.Properties");
  setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  auto* content = new QWidget(this);
  auto* layout = new QVBoxLayout(content);
  layout->setContentsMargins(0, 0, 0, 0);
  heading = new QWidget(content);
  auto* header = new QHBoxLayout(heading);
  header->setContentsMargins(6, 4, 6, 4);
  icon = new QLabel(heading);
  caption = new QLabel(heading);
  caption->setTextFormat(Qt::PlainText);
  caption->setTextInteractionFlags(Qt::TextSelectableByMouse);
  header->addWidget(icon);
  header->addWidget(caption, 1);
  pages = new QStackedWidget(content);
  emptyPage = new QWidget(pages);
  pages->addWidget(emptyPage);
  layout->addWidget(heading);
  layout->addWidget(pages, 1);
  setWidget(content);
  clearSelection();
  hide();
}

void CalculiXPropertiesDock::saveCurrentViewState()
{
  if (currentSelection.feature == PropertyFeature::None)
  {
    return;
  }
  if (auto* active = dynamic_cast<PropertyTreeWidget*>(pages->currentWidget()))
  {
    const auto captured = active->captureViewState();
    auto& saved = viewStates[currentSelection.feature];
    // Retain keys for subtype sections that are temporarily absent.
    for (auto entry = captured.expanded.cbegin(); entry != captured.expanded.cend(); ++entry)
    {
      saved.expanded.insert(entry.key(), entry.value());
    }
    saved.header = captured.header;
    saved.verticalScroll = captured.verticalScroll;
    saved.horizontalScroll = captured.horizontalScroll;
  }
}

void CalculiXPropertiesDock::showEmptyPage()
{
  currentSelection = {};
  heading->hide();
  caption->clear();
  icon->clear();
  pages->setCurrentWidget(emptyPage);
}

void CalculiXPropertiesDock::clearSelection()
{
  saveCurrentViewState();
  showEmptyPage();
}

void CalculiXPropertiesDock::reset()
{
  clearSelection();
  for (const auto& entry : widgets)
  {
    entry.second->clear();
  }
}

void CalculiXPropertiesDock::refreshSelection()
{
  const PropertySelection selection = currentSelection;
  showSelection(selection);
}

void CalculiXPropertiesDock::showSelection(const PropertySelection& selection)
{
  // Copy first: refreshSelection can be called with the existing selection.
  const PropertySelection requested = selection;
  const bool sameEntity = requested.feature == currentSelection.feature &&
    requested.id == currentSelection.id && requested.stepId == currentSelection.stepId &&
    requested.topologyId == currentSelection.topologyId && requested.iterationId == currentSelection.iterationId &&
    requested.homogeneous == currentSelection.homogeneous;
  saveCurrentViewState();
  showEmptyPage();
  if (requested.feature == PropertyFeature::None || requested.id < 0)
  {
    return;
  }
  PropertyTreeWidget* active = nullptr;
  bool loaded = false;
  switch (requested.feature)
  {
    case PropertyFeature::Force:
    {
      auto& cached = widgets[PropertyFeature::Force];
      if (cached == nullptr)
      {
        cached = new LoadsForcesPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<LoadsForcesPropertiesWidget*>(cached);
      loaded = widget->setForce(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::Pressure:
    {
      auto& cached = widgets[PropertyFeature::Pressure];
      if (cached == nullptr)
      {
        cached = new LoadsPressuresPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<LoadsPressuresPropertiesWidget*>(cached);
      loaded = widget->setPressure(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::Heatflux:
    {
      auto& cached = widgets[PropertyFeature::Heatflux];
      if (cached == nullptr)
      {
        cached = new LoadsHeatfluxesPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<LoadsHeatfluxesPropertiesWidget*>(cached);
      loaded = widget->setHeatflux(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::SurfaceTraction:
    {
      auto& cached = widgets[PropertyFeature::SurfaceTraction];
      if (cached == nullptr)
      {
        cached = new LoadsSurfaceTractionPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<LoadsSurfaceTractionPropertiesWidget*>(cached);
      loaded = widget->setSurfaceTraction(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::Gravity:
    {
      auto& cached = widgets[PropertyFeature::Gravity];
      if (cached == nullptr)
      {
        cached = new LoadsGravityPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<LoadsGravityPropertiesWidget*>(cached);
      loaded = widget->setGravity(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::Centrifugal:
    {
      auto& cached = widgets[PropertyFeature::Centrifugal];
      if (cached == nullptr)
      {
        cached = new LoadsCentrifugalPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<LoadsCentrifugalPropertiesWidget*>(cached);
      loaded = widget->setCentrifugal(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::Film:
    {
      auto& cached = widgets[PropertyFeature::Film];
      if (cached == nullptr)
      {
        cached = new LoadsFilmPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<LoadsFilmPropertiesWidget*>(cached);
      loaded = widget->setFilm(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::Radiation:
    {
      auto& cached = widgets[PropertyFeature::Radiation];
      if (cached == nullptr)
      {
        cached = new LoadsRadiationPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<LoadsRadiationPropertiesWidget*>(cached);
      loaded = widget->setRadiation(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::BodyHeatflux:
    {
      auto& cached = widgets[PropertyFeature::BodyHeatflux];
      if (cached == nullptr)
      {
        cached = new LoadsBodyHeatfluxPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<LoadsBodyHeatfluxPropertiesWidget*>(cached);
      loaded = widget->setBodyHeatflux(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::Temperature:
    {
      auto& cached = widgets[PropertyFeature::Temperature];
      if (cached == nullptr)
      {
        cached = new BCsTemperaturesPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<BCsTemperaturesPropertiesWidget*>(cached);
      loaded = widget->setTemperature(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::Displacement:
    {
      auto& cached = widgets[PropertyFeature::Displacement];
      if (cached == nullptr)
      {
        cached = new BCsDisplacementsPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<BCsDisplacementsPropertiesWidget*>(cached);
      loaded = widget->setDisplacement(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::Section:
    {
      auto& cached = widgets[PropertyFeature::Section];
      if (cached == nullptr)
      {
        cached = new SectionsPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<SectionsPropertiesWidget*>(cached);
      loaded = widget->setSection(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::Orientation:
    {
      auto& cached = widgets[PropertyFeature::Orientation];
      if (cached == nullptr)
      {
        cached = new OrientationsPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<OrientationsPropertiesWidget*>(cached);
      loaded = widget->setOrientation(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::Constraint:
    {
      auto& cached = widgets[PropertyFeature::Constraint];
      if (cached == nullptr)
      {
        cached = new ConstraintsPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<ConstraintsPropertiesWidget*>(cached);
      loaded = widget->setConstraint(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::ContactPair:
    {
      auto& cached = widgets[PropertyFeature::ContactPair];
      if (cached == nullptr)
      {
        cached = new ContactPairsPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<ContactPairsPropertiesWidget*>(cached);
      loaded = widget->setContactPair(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::Amplitude:
    {
      auto& cached = widgets[PropertyFeature::Amplitude];
      if (cached == nullptr)
      {
        cached = new AmplitudesPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<AmplitudesPropertiesWidget*>(cached);
      loaded = widget->setAmplitude(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::CustomLine:
    {
      auto& cached = widgets[PropertyFeature::CustomLine];
      if (cached == nullptr)
      {
        cached = new CustomLinesPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<CustomLinesPropertiesWidget*>(cached);
      loaded = widget->setCustomLine(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::Step:
    {
      auto& cached = widgets[PropertyFeature::Step];
      if (cached == nullptr)
      {
        cached = new StepsPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<StepsPropertiesWidget*>(cached);
      loaded = widget->setStep(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::HistoryOutput:
    {
      auto& cached = widgets[PropertyFeature::HistoryOutput];
      if (cached == nullptr)
      {
        cached = new HistoryOutputsPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<HistoryOutputsPropertiesWidget*>(cached);
      loaded = widget->setHistoryOutput(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::FieldOutput:
    {
      auto& cached = widgets[PropertyFeature::FieldOutput];
      if (cached == nullptr)
      {
        cached = new FieldOutputsPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<FieldOutputsPropertiesWidget*>(cached);
      loaded = widget->setFieldOutput(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::InitialCondition:
    {
      auto& cached = widgets[PropertyFeature::InitialCondition];
      if (cached == nullptr)
      {
        cached = new InitialConditionsPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<InitialConditionsPropertiesWidget*>(cached);
      loaded = widget->setInitialCondition(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::Job:
    {
      auto& cached = widgets[PropertyFeature::Job];
      if (cached == nullptr)
      {
        cached = new JobsPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<JobsPropertiesWidget*>(cached);
      loaded = widget->setJob(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::Damping:
    {
      auto& cached = widgets[PropertyFeature::Damping];
      if (cached == nullptr)
      {
        cached = new DampingPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<DampingPropertiesWidget*>(cached);
      loaded = widget->setDamping(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::PhysicalConstants:
    {
      auto& cached = widgets[PropertyFeature::PhysicalConstants];
      if (cached == nullptr)
      {
        cached = new PhysicalConstantsPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<PhysicalConstantsPropertiesWidget*>(cached);
      loaded = widget->setPhysicalConstants(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::SurfaceInteraction:
    {
      auto& cached = widgets[PropertyFeature::SurfaceInteraction];
      if (cached == nullptr)
      {
        cached = new SurfaceInteractionsPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<SurfaceInteractionsPropertiesWidget*>(cached);
      loaded = widget->setSurfaceInteraction(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::Trajectory:
    {
      auto& cached = widgets[PropertyFeature::Trajectory];
      if (cached == nullptr)
      {
        cached = new LoadsTrajectoryPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<LoadsTrajectoryPropertiesWidget*>(cached);
      loaded = widget->setTrajectory(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::Block:
    {
      auto& cached = widgets[PropertyFeature::Block];
      if (cached == nullptr)
      {
        cached = new BlocksPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<BlocksPropertiesWidget*>(cached);
      loaded = widget->setBlock(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::Nodeset:
    {
      auto& cached = widgets[PropertyFeature::Nodeset];
      if (cached == nullptr)
      {
        cached = new NodesetPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<NodesetPropertiesWidget*>(cached);
      loaded = widget->setNodeset(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::Sideset:
    {
      auto& cached = widgets[PropertyFeature::Sideset];
      if (cached == nullptr)
      {
        cached = new SidesetPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<SidesetPropertiesWidget*>(cached);
      loaded = widget->setSideset(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::Material:
    {
      auto& cached = widgets[PropertyFeature::Material];
      if (cached == nullptr)
      {
        cached = new MaterialPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<MaterialPropertiesWidget*>(cached);
      loaded = widget->setMaterial(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::HBCs:
    {
      auto& cached = widgets[PropertyFeature::HBCs];
      if (cached == nullptr)
      {
        cached = new HBCsPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<HBCsPropertiesWidget*>(cached);
      loaded = widget->setHBCs(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::Topology:
    {
      auto& cached = widgets[PropertyFeature::Topology];
      if (cached == nullptr)
      {
        cached = new TopologyPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<TopologyPropertiesWidget*>(cached);
      loaded = widget->setTopology(requested.id);
      active = widget;
      break;
    }
    case PropertyFeature::TopologyIteration:
    {
      auto& cached = widgets[PropertyFeature::TopologyIteration];
      if (cached == nullptr)
      {
        cached = new TopologyIterationPropertiesWidget(pages);
        pages->addWidget(cached);
      }
      auto* widget = static_cast<TopologyIterationPropertiesWidget*>(cached);
      loaded = widget->setTopologyIteration(requested.topologyId, requested.iterationId);
      active = widget;
      break;
    }
    default:
      PRINT_WARNING("The selected feature has no property view.\n");
      return;
  }
  if (!loaded || active == nullptr)
  {
    clearSelection();
    return;
  }
  active->addContext(requested);
  currentSelection = requested;
  caption->setText(requested.caption);
  caption->setToolTip(requested.caption);
  icon->setPixmap(requested.icon.pixmap(20, 20));
  heading->show();
  pages->setCurrentWidget(active);
  const auto saved = viewStates.find(requested.feature);
  active->restoreViewState(saved == viewStates.end() ? PropertyViewState{} : saved->second, sameEntity);
}
