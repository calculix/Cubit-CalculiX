#include "Postprocessor/GUI/Properties/PostprocessorDisplayProperties.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Framework/Interfaces/ResultSourceProviderInterface.hpp"
#include "Framework/Interfaces/ResultSourceRegistryInterface.hpp"
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <vtkCellType.h>
#include <vtkDataObject.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>

namespace
{
  int failures = 0;
  void check(bool condition,const char* message)
  {
    if (!condition)
    {
      std::cerr << message << '\n';
      ++failures;
    }
  }
  class Provider : public ResultSourceProviderInterface
  {
  public:
    std::shared_ptr<SimResults::VtkResultsData> data;
    Provider()
    {
      data = std::make_shared<SimResults::VtkResultsData>();
      data->job_id = 1;
      data->run_identity = "properties-test";
      auto grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
      auto points = vtkSmartPointer<vtkPoints>::New();
      points->InsertNextPoint(0,0,0);
      points->InsertNextPoint(1,0,0);
      points->InsertNextPoint(0,1,0);
      points->InsertNextPoint(0,0,1);
      grid->SetPoints(points);
      grid->Allocate(1);
      vtkIdType ids[] = {0,1,2,3};
      grid->InsertNextCell(VTK_TETRA,4,ids);
      auto array = vtkSmartPointer<vtkDoubleArray>::New();
      array->SetName("stress");
      array->SetNumberOfTuples(4);
      array->FillComponent(0,17);
      grid->GetPointData()->AddArray(array);
      SimResults::Increment frame;
      grid->Register(nullptr);
      frame.dataset = std::shared_ptr<vtkDataObject>(grid.GetPointer(),[](vtkDataObject* object)
      {
        object->Delete();
      });
      SimResults::Field field;
      field.name = "Stress";
      field.array_name = "stress";
      field.components = {"SXX"};
      frame.fields.push_back(field);
      data->increments.push_back(frame);
      frame.time = 1;
      data->increments.push_back(frame);
    }
    std::string command_prefix() const override
    {
      return "fake";
    }
    bool open(const SimResults::ResultSourceReference& request,SimResults::PublishedResultSource& result,std::string& error) override
    {
      auto reference = std::get<SimResults::JobSourceReference>(request);
      reference.run_identity = data->run_identity;
      result = {reference,"Mesh",data};
      error.clear();
      return true;
    }
    bool refresh(const SimResults::ResultSourceReference& request,SimResults::PublishedResultSource& result,std::string& error) override
    {
      return open(request,result,error);
    }
  };
  class Registry : public ResultSourceRegistryInterface
  {
  public:
    mutable Provider provider;
    bool resolve_result_provider(const std::string& prefix,ResolvedResultSourceProvider& result,std::string& error) const override
    {
      if (prefix != "fake")
      {
        error = "Unknown provider";
        return false;
      }
      result.application_id = "test";
      result.provider = &provider;
      return true;
    }
  };
}

