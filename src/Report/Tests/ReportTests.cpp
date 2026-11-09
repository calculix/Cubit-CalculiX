#include "ReportTestProviders.hpp"
#include "Report/Core/ReportCore.hpp"
#include "Report/Core/ReportProjectPersistence.hpp"
#include "Report/Document/ReportDocumentLayout.hpp"
#include "Report/Document/ReportPdfExporter.hpp"
#include "Report/Document/ReportOdtExporter.hpp"
#include "Framework/Storage/ReportProfileStore.hpp"
#include "Framework/Storage/UserOptionsStore.hpp"
#include "Framework/Storage/HDF5Tool.hpp"
#include "CalculiX/Core/reporting/CalculiXReportInputReader.hpp"
#include <QApplication>
#include <QScrollBar>
#include "Report/GUI/ReportPreviewWidget.hpp"
#include "Report/GUI/ReportWorkspace.hpp"
#include "Report/Interfaces/ReportApplicationInterface.hpp"
#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QTreeWidget>
#include <QSlider>
#include <QImage>
#include <QPainter>
#include <QXmlStreamReader>
#include "Report/Core/ReportLogo.hpp"
#include "Report/Document/ReportPageDecoration.hpp"
#include <QTemporaryDir>
#include <QLockFile>
#include <QFile>
#include <iostream>

namespace
{
int failures = 0;

void check(bool condition, const char* message)
{
  if (!condition)
  {
    std::cerr << message << '\n';
    ++failures;
  }
}

void fixture(const QString& filename, const QByteArray& text)
{
  QFile file(filename);
  check(file.open(QIODevice::WriteOnly) && file.write(text) == text.size(), "Cannot create fixture");
}
}

