#include "Postprocessor/Commands/options/PostprocessorOptionCommands.hpp"
#include "PostprocessorOptionsPanel.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"

#include <QFileDialog>
#include <QColorDialog>
#include <QGroupBox>
#include <QVariant>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <utility>

PostprocessorOptionsPanel::PostprocessorOptionsPanel(std::weak_ptr<PostprocessorApplicationInterface> application,
  std::function<bool(const std::string&, std::string&)> submit, QWidget* parent)
  : QWidget(parent), application(std::move(application)), submitCommand(std::move(submit))
{
  auto* layout = new QVBoxLayout(this);
  auto* form = new QFormLayout;
  auto* pathRow = new QHBoxLayout;
  iconsPath = new QLineEdit(this);
  iconsPath->setToolTip(tr("Relative to the component directory, or an absolute path. Changes require a restart."));
  auto* browse = new QPushButton(tr("Browse…"), this);
  pathRow->addWidget(iconsPath);
  pathRow->addWidget(browse);
  form->addRow(tr("Icons directory"), pathRow);
  layout->addLayout(form);
  restartNotice = new QLabel(this);
  restartNotice->setWordWrap(true);
  layout->addWidget(restartNotice);
  auto* displayGroup = new QGroupBox(tr("Display"), this);
  auto* displayForm = new QFormLayout(displayGroup);
  backgroundTop = new QPushButton(displayGroup);
  backgroundBottom = new QPushButton(displayGroup);
  displayForm->addRow(tr("Screen Top"), backgroundTop);
  displayForm->addRow(tr("Screen Bottom"), backgroundBottom);
  triad = new QCheckBox(tr("Display Triad"), displayGroup);
  displayForm->addRow(triad);
  resultPalette = new QComboBox(displayGroup);
  resultPalette->setObjectName("Postprocessor.Options.DefaultResultPalette");
  resultPalette->addItem(tr("Viridis"),QStringLiteral("viridis"));
  resultPalette->addItem(tr("Blue - White - Red"),QStringLiteral("bluewhitered"));
  resultPalette->addItem(tr("Turbo"),QStringLiteral("turbo"));
  resultPalette->addItem(tr("Classic rainbow"),QStringLiteral("rainbow"));
  resultPalette->addItem(tr("Grayscale"),QStringLiteral("grayscale"));
  resultPalette->setToolTip(tr("Default palette for newly opened results and new views. Existing result display settings are preserved."));
  displayForm->addRow(tr("Default result palette"),resultPalette);
  layout->addWidget(displayGroup);
  for (auto* button : {backgroundTop, backgroundBottom})
  {
    connect(button, &QPushButton::clicked, this, [this, button]()
    {
      choose_color(button);
    });
  }
  journalCamera = new QCheckBox(tr("Record camera navigation"), this);
  journalCamera->setToolTip(tr("Record completed GUI rotate, pan, zoom and fit actions. Typed commands follow Cubit's journal settings. Takes effect immediately."));
  layout->addWidget(new QLabel(tr("Journal"), this));
  layout->addWidget(journalCamera);
  journalNavigation = new QCheckBox(tr("Record result navigation"),this);
  journalNavigation->setObjectName("Postprocessor.Journal.Navigation");
  journalNavigation->setToolTip(tr("Journal completed GUI frame selections. Slider previews are never journaled. Typed commands follow Cubit's journal settings."));
  layout->addWidget(journalNavigation);
  auto* journalNote = new QLabel(tr("Source opening/closing and result display settings remain journaled."), this);
  journalNote->setWordWrap(true);
  layout->addWidget(journalNote);
  auto* reset = new QPushButton(tr("Reset to defaults"),this);
  reset->setObjectName("Postprocessor.Options.ResetDefaults");
  reset->setToolTip(tr("Restore this page's Core defaults as unsaved edits. Press Save to apply."));
  layout->addWidget(reset);
  connect(reset,&QPushButton::clicked,this,[this]()
  {
    const auto active = this->application.lock();
    if (!active)
    {
      return;
    }
    UserOptionValues defaults;
    for (const auto& option : active->user_option_definitions())
    {
      defaults.emplace(option.key,option.defaultValue);
    }
    iconsPath->setText(QString::fromStdString(std::get<std::string>(defaults.at("paths/icons"))));
    journalCamera->setChecked(std::get<bool>(defaults.at("journal/camera")));
    journalNavigation->setChecked(std::get<bool>(defaults.at("journal/navigation")));
    triad->setChecked(std::get<bool>(defaults.at("display/triad")));
    resultPalette->setCurrentIndex(resultPalette->findData(QString::fromStdString(std::get<std::string>(defaults.at("color/palette")))));
    for (const auto& pair : {std::make_pair(backgroundTop,"background/top"),std::make_pair(backgroundBottom,"background/bottom")})
    {
      const auto text = std::get<std::string>(defaults.at(pair.second));
      std::array<double,3> rgb;
      if (Postprocessor::parse_background_color(text,rgb))
      {
        const auto color = QColor::fromRgbF(rgb[0],rgb[1],rgb[2]);
        pair.first->setProperty("rgb",QString::fromStdString(text));
        pair.first->setText(color.name());
        pair.first->setStyleSheet(QStringLiteral("background-color: %1; color: %2;").arg(color.name(),color.lightnessF()>0.5 ? "black":"white"));
      }
    }
  });
  layout->addStretch();
  connect(browse, &QPushButton::clicked, this, [this]()
  {
    const auto path = QFileDialog::getExistingDirectory(this, tr("Select Postprocessor icons directory"), iconsPath->text());
    if (!path.isEmpty())
    {
      iconsPath->setText(path);
    }
  });
  refresh_settings();
}

