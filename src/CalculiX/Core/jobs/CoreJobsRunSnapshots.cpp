#include "CoreJobs.hpp"
#include "CoreJobSnapshotBuilder.hpp"
#include "CubitMessage.hpp"
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QUuid>
#include <exception>

bool CoreJobs::get_run_record(int jobId, std::shared_ptr<const CalculiXJobRunRecord>& output, std::string& error) const
{
  const auto found = runRecords.find(jobId);
  if (found == runRecords.end())
  {
    error = "The job has no archived solver launch.";
    return false;
  }
  output = found->second;
  error.clear();
  return true;
}

void CoreJobs::clear_prepared_setup(int jobId)
{
  preparedRuns.erase(jobId);
  preparedRunErrors.erase(jobId);
  unsupportedPreparedRuns.erase(jobId);
}

void CoreJobs::freeze_prepared_setup(int jobId, const std::string& filename, bool supportedContext) noexcept
{
  try
  {
    clear_prepared_setup(jobId);
    std::string error;
    CalculiXJobRunRecord candidate;
    if (!supportedContext)
    {
      unsupportedPreparedRuns.insert(jobId);
    }
    if (!supportedContext || !CoreJobSnapshotBuilder::prepare(jobId, filename, candidate, error))
    {
      preparedRunErrors[jobId] = supportedContext ? error : "Topology and adjoint setup snapshots are not supported yet.";
      return;
    }
    preparedRuns.emplace(jobId, std::move(candidate));
  }
  catch (const std::exception& exception)
  {
    PRINT_WARNING("Job %d: cannot freeze setup: %s\n", jobId, exception.what());
  }
  catch (...)
  {
    PRINT_WARNING("Job %d: cannot freeze setup.\n", jobId);
  }
}

void CoreJobs::prepare_run_record(int jobId, const std::string& filename, bool imported, CalculiXJobRunRecord& output) noexcept
{
  output = {};
  output.jobId = jobId;
  try
  {
    output.launchId = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    output.launchedAt = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs).toStdString();
    output.workingDirectory = QDir::currentPath().toStdString();
    std::string error;
    if (unsupportedPreparedRuns.count(jobId) != 0)
    {
      output.setupUnavailableReason = "Run input/setup archiving is not enabled for topology or adjoint exports.";
    }
    else if (!CoreJobSnapshotBuilder::captureInput(filename, output.input, error))
    {
      output.setupUnavailableReason = "Input archive unavailable: " + error;
    }
    else
    {
      const auto prepared = preparedRuns.find(jobId);
      if (!imported && prepared != preparedRuns.end() && prepared->second.input.manifestFingerprint == output.input.manifestFingerprint)
      {
        output.setup = prepared->second.setup;
      }
      else
      {
        const auto reason = preparedRunErrors.find(jobId);
        output.setupUnavailableReason = imported ? "Imported input has no verified typed setup snapshot." :
          reason != preparedRunErrors.end() ? reason->second : "Final input does not match a prepared setup snapshot.";
      }
      if (!CoreJobSnapshotBuilder::validate(output, error))
      {
        output.setup.reset();
        output.setupUnavailableReason = "Setup snapshot validation failed: " + error;
      }
    }
    clear_prepared_setup(jobId);
    if (!output.setup && !output.setupUnavailableReason.empty())
    {
      PRINT_WARNING("Job %d: %s\n", jobId, output.setupUnavailableReason.c_str());
    }
  }
  catch (const std::exception& exception)
  {
    output.setup.reset();
    PRINT_WARNING("Job %d: cannot prepare run archive: %s\n", jobId, exception.what());
  }
  catch (...)
  {
    output.setup.reset();
    PRINT_WARNING("Job %d: cannot prepare run archive.\n", jobId);
  }
}

void CoreJobs::publish_run_record(CalculiXJobRunRecord&& record) noexcept
{
  runRecords.erase(record.jobId);
  try
  {
    record.launchedAt = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs).toStdString();
    runRecords.emplace(record.jobId, std::make_shared<const CalculiXJobRunRecord>(std::move(record)));
  }
  catch (const std::exception& exception)
  {
    PRINT_WARNING("Cannot publish job launch archive: %s\n", exception.what());
  }
  catch (...)
  {
    PRINT_WARNING("Cannot publish job launch archive.\n");
  }
}

bool CoreJobs::copy_job_input(const std::string& source, const std::string& destination)
{
  const QFileInfo original(QString::fromStdString(source)), target(QString::fromStdString(destination));
  if (!original.canonicalFilePath().isEmpty() && original.canonicalFilePath() == target.canonicalFilePath())
  {
    return true;
  }
  QFile input(original.absoluteFilePath());
  QSaveFile output(target.absoluteFilePath());
  output.setDirectWriteFallback(false);
  if (!input.open(QIODevice::ReadOnly) || !output.open(QIODevice::WriteOnly))
  {
    PRINT_ERROR("Cannot open job input for copying.\n");
    return false;
  }
  while (!input.atEnd())
  {
    const auto bytes = input.read(1024 * 1024);
    if (input.error() != QFileDevice::NoError || bytes.isEmpty() || output.write(bytes) != bytes.size())
    {
      PRINT_ERROR("Cannot copy job input bytes.\n");
      return false;
    }
  }
  if (!output.commit())
  {
    PRINT_ERROR("Cannot commit copied job input.\n");
    return false;
  }
  return true;
}