int main(int argc,char** argv)
{
  QApplication qt(argc,argv);
  using namespace Postprocessor;
  Registry registry;
  auto application = std::make_shared<PostprocessorApplicationInterface>(registry);
  std::string error;
  check(application->open_job("fake",1,std::nullopt,error),"Cannot open property fixture");
  check(application->color_field("Mesh","Stress","SXX",false,std::nullopt,error),"Cannot select property fixture field");
  std::vector<std::string> sent;
  std::size_t failAt = 0;
  bool rejectWithoutFailure = false;
  PostprocessorDisplayProperties* activePage = nullptr;
  PostprocessorDisplayProperties page(application,[&](const std::string& command,std::string& issue)
  {
    sent.push_back(command);
    if (failAt == sent.size())
    {
      issue = "Injected submission failure";
      return false;
    }
    if (rejectWithoutFailure)
    {
      return true;
    }
    const auto suffix = command.rfind(" source ");
    if (suffix == std::string::npos)
    {
      issue = "Missing explicit source";
      return false;
    }
    std::istringstream sourceStream(command.substr(suffix+8));
    std::string source;
    sourceStream >> std::quoted(source);
    auto settings = application->coloring(source,issue);
    if (!settings)
    {
      return false;
    }
    auto& c = *settings;
    std::istringstream stream(command.substr(0,suffix));
    stream.imbue(std::locale::classic());
    std::string post,kind,operation,value;
    stream >> post >> kind >> operation;
    if (kind == "legend")
    {
      c.legend_visible = operation == "on";
    }
    else if (operation == "palette")
    {
      stream >> value;
      if (value == "viridis")
      {
        c.palette = ColorPalette::Viridis;
      }
      else if (value == "bluewhitered")
      {
        c.palette = ColorPalette::BlueWhiteRed;
      }
      else if (value == "turbo")
      {
        c.palette = ColorPalette::Turbo;
      }
      else if (value == "rainbow")
      {
        c.palette = ColorPalette::Rainbow;
      }
      else
      {
        c.palette = ColorPalette::Grayscale;
      }
    }
    else if (operation == "reverse")
    {
      stream >> value;
      c.reversed = value == "on";
    }
    else if (operation == "bands")
    {
      stream >> value;
      if (value == "continuous")
      {
        c.bands.reset();
      }
      else
      {
        c.bands = static_cast<unsigned int>(std::stoul(value));
      }
    }
    else if (operation == "range")
    {
      stream >> value;
      if (value == "auto")
      {
        c.range_mode = ColorRangeMode::Automatic;
      }
      else if (value == "allframes")
      {
        c.range_mode = ColorRangeMode::AllFrames;
      }
      else if (value == "symmetric")
      {
        c.range_mode = ColorRangeMode::Symmetric;
      }
      else
      {
        c.range_mode = ColorRangeMode::Fixed;
        c.fixed_range[0] = std::stod(value);
        stream >> c.fixed_range[1];
      }
    }
    else if (operation == "limit")
    {
      stream >> value;
      auto& limit = value == "minimum" ? c.minimum : c.maximum;
      stream >> value;
      if (value == "on" || value == "off")
      {
        limit.enabled = value == "on";
      }
      else
      {
        limit.value = std::stod(value);
        stream >> value >> limit.color[0] >> limit.color[1] >> limit.color[2];
        limit.enabled = true;
      }
    }
    else
    {
      issue = "Unexpected command";
      return false;
    }
    const bool accepted = application->set_coloring(source,c,issue);
    if (activePage)
    {
      activePage->refresh(); // Synchronous observer refresh must not wipe a plan.
    }
    return accepted;
  });
  activePage = &page;
  auto* palette = page.findChild<QComboBox*>("Postprocessor.Display.Palette");
  auto* range = page.findChild<QComboBox*>("Postprocessor.Display.Range");
  auto* low = page.findChild<QLineEdit*>("Postprocessor.Display.RangeMinimum");
  auto* high = page.findChild<QLineEdit*>("Postprocessor.Display.RangeMaximum");
  auto* minimum = page.findChild<QCheckBox*>("Postprocessor.Display.MinimumEnabled");
  auto* maximum = page.findChild<QCheckBox*>("Postprocessor.Display.MaximumEnabled");
  auto* minValue = page.findChild<QLineEdit*>("Postprocessor.Display.MinimumValue");
  auto* maxValue = page.findChild<QLineEdit*>("Postprocessor.Display.MaximumValue");
  auto* reverse = page.findChild<QCheckBox*>("Postprocessor.Display.Reverse");
  auto* apply = page.findChild<QPushButton*>("Postprocessor.Display.Apply");
  auto* message = page.findChild<QLabel*>("Postprocessor.Display.Error");
  auto* contours = page.findChild<QComboBox*>("Postprocessor.Display.Contours");
  auto* bands = page.findChild<QSpinBox*>("Postprocessor.Display.Bands");
  auto* legend = page.findChild<QCheckBox*>("Postprocessor.Display.Legend");
  check(palette && range && low && high && minimum && maximum && minValue && maxValue && reverse && apply && message && contours && bands && legend,
    "Display property controls missing");
  if (!palette || !range || !low || !high || !minimum || !maximum || !minValue || !maxValue || !reverse || !apply || !message || !contours || !bands || !legend)
  {
    return 1;
  }
  check(sent.empty() && !apply->isEnabled(),"Refreshing properties submitted commands or enabled Apply");
  range->setCurrentIndex(range->findData(static_cast<int>(ColorRangeMode::Fixed)));
  low->setText("1e2");
  high->setText("3e2");
  minimum->setChecked(true);
  maximum->setChecked(true);
  minValue->setText("100");
  maxValue->setText("250");
  check(sent.empty() && apply->isEnabled(),"Editing controls applied state before Apply");
  check(page.apply(error),"Cannot apply scientific fixed range and paired limits");
  auto settings = application->coloring("Mesh",error);
  check(settings && settings->fixed_range == std::array<double,2>{100,300} && settings->minimum.enabled && settings->maximum.enabled &&
    settings->minimum.value == 100 && settings->maximum.value == 250,"Property commands did not publish the complete draft");
  check(!apply->isEnabled(),"Successful Apply retained dirty state");
  contours->setCurrentIndex(contours->findData(true));
  bands->setValue(8);
  legend->setChecked(false);
  check(page.apply(error),"Cannot apply banded contours and legend visibility");
  settings = application->coloring("Mesh",error);
  check(settings && settings->bands == std::optional<unsigned int>(8) && !settings->legend_visible,
    "Bands/legend controls did not publish their settings");

  range->setCurrentIndex(range->findData(static_cast<int>(ColorRangeMode::AllFrames)));
  check(page.apply(error),"Cannot apply an all-frame legend range");
  settings = application->coloring("Mesh",error);
  check(settings && settings->range_mode == ColorRangeMode::AllFrames,"All frames selector did not publish its mode");

  palette->setCurrentIndex(palette->findData(static_cast<int>(ColorPalette::Grayscale)));
  check(application->set_frame("Mesh",2,error),"Cannot update frame during a draft");
  page.refresh();
  check(palette->currentData().toInt() == static_cast<int>(ColorPalette::Grayscale) && apply->isEnabled(),"Frame refresh overwrote pending edits");
  settings = application->coloring("Mesh",error);
  if (settings)
  {
    settings->reversed = true;
    check(application->set_coloring("Mesh",*settings,error),"Cannot apply external property change");
  }
  page.refresh();
  check(reverse->isChecked() && palette->currentData().toInt() == static_cast<int>(ColorPalette::Viridis) && !apply->isEnabled(),
    "External coloring command failed to replace the draft with committed state");

  minValue->setText("300");
  maxValue->setText("200");
  auto before = sent.size();
  check(!page.apply(error) && sent.size() == before,"Crossed draft limits emitted commands");
  minValue->setText("300");
  maxValue->setText("400");
  check(page.apply(error),"Moving both active limits upward generated an invalid intermediate state");
  maximum->setChecked(false);
  check(page.apply(error),"Cannot disable a limit from Properties");
  settings = application->coloring("Mesh",error);
  check(settings && !settings->maximum.enabled && settings->maximum.value == 400,"Disabling a limit lost its retained threshold");

  palette->setCurrentIndex(palette->findData(static_cast<int>(ColorPalette::Grayscale)));
  reverse->setChecked(false);
  failAt = sent.size()+2;
  check(!page.apply(error),"Partial command failure was reported as success");
  settings = application->coloring("Mesh",error);
  check(settings && settings->palette == ColorPalette::Grayscale && settings->reversed && reverse->isChecked() && !message->text().isEmpty(),
    "Failure recovery did not show the committed partial state");
  failAt = 0;
  rejectWithoutFailure = true;
  reverse->setChecked(false);
  check(!page.apply(error) && reverse->isChecked(),"Accepted-but-rejected command was mistaken for a published change");
  rejectWithoutFailure = false;

  reverse->setChecked(false);
  before = sent.size();
  check(application->open_job("fake",1,std::string("Other"),error),"Cannot change property source");
  check(!page.apply(error) && sent.size() == before && palette->currentData().toInt() == static_cast<int>(ColorPalette::Viridis),
    "A stale draft was applied to the newly selected source");
  check(application->reset(error),"Cannot reset property fixture");
  page.refresh();
  check(!page.isEnabled() && !apply->isEnabled(),"Reset left stale property controls active");
  return failures == 0 ? 0 : 1;
}