void PostprocessorOptionsPanel::refresh_settings()
{
  const auto active = application.lock();
  setEnabled(active != nullptr);
  if (!active)
  {
    return;
  }
  iconsPath->setText(QString::fromUtf8(active->user_options().iconsPath.c_str()));
  const auto& options = active->user_options();
  journalCamera->setChecked(options.journalCamera);
  journalNavigation->setChecked(options.journalNavigation);
  triad->setChecked(options.triadVisible);
  resultPalette->setCurrentIndex(resultPalette->findData(QString::fromLatin1(Postprocessor::color_palette_name(options.resultPalette))));
  const std::pair<QPushButton*, std::array<double, 3>> colors[] =
  {
    {backgroundTop, options.backgroundTop}, {backgroundBottom, options.backgroundBottom}
  };
  for (const auto& entry : colors)
  {
    const auto color = QColor::fromRgbF(entry.second[0], entry.second[1], entry.second[2]);
    entry.first->setProperty("rgb", QString::fromStdString(Postprocessor::format_background_color(entry.second)));
    entry.first->setText(color.name());
    entry.first->setStyleSheet(QStringLiteral("background-color: %1; color: %2;").arg(color.name(), color.lightnessF() > 0.5 ? "black" : "white"));
  }
  restartNotice->setText(active->user_options_restart_required()
    ? tr("Saved icons directory changes will take effect after restarting Cubit.")
    : tr("Icons directory changes require a restart."));
  baseline = option_values();
}

QJsonObject PostprocessorOptionsPanel::option_values() const
{
  QJsonObject values;
  values["paths/icons"] = iconsPath->text();
  values["journal/camera"] = journalCamera->isChecked();
  values["journal/navigation"] = journalNavigation->isChecked();
  values["background/top"] = backgroundTop->property("rgb").toString();
  values["background/bottom"] = backgroundBottom->property("rgb").toString();
  values["display/triad"] = triad->isChecked();
  values["color/palette"] = resultPalette->currentData().toString();
  return values;
}

bool PostprocessorOptionsPanel::save_settings(std::string& error)
{
  error.clear();
  const auto active = application.lock();
  if (!active || !submitCommand)
  {
    error = "Postprocessor user options are unavailable.";
    return false;
  }
  const auto edited = option_values();
  std::vector<std::pair<std::string, std::string>> commands;
  if (!PostprocessorOptionCommands::edited_commands(active->user_option_definitions(), edited, baseline, commands, error))
  {
    return false;
  }
  for (const auto& command : commands)
  {
    if (!submitCommand(command.second, error))
    {
      return false;
    }
    const auto key = QString::fromStdString(command.first);
    baseline[key] = edited.value(key);
  }
  refresh_settings();
  return true;
}

void PostprocessorOptionsPanel::choose_color(QPushButton* button)
{
  const auto color = QColorDialog::getColor(QColor(button->text()), this, tr("Background Color"));
  if (!color.isValid())
  {
    return;
  }
  button->setProperty("rgb", QString::fromStdString(Postprocessor::format_background_color({color.redF(), color.greenF(), color.blueF()})));
  button->setText(color.name());
  button->setStyleSheet(QStringLiteral("background-color: %1; color: %2;").arg(color.name(), color.lightnessF() > 0.5 ? "black" : "white"));
}
