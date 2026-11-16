#pragma once
#include <QTreeWidget>
#include <functional>
#include <memory>
#include <string>
#include <vector>
class ReportDocument;

class ReportOutlineTree final : public QTreeWidget
{
public:
  ReportOutlineTree(QWidget* parent, std::function<void(int)> navigate);
  ~ReportOutlineTree() override;
  void refresh(std::shared_ptr<const ReportDocument> document,
    const std::vector<std::pair<std::string, int>>* pages = nullptr, int pageCount = 0);

private:
  std::shared_ptr<const ReportDocument> snapshot;
  std::vector<std::pair<std::string, int>> pageMap;
  int preparedPages = 0;
};
