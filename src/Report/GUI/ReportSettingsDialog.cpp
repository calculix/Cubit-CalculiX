#include "ReportSettingsDialog.hpp"
#include "Report/Interfaces/ReportApplicationInterface.hpp"
#include "Framework/Reporting/ReportConfiguration.hpp"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QCheckBox>
#include <QFileDialog>
#include <QPixmap>
#include "ReportSettingsCommandText.hpp"
#include <QScrollArea>
#include <QHBoxLayout>
#include <utility>

ReportSettingsDialog::ReportSettingsDialog(QWidget* parent, const ReportConfiguration& configuration, std::weak_ptr<ReportApplicationInterface> active,
  std::function<bool(const std::string&, std::string&)> callback) : QDialog(parent), application(std::move(active)), submit(std::move(callback))
{
  if (const auto active = application.lock())
  {
    source = active->configuration_request();
  }
  setWindowTitle(tr("Report settings"));
  resize(720, 640);
  stagedLogo = configuration.company_logo;
  auto* outer = new QVBoxLayout(this);
  auto* tabs = new QTabWidget(this);
  ReportConfiguration displayed = configuration;
  ReportTemplate definition;
  if (const auto active = application.lock())
  {
    std::string error;
    if (active->describe(definition, error))
    {
      for (const auto& entry : definition.defaults.values)
      {
        displayed.values.emplace(entry.first, entry.second);
      }
    }
  }
  initialConfiguration = displayed;
  const auto text = [&](const std::string& key)
  {
    const auto found = displayed.values.find(key);
    return found == displayed.values.end() ? QString() : QString::fromStdString(found->second);
  };
  auto* general = new QWidget(tabs);
  auto* fields = new QFormLayout(general);
  title = new QLineEdit(text("title"), general);
  title->setObjectName("ReportSetting_title");
  author = new QLineEdit(text("author"), general);
  project = new QLineEdit(text("project"), general);
  revision = new QLineEdit(text("revision"), general);
  fields->addRow(tr("Title"), title);
  fields->addRow(tr("Author"), author);
  fields->addRow(tr("Project"), project);
  fields->addRow(tr("Revision"), revision);
  tabs->addTab(general, tr("General"));
  auto* scope = new QWidget(tabs);
  auto* scopeFields = new QFormLayout(scope);
  steps = new QLineEdit(text("steps"), scope);
  steps->setPlaceholderText(tr("all or 1,2,3"));
  scopeFields->addRow(tr("Steps"), steps);
  scopeFields->addRow(new QLabel(tr("Enter all, or comma-separated step numbers. Content inclusion is controlled in the report content tree."), scope));
  tabs->addTab(scope, tr("Steps"));
  auto* page = new QWidget(tabs);
  auto* pageFields = new QFormLayout(page);
  paper = new QComboBox(page);
  paper->addItems({"A4", "Letter"});
  paper->setCurrentText(text("page_size"));
  precision = new QSpinBox(page);
  precision->setRange(2, 12);
  precision->setValue(text("precision").toInt());
  pageFields->addRow(tr("Paper size"), paper);
  pageFields->addRow(tr("Significant digits"), precision);
  tabs->addTab(page, tr("Page layout"));
  // Providers describe available values and limits; GUI supplies no solver defaults.
  auto* capture = new QWidget(tabs);
  auto* captureFields = new QFormLayout(capture);
  for (const auto& option : definition.options)
  {
    if (option.kind == "choice")
    {
      auto* combo = new QComboBox(capture);
      combo->setObjectName(QString::fromStdString("ReportSetting_" + option.key));
      for (const auto& choice : option.choices)
      {
        combo->addItem(QString::fromStdString(choice));
      }
      combo->setCurrentText(text(option.key));
      captureChoices.emplace(option.key, combo);
      captureFields->addRow(QString::fromStdString(option.label), combo);
    }
    else if (option.kind == "integer")
    {
      auto* spin = new QSpinBox(capture);
      spin->setObjectName(QString::fromStdString("ReportSetting_" + option.key));
      spin->setRange(option.minimum, option.maximum);
      spin->setValue(text(option.key).toInt());
      captureIntegers.emplace(option.key, spin);
      captureFields->addRow(QString::fromStdString(option.label), spin);
    }
  }
  if (!captureChoices.empty() || !captureIntegers.empty())
  {
    auto* hint = new QLabel(tr("Image inclusion and label visibility are controlled in the report content tree. These settings are saved with the report profile."), capture);
    hint->setWordWrap(true);
    captureFields->addRow(hint);
    tabs->addTab(capture, tr("Images"));
  }
  else
  {
    delete capture;
  }
  auto* brandingScroll = new QScrollArea(tabs);
  brandingScroll->setWidgetResizable(true);
  auto* branding = new QWidget(brandingScroll);
  auto* brandingFields = new QFormLayout(branding);
  for (const auto& entry : std::vector<std::pair<std::string, QString>>
    {{"company_name", tr("Company name")}, {"company_details", tr("Company details")},
     {"header_left", tr("Header left")}, {"header_center", tr("Header centre")}, {"header_right", tr("Header right")},
     {"footer_left", tr("Footer left")}, {"footer_center", tr("Footer centre")}, {"footer_right", tr("Footer right")}})
  {
    auto* field = new QLineEdit(text(entry.first), branding);
    field->setMaxLength(entry.first.find("company_") == 0 ? 256 : 160);
    decorationFields.emplace(entry.first, field);
    brandingFields->addRow(entry.second, field);
  }
  auto* tokens = new QLabel(tr("Placeholders: {company_name}, {company_details}, {title}, {project}, {author}, {revision}, {page}, {pages}. Leave a position empty to omit its text."), branding);
  tokens->setWordWrap(true);
  brandingFields->addRow(tokens);
  for (const auto& entry : std::vector<std::pair<std::string, QString>>
    {{"header_line", tr("Header line")}, {"footer_line", tr("Footer line")}, {"decoration_cover", tr("Show header/footer on cover")}})
  {
    auto* field = new QCheckBox(branding);
    field->setChecked(text(entry.first) == "true");
    decorationFlags.emplace(entry.first, field);
    brandingFields->addRow(entry.second, field);
  }
  auto* logoRow = new QWidget(branding);
  auto* logoButtons = new QHBoxLayout(logoRow);
  logoButtons->setContentsMargins(0, 0, 0, 0);
  logoThumbnail = new QLabel(logoRow);
  logoThumbnail->setMinimumSize(120, 48);
  const auto updateLogo = [this]()
  {
    logoThumbnail->clear();
    if (stagedLogo)
    {
      QPixmap image;
      image.loadFromData(QByteArray::fromBase64(QByteArray::fromStdString(stagedLogo->base64_data)));
      logoThumbnail->setPixmap(image.scaled(120, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    else
    {
      logoThumbnail->setText(tr("No logo selected"));
    }
  };
  updateLogo();
  auto* chooseLogo = new QPushButton(tr("Choose file..."), logoRow);
  auto* removeLogo = new QPushButton(tr("Remove"), logoRow);
  logoButtons->addWidget(logoThumbnail);
  logoButtons->addWidget(chooseLogo);
  logoButtons->addWidget(removeLogo);
  brandingFields->addRow(tr("Company logo"), logoRow);
  connect(chooseLogo, &QPushButton::clicked, this, [this, updateLogo]()
  {
    const auto filename = QFileDialog::getOpenFileName(this, tr("Company logo"), QString(), tr("Company logos (*.svg *.png *.jpg *.jpeg)"));
    if (filename.isEmpty())
    {
      return;
    }
    const auto active = application.lock();
    ReportImage image;
    std::string error;
    if (!active || !active->load_logo(filename.toStdString(), image, error))
    {
      message->setText(QString::fromStdString(error.empty() ? "The Report application is unavailable." : error));
      return;
    }
    stagedLogo = std::move(image);
    logoFilename = filename;
    logoChanged = true;
    message->clear();
    updateLogo();
  });
  connect(removeLogo, &QPushButton::clicked, this, [this, updateLogo]()
  {
    stagedLogo.reset();
    logoFilename.clear();
    logoChanged = true;
    updateLogo();
  });
  logoPlacement = new QComboBox(branding);
  for (const auto& entry : std::vector<std::pair<QString, QString>>
    {{tr("Header left"), "header_left"}, {tr("Header centre"), "header_center"}, {tr("Header right"), "header_right"},
     {tr("Footer left"), "footer_left"}, {tr("Footer centre"), "footer_center"}, {tr("Footer right"), "footer_right"}})
  {
    logoPlacement->addItem(entry.first, entry.second);
  }
  logoPlacement->setCurrentIndex(logoPlacement->findData(text("logo_placement")));
  logoWidth = new QSpinBox(branding);
  logoWidth->setRange(5, 60);
  logoWidth->setValue(text("logo_width_mm").toInt());
  logoWidth->setSuffix(tr(" mm"));
  brandingFields->addRow(tr("Logo placement"), logoPlacement);
  brandingFields->addRow(tr("Maximum logo width"), logoWidth);
  auto* proportions = new QLabel(tr("Proportions are preserved. Logos fit within their column and a maximum height of 12.7 mm. Imported files are embedded as PNG."), branding);
  proportions->setWordWrap(true);
  brandingFields->addRow(proportions);
  brandingScroll->setWidget(branding);
  tabs->addTab(brandingScroll, tr("Header & footer"));
  outer->addWidget(tabs);
  message = new QLabel(this);
  message->setWordWrap(true);
  outer->addWidget(message);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Cancel, this);
  outer->addWidget(buttons);
  connect(buttons->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, [this]()
  {
    apply();
  });
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

ReportSettingsDialog::~ReportSettingsDialog() = default;

void ReportSettingsDialog::apply()
{
  const auto active = application.lock();
  if (!active)
  {
    return;
  }
  const auto& current = active->configuration_request();
  if (current.provider != source.provider || current.kind != source.kind || current.job_id != source.job_id || current.model_id != source.model_id)
  {
    message->setText(tr("The selected report source changed. Reopen settings for that source."));
    return;
  }
  ReportConfiguration patch;
  patch.values = {{"title", title->text().toStdString()}, {"author", author->text().toStdString()},
    {"project", project->text().toStdString()}, {"revision", revision->text().toStdString()},
    {"steps", steps->text().trimmed().toStdString()}, {"precision", std::to_string(precision->value())},
    {"page_size", paper->currentText().toStdString()}};
  for (const auto& entry : decorationFields)
  {
    patch.values[entry.first] = entry.second->text().toStdString();
  }
  for (const auto& entry : decorationFlags)
  {
    patch.values[entry.first] = entry.second->isChecked() ? "true" : "false";
  }
  patch.values["logo_placement"] = logoPlacement->currentData().toString().toStdString();
  for (const auto& entry : captureChoices)
  {
    patch.values[entry.first] = entry.second->currentText().toStdString();
  }
  for (const auto& entry : captureIntegers)
  {
    patch.values[entry.first] = std::to_string(entry.second->value());
  }
  patch.values["logo_width_mm"] = std::to_string(logoWidth->value());
  patch.replace_company_logo = logoChanged;
  patch.company_logo = stagedLogo;
  for (auto entry = patch.values.begin(); entry != patch.values.end();)
  {
    const auto initial = initialConfiguration.values.find(entry->first);
    if (initial != initialConfiguration.values.end() && initial->second == entry->second)
    {
      entry = patch.values.erase(entry);
    }
    else
    {
      ++entry;
    }
  }
  if (patch.values.empty() && !patch.replace_company_logo)
  {
    accept();
    return;
  }
  std::string validation;
  if (!active->validate_configuration_patch(patch, validation))
  {
    message->setText(QString::fromStdString(validation));
    return;
  }
  std::string command, error;
  ReportTemplate definition;
  if (!active->describe(definition, error) || !ReportSettingsCommandText::build(patch, logoFilename, command, error, &definition))
  {
    message->setText(QString::fromStdString(error));
    return;
  }
  if (!submit(command, error))
  {
    message->setText(QString::fromStdString(error));
    return;
  }
  // Framework submission confirms dispatch, not command success. Verify the
  // committed configuration before closing the dialog.
  const auto& committed = active->configuration_request().overrides.values;
  for (const auto& entry : patch.values)
  {
    const auto value = committed.find(entry.first);
    if (value == committed.end() || value->second != entry.second)
    {
      message->setText(tr("The settings command did not commit. Check the command output and retry."));
      return;
    }
  }
  const auto& logo = active->configuration_request().overrides.company_logo;
  if (logoChanged && (logo.has_value() != stagedLogo.has_value() ||
      (logo && stagedLogo && logo->base64_data != stagedLogo->base64_data)))
  {
    message->setText(tr("The logo command did not commit the selected image. Check the command output and retry."));
    return;
  }
  accept();
}