int main(int argc, char** argv)
{
  QApplication application(argc, argv);
  QTemporaryDir directory;
  check(directory.isValid(), "Temporary directory unavailable");
  std::string error;
  FrameworkPaths paths;
  paths.optionsFilename = directory.filePath("options.h5").toStdString();
  UserOptionsStore options(paths);
  const std::vector<UserOption> schema = {{"sentinel", UserOptionKind::Integer, 7}};
  UserOptionValues settings;
  check(options.load("test", schema, settings, error), "Initialize options schema");
  ReportProfileStore profiles(paths.optionsFilename);
  ReportTestProviders providers;
  providers.profiles = &profiles;
  ReportCore core(providers);
  check(core.generate("test", 1, "", error), "Generate initial document");
  check(core.note("test.content", "Retain this note", error), "Attach note");
  const auto initial = core.document();
  providers.fail_collection = true;
  check(!core.update(error) && core.document() == initial, "Failed collection must preserve committed document");
  providers.fail_collection = false;
  providers.changed_source = true;
  check(!core.update(error) && core.document() == initial, "Changed sources must not be silently accepted");
  providers.changed_source = false;
  check(!core.configure("unknown", "true", error), "Reject unknown configuration");
  check(!core.configure("precision", "100", error), "Reject invalid precision");
  check(core.configure("author", "Author A", error), "Configure author");
  std::vector<ReportProfile> saved;
  check(core.profile("save", "Standard / display name", saved, error), "Save profile with path-like display name");
  check(saved.size() == 1 && saved.front().revision == 1, "Initial profile revision");
  check(core.profile("save", "Standard / display name", saved, error) && saved.front().revision == 2, "Stable profile revision increment");
  check(core.profile("default", "Standard / display name", saved, error), "Choose default profile");
  check(core.update(error), "Update unchanged sources");
  check(core.document()->notes.at("test.content") == "Retain this note", "Preserve notes during update");
  const auto firstId = core.document()->id;
  check(core.configure("author", "Pending draft", error), "Create pending draft");
  check(core.generate("test", 2, "", error), "Generate second document using default profile");
  check(core.document()->configuration.values.at("author") == "Author A", "Apply default profile to future report");
  check(core.select(firstId, error) && core.session().configuration_changed &&
    core.session().request.overrides.values.at("author") == "Pending draft", "Preserve per-report configuration drafts");
  check(options.load("test", schema, settings, error) && std::get<int>(settings.at("sentinel")) == 7, "Profile writes preserve unrelated preferences");
  std::vector<ReportProfile> listed;
  check(profiles.list("test", listed, error) && listed.size() == 1, "Options writes preserve report profiles");
  QLockFile lock(QString::fromStdString(paths.optionsFilename) + ".lock");
  check(lock.tryLock(0), "Acquire options lock");
  check(!profiles.set_default_profile("test", "Standard / display name", error), "Profile writes honor shared options lock");
  lock.unlock();

  const auto project = directory.filePath("project.cub").toStdString();
  {
    HDF5Tool file(project);
    check(file.ensureGroup("native-sentinel") && file.write_dataset_int_rank_1("value", "native-sentinel", {42}), "Create unrelated project data");
    check(ReportProjectPersistence().write(file, core.session(), error), "Persist report session");
    check(file.close(), "Close project fixture");
  }
  ReportSession restored;
  {
    HDF5Tool file(project, false);
    check(ReportProjectPersistence().read(file, restored, error), "Restore report session");
    std::vector<int> sentinel;
    check(file.read_dataset_int_rank_1("value", "native-sentinel", sentinel) && sentinel == std::vector<int>{42}, "Report writes preserve other project data");
    check(file.close(), "Close restored project");
  }
  check(restored.documents.size() == 2 && restored.configuration_changed, "Restore documents and pending configuration");
  auto json = restored.to_json();
  json["active_id"] = "missing";
  const auto previousId = restored.active_id;
  check(!ReportSession::from_json(json, restored, error) && restored.active_id == previousId, "Invalid restore leaves session unchanged");
  auto numeric = core.document()->to_json();
  ReportDocument roundtrip;
  check(ReportDocument::from_json(numeric, roundtrip, error), "Round-trip document");
  check(roundtrip.sections.front().tables.front().rows.front()[1].isDouble(), "Preserve numeric cell type");
  roundtrip.sections.front().tables.front().rows.front().pop_back();
  check(!roundtrip.validate(error), "Reject malformed table dimensions");

  ReportDocumentLayout layout;
  check(layout.prepare(*core.document(), error) && layout.page_count() > 0, "Prepare paginated layout");
  check(ReportPdfExporter().write(layout, directory.filePath("report.pdf").toStdString(), error), "Export PDF");
  check(ReportOdtExporter().write(layout, directory.filePath("report.odt").toStdString(), error), "Export editable ODT");
  QFile odt(directory.filePath("report.odt"));
  check(odt.open(QIODevice::ReadOnly), "Open exported ODT");
  const auto package = odt.readAll();
  check(package.startsWith("PK\003\004") && package.mid(30, 8) == "mimetype" && package.contains("office:master-styles") && package.contains("Literal &lt;tag&gt; &amp; text"), "ODT package, native page styles and text escaping");

  fixture(directory.filePath("mesh.inp"), "*NODE\n1,0,0,0\n");
  fixture(directory.filePath("model.inp"), "*INCLUDE,INPUT=mesh.inp\n*STEP\n*STATIC\n1,1\n*END STEP\n");
  ReportSource source;
  source.input_files = {directory.filePath("model.inp").toStdString()};
  CalculiXReportInputReader reader;
  check(reader.read(source, error) && reader.resolved_source().fingerprints.size() == 2, "Resolve and fingerprint INCLUDE files");
  const auto previousRecords = reader.records().size();
  fixture(directory.filePath("mesh.inp"), "*INCLUDE,INPUT=model.inp\n");
  check(!reader.read(source, error) && reader.records().size() == previousRecords, "Reject cyclic INCLUDE without replacing reader output");
  const auto beforeReset = core.document();
  check(core.profile("reset", "", saved, error) && core.document() == beforeReset,
    "Loading workspace defaults changes configuration without regenerating the saved report");
  ReportRequest modelSource;
  modelSource.provider = "test";
  modelSource.kind = "model";
  modelSource.model_id = "current";
  const auto beforeSourceChoice = core.document();
  check(core.choose_source(modelSource, error) && core.document() == beforeSourceChoice && core.session().pending_source,
    "Source selection must not replace the visible report");
  ReportConfiguration invalidPatch;
  invalidPatch.values = {{"author", "Must not commit"}, {"precision", "100"}};
  const auto pendingBefore = core.configuration_request().overrides.values;
  check(!core.configure_patch(invalidPatch, error) && core.configuration_request().overrides.values == pendingBefore,
    "Invalid configuration patch must commit no fields");
  check(core.generate_request(modelSource, "", error) && core.document()->source.kind == "model" && core.document()->source.job_id == 0,
    "Generate a model-only report without a fabricated job ID");
  providers.changed_source = true;
  check(core.update(error), "An explicit model update accepts changed current model content");
  providers.changed_source = false;
  ReportPreviewWidget viewer;
  viewer.resize(500, 500);
  auto viewerLayout = std::make_shared<ReportDocumentLayout>();
  check(viewerLayout->prepare(*core.document(), error), "Prepare continuous viewer layout");
  viewer.set_layout(viewerLayout);
  viewer.show();
  QApplication::processEvents();
  check(viewer.verticalScrollBar()->maximum() > 0, "Continuous preview exposes vertical scrolling");
  viewer.set_view(1, 200, "percent");
  check(viewer.horizontalScrollBar()->maximum() > 0, "Large zoom exposes horizontal scrolling");
  viewer.verticalScrollBar()->setValue(120);
  const int scrolled = viewer.verticalScrollBar()->value();
  viewer.set_layout(viewerLayout);
  viewer.set_view(1, 200, "percent");
  check(viewer.verticalScrollBar()->value() == scrolled, "Ordinary refresh preserves preview scroll position");
  viewer.set_view(2, 200, "percent", true);
  check(viewer.page() == 1, "Explicit page navigation scrolls to the requested page");
  viewer.set_view(2, 200, "page", false, true);
  check(viewer.page() == 1 && viewer.horizontalScrollBar()->maximum() == 0,
    "Fit page keeps the visible page and fits its width");
  viewer.resize(640, 420);
  QApplication::processEvents();
  check(viewer.page() == 1 && viewer.horizontalScrollBar()->maximum() == 0,
    "Fit page survives viewport resizing without changing the visible page");
  viewer.verticalScrollBar()->setValue(viewer.verticalScrollBar()->value() + 40);
  viewer.set_view(1, 200, "page", false, true);
  const auto fittedPosition = viewer.verticalScrollBar()->value();
  viewer.set_view(1, 200, "page");
  check(viewer.page() == 1 && viewer.verticalScrollBar()->value() == fittedPosition,
    "Explicit refit aligns the current page; ordinary refresh preserves the view");
  ReportTestProviders workspaceProviders;
  workspaceProviders.profiles = &profiles;
  auto workspaceApplication = std::make_shared<ReportApplicationInterface>(workspaceProviders);
  QMainWindow host;
  std::vector<std::string> commands;
  const auto recordCommand = [&commands](const std::string& command, std::string& commandError)
  {
    commands.push_back(command);
    commandError.clear();
    return true;
  };
  ReportWorkspace workspace(&host, workspaceApplication, recordCommand, recordCommand, QIcon());
  auto* sourceSelector = workspace.findChild<QComboBox*>("ReportSourceSelector");
  auto* generateButton = workspace.findChild<QPushButton*>("ReportGenerateButton");
  auto* settingsButton = workspace.findChild<QPushButton*>("ReportSettingsButton");
  auto* profileButton = workspace.findChild<QPushButton*>("ReportProfileButton");
  auto* profileSelector = workspace.findChild<QComboBox*>("ReportProfileSelector");
  auto* contentTree = host.findChild<QTreeWidget*>();
  check(sourceSelector && generateButton && settingsButton && profileButton && profileSelector && contentTree,
    "Workspace exposes source and configuration controls");
  if (sourceSelector && generateButton && settingsButton && profileButton && profileSelector && contentTree)
  {
    for (int refresh = 0; refresh < 5; ++refresh)
    {
      workspace.refresh();
      QApplication::processEvents();
    }
    check(commands.empty() && sourceSelector->currentIndex() == -1,
      "Construction and queued refreshes never select a source by command");
    check(!generateButton->isEnabled() && !settingsButton->isEnabled() && !profileButton->isEnabled() &&
      !profileSelector->isEnabled() && !contentTree->isEnabled(), "Source-dependent controls require a committed source");
    workspaceProviders.publish_sources = false;
    workspace.refresh();
    check(sourceSelector->count() == 0, "Removed sources disappear from the selector");
    workspaceProviders.publish_sources = true;
    workspace.refresh();
    QApplication::processEvents();
    check(commands.empty() && sourceSelector->currentIndex() == -1 && sourceSelector->count() > 0,
      "Newly discovered sources do not submit commands");
    const int modelIndex = sourceSelector->findData(0);
    check(modelIndex >= 0, "Find the test model source");
    if (modelIndex >= 0)
    {
      sourceSelector->setCurrentIndex(modelIndex);
      check(QMetaObject::invokeMethod(sourceSelector, "activated", Qt::DirectConnection, Q_ARG(int, modelIndex)),
        "Activate the source selector");
      QApplication::processEvents();
      workspace.refresh();
      QApplication::processEvents();
      check(commands == std::vector<std::string>{"report source test model"},
        "Explicit activation submits once without retrying an uncommitted command");
      check(sourceSelector->currentIndex() == -1 && !generateButton->isEnabled() && !settingsButton->isEnabled(),
        "An uncommitted source does not enable generation or settings");
    }
    check(workspaceApplication->choose_source(modelSource, error), "Commit the explicit source through the application interface");
    workspace.refresh();
    check(sourceSelector->currentIndex() >= 0 && generateButton->isEnabled() && settingsButton->isEnabled() &&
      profileButton->isEnabled() && profileSelector->isEnabled() && contentTree->isEnabled(),
      "Committed source enables its configuration controls");
    const auto commandCount = commands.size();
    workspaceProviders.fail_discovery = true;
    workspace.refresh();
    QApplication::processEvents();
    check(sourceSelector->count() == 0 && !generateButton->isEnabled() && !settingsButton->isEnabled() &&
      profileSelector->count() == 0 && contentTree->topLevelItemCount() == 0 && commands.size() == commandCount,
      "Discovery failure clears stale controls without dispatch");
    workspaceProviders.fail_discovery = false;
    workspace.refresh();
    check(generateButton->isEnabled(), "Discovery recovery restores the committed source");
    workspaceProviders.publish_sources = false;
    workspace.refresh();
    check(sourceSelector->count() == 0 && !generateButton->isEnabled(), "Removal invalidates a committed source in the GUI");
    workspaceApplication->reset();
    workspaceProviders.publish_sources = true;
    workspace.refresh();
    QApplication::processEvents();
    check(sourceSelector->currentIndex() == -1 && !settingsButton->isEnabled() && commands.size() == commandCount,
      "Project reset leaves source selection explicit without dispatch");
    workspaceApplication->set_gui_available(true);
    check(workspaceApplication->generate_request(modelSource, "", error), "Generate a report for slider tests");
    workspace.refresh();
    QApplication::processEvents();
    auto* slider = workspace.findChild<QSlider*>("ReportWorkspacePageSlider");
    auto* windowSlider = workspace.findChild<QSlider*>("ReportPreviewWindowPageSlider");
    check(slider && windowSlider && slider->maximum() > 1 && windowSlider->maximum() == slider->maximum(),
      "Both preview sliders expose the complete page range");
    if (slider && windowSlider && slider->maximum() > 1)
    {
      const auto beforeSlider = commands.size();
      slider->setValue(2);
      QApplication::processEvents();
      check(commands.size() == beforeSlider + 1 && commands.back() == "report preview page 2",
        "Embedded slider submits one navigation command without refresh feedback");
      windowSlider->setValue(2);
      QApplication::processEvents();
      check(commands.size() == beforeSlider + 2 && commands.back() == "report preview page 2",
        "Preview window slider uses silent navigation without refresh feedback");
      const auto fitBefore = workspaceApplication->preview_fit_revision();
      check(workspaceApplication->request_preview(error, -1, -1, "page") &&
        workspaceApplication->preview_fit_revision() == fitBefore + 1, "Fit requests have an explicit refit revision");
      workspace.refresh();
      QApplication::processEvents();
      check(commands.size() == beforeSlider + 2, "Fit and slider synchronization do not submit additional commands");
    }
  }
  ReportTestProviders brandingProviders;
  brandingProviders.profiles = &profiles;
  ReportCore brandingCore(brandingProviders);
  check(brandingCore.generate("test", 1, "", error) && !brandingCore.document()->configuration.company_logo,
    "Factory company template leaves the logo unset");
  const auto logoPath = directory.filePath("company.svg");
  fixture(logoPath, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"120\" height=\"40\"><rect width=\"120\" height=\"40\" fill=\"#18334c\"/></svg>");
  ReportImage importedLogo;
  check(ReportLogo::load(logoPath.toStdString(), importedLogo, error) && ReportLogo::validate(importedLogo, error),
    "Import a self-contained SVG as a bounded PNG snapshot");
  check(QFile::remove(logoPath), "Remove the temporary original logo file");
  ReportConfiguration branding;
  branding.values = {{"company_name", "Example & Co"}, {"company_details", "Address | Telephone | Email"},
    {"header_left", "{company_name}"}, {"header_right", "{project}"}, {"footer_right", "Page {page} of {pages}"},
    {"logo_placement", "header_right"}, {"logo_width_mm", "30"}, {"decoration_cover", "false"}};
  branding.company_logo = importedLogo;
  branding.replace_company_logo = true;
  check(brandingCore.configure_patch(branding, error), "Apply company text and embedded logo atomically");
  check(brandingCore.configure("author", "Designer", error) && brandingCore.configuration_request().overrides.company_logo,
    "Text-only patches preserve the logo");
  ReportConfiguration badBranding;
  badBranding.values = {{"company_name", "Must not commit"}, {"header_left", "{unknown}"}};
  const auto brandBefore = brandingCore.configuration_request().overrides.to_json();
  check(!brandingCore.configure_patch(badBranding, error) && brandingCore.configuration_request().overrides.to_json() == brandBefore,
    "Unknown placeholders reject the entire settings patch");
  badBranding = branding;
  badBranding.company_logo->base64_data = "invalid image";
  check(!brandingCore.configure_patch(badBranding, error) && brandingCore.configuration_request().overrides.to_json() == brandBefore,
    "Invalid logo bytes reject the entire settings patch");
  check(brandingCore.profile("save", "Company", saved, error), "Save company decoration and logo in a profile");
  ReportProfile companyProfile;
  check(profiles.load("test", "Company", companyProfile, error) && companyProfile.configuration.company_logo &&
    companyProfile.configuration.company_logo->base64_data == importedLogo.base64_data,
    "HDF5 profile retains logo after the original file is removed");
  check(brandingCore.profile("load", "Company", saved, error) && brandingCore.update(error),
    "Reload company settings and update the report");
  const auto brandedDocument = brandingCore.document();
  check(ReportPageDecoration::expand("{company_name} | {page}/{pages}", *brandedDocument, 2, 9) == "Example & Co | 2/9",
    "Resolve company and page placeholders without changing literal text");
  const auto brandedProject = directory.filePath("branded.cub").toStdString();
  {
    HDF5Tool file(brandedProject);
    check(ReportProjectPersistence().write(file, brandingCore.session(), error) && file.close(), "Persist a report containing an embedded logo");
  }
  ReportSession brandedSession;
  {
    HDF5Tool file(brandedProject, false);
    check(ReportProjectPersistence().read(file, brandedSession, error) && file.close(), "Restore the report containing an embedded logo");
  }
  check(brandedSession.documents.count(brandedDocument->id) != 0 &&
    brandedSession.documents.at(brandedDocument->id)->configuration.company_logo &&
    brandedSession.documents.at(brandedDocument->id)->configuration.company_logo->base64_data == importedLogo.base64_data,
    "Project round-trip preserves self-contained company branding");
  ReportDocumentLayout brandedLayout;
  check(brandedLayout.prepare(*brandedDocument, error), "Prepare branded report layout");
  QImage rendered(QSize(596, 842), QImage::Format_ARGB32_Premultiplied);
  rendered.fill(Qt::white);
  {
    QPainter painter(&rendered);
    check(brandedLayout.paint_page(painter, 0, error), "Render a cover without company decoration");
  }
  const auto headerHasInk = [](const QImage& image)
  {
    for (int y = 10; y < 86; ++y)
    {
      for (int x = 44; x < image.width() - 44; ++x)
      {
        if (image.pixelColor(x, y) != QColor(Qt::white))
        {
          return true;
        }
      }
    }
    return false;
  };
  check(!headerHasInk(rendered), "Disabling cover decoration leaves the reserved header band empty");
  auto decoratedCover = *brandedDocument;
  decoratedCover.configuration.values["decoration_cover"] = "true";
  ReportDocumentLayout decoratedCoverLayout;
  check(decoratedCoverLayout.prepare(decoratedCover, error), "Prepare a cover with company decoration enabled");
  rendered.fill(Qt::white);
  {
    QPainter painter(&rendered);
    check(decoratedCoverLayout.paint_page(painter, 0, error), "Paint the decorated cover");
  }
  check(headerHasInk(rendered), "Enabling cover decoration paints company text and logo in the reserved band");
  check(ReportPdfExporter().write(brandedLayout, directory.filePath("branded.pdf").toStdString(), error), "Export branded PDF");
  check(ReportOdtExporter().write(brandedLayout, directory.filePath("branded.odt").toStdString(), error), "Export branded editable ODT");
  QFile brandedOdt(directory.filePath("branded.odt"));
  check(brandedOdt.open(QIODevice::ReadOnly), "Open branded ODT");
  const auto brandedPackage = brandedOdt.readAll();
  check(brandedPackage.contains("Pictures/company-logo.png") && brandedPackage.contains("Example &amp; Co") &&
    brandedPackage.contains("text:page-number") && brandedPackage.contains("text:page-count") && brandedPackage.contains("style:next-style-name=\"Standard\""),
    "ODT embeds company logo, escaped text, native page fields and cover style");
  const auto stylesStart = brandedPackage.indexOf("<office:document-styles");
  const auto stylesEnd = brandedPackage.indexOf("</office:document-styles>", stylesStart);
  check(stylesStart >= 0 && stylesEnd > stylesStart, "ODT contains its native styles document");
  if (stylesStart >= 0 && stylesEnd > stylesStart)
  {
    QXmlStreamReader styles(brandedPackage.mid(stylesStart, stylesEnd - stylesStart + QByteArray("</office:document-styles>").size()));
    while (!styles.atEnd())
    {
      styles.readNext();
    }
    check(!styles.hasError(), "ODT header/footer tables and cover styles form valid XML");
  }
  ReportConfiguration removeLogo;
  removeLogo.replace_company_logo = true;
  check(brandingCore.configure_patch(removeLogo, error) && !brandingCore.configuration_request().overrides.company_logo,
    "Explicit removal clears the company logo");
  std::cout << failures << " report test failures\n";
  return failures == 0 ? 0 : 1;
}
