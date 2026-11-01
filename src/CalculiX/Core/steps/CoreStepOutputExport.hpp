#ifndef CORESTEPOUTPUTEXPORT_HPP
#define CORESTEPOUTPUTEXPORT_HPP

#include <string>
#include <vector>

class CoreSteps;
class CoreHistoryOutputs;
class CoreFieldOutputs;

class CoreStepOutputExport
{
public:
  static void append_history(CoreSteps& steps, CoreHistoryOutputs& outputs, int historyoutputs_id, std::vector<std::string>& lines);
  static void append_field(CoreSteps& steps, CoreFieldOutputs& outputs, int fieldoutputs_id, std::vector<std::string>& lines);
};

#endif
