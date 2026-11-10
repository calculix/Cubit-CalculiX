#pragma once
#include <functional>
#include <string>
class QWidget;

class ReportExportDialog
{
public:
  // Cancellation succeeds without submitting a command. Replacement requires
  // confirmation and adds the explicit overwrite keyword to the command.
  static bool run(QWidget* parent, const std::string& format,
    const std::function<bool(const std::string&, std::string&)>& submit, std::string& error);
};
