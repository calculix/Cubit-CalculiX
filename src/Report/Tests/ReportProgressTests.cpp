#include "ReportTestProgress.hpp"
#include "Framework/Reporting/ReportResultRequest.hpp"
#include "ReportTestProviders.hpp"
#include "Report/Core/ReportCore.hpp"
#include "Report/Interfaces/ReportApplicationInterface.hpp"
#include "Report/Document/ReportDocumentLayout.hpp"
#include "Report/GUI/ReportWorkspace.hpp"
#include "CalculiX/Core/reporting/CalculiXReportInputReader.hpp"
#include <QFile>
#include <QJsonArray>
#include <QMainWindow>
#include <algorithm>
#include <iostream>

int reportProgressTests(const QString& directory)
{
  int failures = 0;
  const auto check = [&](bool result, const char* message)
  {
    if (!result)
    {
      std::cerr << message << '\n';
      ++failures;
    }
  };
  std::string error;
  ReportTestProviders providers;
  ReportCore core(providers);
  ReportTestProgress progress;
  check(core.generate("test", 1, "", error, &progress), "Generate publishes progress through the Core contract");
  check(std::find(progress.stages.begin(), progress.stages.end(), "Collecting model tables and analysis steps") != progress.stages.end(),
    "Model collection has an explicit progress stage");
  check(std::find(progress.stages.begin(), progress.stages.end(), "Validating generated report") != progress.stages.end(),
    "Unit/document validation has an explicit progress stage");
  check(core.session().request.progress == nullptr, "Committed session does not retain its operation progress pointer");
  const auto previous = core.document();
  progress.cancelStage = "Collecting model";
  check(!core.update(error, &progress) && core.document() == previous, "Cancellation preserves the previous committed report");
  check(core.session().request.progress == nullptr, "Cancellation leaves session progress pointers empty");
  progress.cancelStage = "Report content complete";
  check(!core.update(error, &progress) && core.document() == previous, "Cancellation at the final checkpoint prevents report publication");
  progress.cancelStage.clear();
  check(core.update(error, &progress), "Generation can resume after cancellation");

  const auto beforePreparation = core.document();
  const auto beforeSession = core.session().to_json();
  bool preparedBeforePublication = false;
  const auto failPreparation = [&](const ReportDocument& candidate, std::string& message)
  {
    preparedBeforePublication = core.document() == beforePreparation && &candidate != beforePreparation.get();
    message = "Preview preparation cancelled by regression fixture.";
    return false;
  };
  check(!core.update(error, &progress, failPreparation) && preparedBeforePublication &&
    core.document() == beforePreparation && core.session().to_json() == beforeSession,
    "Preview failure leaves the previous report and session unpublished");
  const auto prepare = [&](const ReportDocument&, std::string&)
  {
    preparedBeforePublication = core.document() == beforePreparation;
    return true;
  };
  progress.cancelStage = "Publishing report";
  check(!core.update(error, &progress, prepare) && core.document() == beforePreparation,
    "Cancellation after preview preparation still prevents publication");
  progress.cancelStage.clear();
  check(core.update(error, &progress, prepare) && preparedBeforePublication && core.document() != beforePreparation,
    "Successful preparation precedes report publication");
  check(ReportProgress::fraction(ReportProgressStage::Pagination, 0, 1) >=
    ReportProgress::fraction(ReportProgressStage::PreviewContent, 1, 1),
    "Pagination follows content construction in overall workflow progress");
  check(ReportProgress::fraction(ReportProgressStage::Contents, 1, 1) < 1.0,
    "Progress callbacks cannot reach 100 percent before publication");
  check(ReportProgress::fraction(ReportProgressStage::PreviewContent, 0, 1, ReportProgressStage::PreviewContent) == 0.0,
    "Preview-only preparation starts its own workflow at zero");

  auto application = std::make_shared<ReportApplicationInterface>(providers);
  check(application->generate("test", 1, "", error) && !application->prepared_layout(),
    "Headless content generation does not construct a Qt layout");
  QMainWindow host;
  const auto command = [](const std::string&, std::string&)
  {
    return true;
  };
  ReportWorkspace workspace(&host, application, command, command, QIcon());
  workspace.refresh();
  check(!application->prepared_layout(), "GUI refresh never starts expensive preview preparation");
  std::shared_ptr<const ReportDocumentLayout> preparedLayout;
  check(application->layout(preparedLayout, error) && preparedLayout == application->prepared_layout(),
    "An explicit preview service prepares and publishes the matching cache");
  workspace.refresh();
  check(application->prepared_layout() == preparedLayout, "GUI refresh reuses the already prepared immutable layout");

  ReportRequest request;
  request.provider = "test";
  request.job_id = 1;
  request.progress = &progress;
  ReportRequest decoded;
  check(ReportRequest::from_json(request.to_json(), decoded, error) && decoded.progress == nullptr,
    "Progress is excluded from persisted requests");

  ReportResultRequest resultRequest;
  resultRequest.provider = "test";
  resultRequest.source.workspace_id = "test";
  resultRequest.source.job_id = 1;
  resultRequest.progress = &progress;
  ReportResultRequest decodedResult;
  check(ReportResultRequest::from_json(resultRequest.to_json(), decodedResult, error) &&
    decodedResult.progress == nullptr, "Result collection progress is never persisted");

  const auto filename = directory + "/report-progress.inp";
  QFile file(filename);
  check(file.open(QIODevice::WriteOnly), "Create progress input fixture");
  file.write("*NODE\n1,0,0,0\n");
  file.close();
  ReportSource source;
  source.workspace_id = "calculix";
  source.job_id = 1;
  source.input_files = {filename.toStdString()};
  CalculiXReportInputReader reader;
  check(reader.read(source, error, &progress), "Input parsing reports progress without a native GUI");
  const auto records = reader.records();
  progress.cancelStage = "Reading report input";
  check(!reader.read(source, error, &progress) && reader.records() == records,
    "Input parsing cancellation does not publish partial records");
  return failures;
}
