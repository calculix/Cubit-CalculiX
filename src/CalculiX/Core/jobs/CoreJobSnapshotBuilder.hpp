#pragma once
#include "CalculiXJobRunRecord.hpp"

class CoreJobSnapshotBuilder
{
public:
  CoreJobSnapshotBuilder();
  // Host thread, before export cleanup. Reads final emitted node/element records
  // and prepared typed step conditions; never renders or dispatches commands.
  static bool prepare(int jobId, const std::string& filename,
    CalculiXJobRunRecord& output, std::string& error);
  static bool captureInput(const std::string& filename, CalculiXJobInputBundle& output, std::string& error);
  static std::string inputFingerprint(const CalculiXJobInputBundle& input);
  static std::string setupFingerprint(const CalculiXJobSetupSnapshot& setup);
  static bool validate(const CalculiXJobRunRecord& record, std::string& error);
};
