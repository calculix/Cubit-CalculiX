#include "ReportTestProviders.hpp"
#include "ReportTestImages.hpp"
#include "Report/Core/ReportCore.hpp"
#include "Report/Core/ReportProjectPersistence.hpp"
#include "Report/Document/ReportDocumentLayout.hpp"
#include "Report/Document/ReportPdfExporter.hpp"
#include "Report/Document/ReportOdtExporter.hpp"
#include "Report/GUI/ReportSettingsCommandText.hpp"
#include "Framework/Reporting/ReportImageRequest.hpp"
#include "Framework/Reporting/ReportProfileStoreInterface.hpp"
#include "Framework/Storage/HDF5Tool.hpp"
#include <QFile>
#include <iostream>

int reportImageTests(const QString& directory, ReportProfileStoreInterface& profiles)
{
  int failures = 0;
  const auto check = [&](bool value, const char* message)
  {
    if (!value)
    {
      std::cerr << message << '\n';
      ++failures;
    }
  };
  std::string error;
  ReportTestProviders providers;
  ReportTestImages images;
  providers.images = &images;
  providers.profiles = &profiles;
  providers.image_options = true;
  ReportCore core(providers);
  check(core.generate("test", 1, "", error) && core.document()->sections.back().images.size() == 1,
    "Report Core collects embedded images through the registered workspace provider");
  const auto first = core.document();
  check(core.generate("test", 1, "", error) && core.document()->id == first->id && core.document()->sections.size() == 2,
    "Regeneration replaces source content without accumulating image sections");
  const auto previous = core.document();
  images.fail = true;
  check(!core.update(error) && core.document() == previous,
    "A fatal image-provider failure preserves the committed report");
  images.fail = false;
  providers.changed_launch = true;
  check(!core.update(error) && core.document() == previous,
    "A new solver launch is detected even when the input file fingerprints are unchanged");
  providers.changed_launch = false;
  check(!core.configure("test.images.camera", "injected camera", error) &&
      !core.configure("test.images.width", "63", error) && !core.configure("test.images.width", "1920.5", error),
    "Choice and integer option metadata reject invalid profile values");
  check(core.configure("test.images.camera", "front", error) && core.configure("test.images.width", "1280", error),
    "Valid template-specific image options are accepted");
  std::vector<ReportProfile> saved;
  check(core.profile("save", "Image settings fixture", saved, error), "Save image settings to framework HDF5 profile store");
  ReportProfile stored;
  check(profiles.load("test", "Image settings fixture", stored, error) && stored.configuration.values.at("test.images.camera") == "front" &&
      stored.configuration.values.at("test.images.width") == "1280", "HDF5 profile retains template image settings");
  ReportTemplate definition;
  providers.describe(definition, error);
  ReportOption option;
  check(ReportOption::from_json(definition.options[2].to_json(), option, error) && option.minimum == 64 && option.maximum == 8192,
    "Integer metadata round-trips through the option JSON contract");
  check(ReportOption::from_json(definition.options[1].to_json(), option, error) && option.choices.size() == 2,
    "Choice metadata round-trips through the option JSON contract");
  ReportConfiguration patch;
  patch.values = {{"author", "Author"}, {"test.images.camera", "front"}, {"test.images.width", "1280"}};
  std::string command;
  check(ReportSettingsCommandText::build(patch, QString(), command, error, &definition) &&
      command == "report configure settings author \"Author\" option \"test.images.camera\" value \"front\" option \"test.images.width\" value \"1280\"",
    "The GUI emits readable common fields and repeated quoted template key/value options in one atomic command");
  const auto oldCommand = command;
  patch.values["unknown"] = "value";
  check(!ReportSettingsCommandText::build(patch, QString(), command, error, &definition) && command == oldCommand,
    "Unknown template settings preserve the prior command");
  ReportImageRequest request, restoredRequest;
  request.provider = "ccx";
  request.purpose = "setup";
  request.context = "step:7";
  request.source = core.document()->source;
  request.width = 1920;
  request.height = 1080;
  check(ReportImageRequest::from_json(request.to_json(), restoredRequest, error) &&
      restoredRequest.purpose == "setup" && restoredRequest.context == "step:7" && restoredRequest.width == 1920,
    "Provider-defined image purpose/context survive request serialization");
  ReportDocument restored;
  check(ReportDocument::from_json(core.document()->to_json(), restored, error) &&
      restored.sections.back().images.front().base64_data == core.document()->sections.back().images.front().base64_data,
    "Embedded images round-trip without access to a capture provider");
  const auto project = (directory + "/images.cub").toStdString();
  {
    HDF5Tool file(project, true);
    check(ReportProjectPersistence().write(file, core.session(), error) && file.close(), "Persist report images in the project");
  }
  ReportSession session;
  {
    HDF5Tool file(project, false);
    check(ReportProjectPersistence().read(file, session, error) && session.documents.size() == 1 &&
        session.documents.begin()->second->sections.back().images.size() == 1, "Restore stored images without rendering them again");
  }
  ReportDocumentLayout layout;
  check(layout.prepare(*core.document(), error) && layout.page_count() > 0, "Preview layout accepts section images");
  const auto pdfPath = directory + "/setup-images.pdf";
  const auto odtPath = directory + "/setup-images.odt";
  check(ReportPdfExporter().write(layout, pdfPath.toStdString(), error), "Export PDF containing setup image");
  check(ReportOdtExporter().write(layout, odtPath.toStdString(), error), "Export ODT containing setup image");
  QFile pdf(pdfPath), odt(odtPath);
  check(pdf.open(QIODevice::ReadOnly) && pdf.readAll().contains("/Subtype /Image"), "PDF embeds the raster image");
  check(odt.open(QIODevice::ReadOnly) && odt.readAll().contains("Pictures/image1.png"), "ODT packages the embedded PNG");
  ReportImageRequest figureRequest;
  figureRequest.provider = "postprocessor";
  figureRequest.visual = ReportVisualReference{ReportVisualKind::View,"view1"};
  check(core.add_image(figureRequest,"User snapshot",error) && core.document()->sections.back().id == "user.figures" &&
      core.document()->sections.back().images.front().caption == "User snapshot", "Append immutable Postprocessor figure");
  const auto figure = core.document()->sections.back().images.front().base64_data;
  check(core.generate("test",1,"",error) && core.document()->sections.back().id != "", "Regenerate report with user figures");
  bool retained = false;
  for (const auto& section : core.document()->sections)
  {
    if (section.id == "user.figures")
    {
      retained = section.images.size() == 1 && section.images.front().base64_data == figure;
    }
  }
  check(retained,"Report regeneration discarded or refreshed user snapshot");
  const auto beforeFailure = core.document();
  images.fail = true;
  check(!core.add_image(figureRequest,"Rejected",error) && core.document() == beforeFailure,
    "Failed capture mutated committed Report");
  images.fail = false;
  ReportSession restoredFigures;
  check(ReportSession::from_json(core.session().to_json(),restoredFigures,error) &&
      restoredFigures.documents.at(core.session().active_id)->sections.size() == core.document()->sections.size(),
    "Report session roundtrip lost Figures section");
  ReportImageRequest restoredRequest;
  check(ReportImageRequest::from_json(figureRequest.to_json(),restoredRequest,error) && restoredRequest.visual &&
      restoredRequest.visual->reference == "view1", "Typed visual reference roundtrip failed");
  auto legacy = figureRequest.to_json();
  legacy.remove("visual");
  check(ReportImageRequest::from_json(legacy,restoredRequest,error) && !restoredRequest.visual,
    "Legacy report image request requires a visual reference");
  ReportImage provenanceImage, restoredImage;
  provenanceImage.provenance = QJsonObject{{"camera","snapshot"}};
  check(ReportImage::from_json(provenanceImage.to_json(),restoredImage,error) && restoredImage.provenance == provenanceImage.provenance,
    "Image provenance roundtrip failed");
  return failures;
}
