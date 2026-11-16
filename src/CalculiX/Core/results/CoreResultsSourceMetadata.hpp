#ifndef CORERESULTSSOURCEMETADATA_HPP
#define CORERESULTSSOURCEMETADATA_HPP
#include <QJsonObject>
#include <map>
#include <string>
class CalculiXJobRunRecord;
class CoreResultsSourceMetadata {
public:
  CoreResultsSourceMetadata();
  explicit CoreResultsSourceMetadata(const CalculiXJobRunRecord& record);
  QJsonObject to_json() const;
  static bool from_json(const QJsonObject& value, CoreResultsSourceMetadata& output, std::string& error);
  std::string unitSystem;
  std::map<int, std::string> step_axis_quantities;
  std::string launchId;
  std::string materialProjectId;
  std::map<std::string, int> materialIds;
};
#endif
