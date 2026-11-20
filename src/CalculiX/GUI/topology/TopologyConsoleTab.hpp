#pragma once
#include <QWidget>
#include <cstdint>

class QCheckBox;
class QPlainTextEdit;

class TopologyConsoleTab : public QWidget
{
public:
  explicit TopologyConsoleTab(QWidget* parent);
  void refresh(int topology);

private:
  int selected = -1;
  std::uint64_t generation = 0;
  std::uint64_t sequence = 0;
  QPlainTextEdit* output;
  QCheckBox* follow;
};
