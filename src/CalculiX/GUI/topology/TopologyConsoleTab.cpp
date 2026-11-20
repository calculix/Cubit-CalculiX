#include "TopologyConsoleTab.hpp"
#include "Core/topology/CoreTopologyConsole.hpp"
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QTextCursor>
#include <QVBoxLayout>

TopologyConsoleTab::TopologyConsoleTab(QWidget* parent) : QWidget(parent)
{
  auto* layout = new QVBoxLayout(this);
  auto* actions = new QHBoxLayout;
  follow = new QCheckBox(tr("Follow latest"),this);
  follow->setChecked(true);
  actions->addWidget(follow);
  actions->addStretch();
  auto* copy = new QPushButton(tr("Copy"),this);
  actions->addWidget(copy);
  layout->addLayout(actions);
  output = new QPlainTextEdit(this);
  output->setObjectName("CalculiX.Topology.Console");
  output->setReadOnly(true);
  output->setMaximumBlockCount(4000);
  output->setLineWrapMode(QPlainTextEdit::NoWrap);
  output->setPlaceholderText(tr("Topology run messages will appear here."));
  layout->addWidget(output);
  connect(copy,&QPushButton::clicked,this,[this]()
  {
    if (output->textCursor().hasSelection())
    {
      output->copy();
    }
    else
    {
      QApplication::clipboard()->setText(output->toPlainText());
    }
  });
  connect(follow,&QCheckBox::toggled,this,[this](bool enabled)
  {
    if (enabled)
    {
      output->verticalScrollBar()->setValue(output->verticalScrollBar()->maximum());
    }
  });
}

void TopologyConsoleTab::refresh(int topology)
{
  if (selected != topology)
  {
    selected = topology;
    generation = 0;
    sequence = 0;
    output->clear();
  }
  const auto batch = CoreTopologyConsole::read(topology,generation,sequence);
  if (batch.generation != generation)
  {
    generation = batch.generation;
    sequence = 0;
    output->clear();
  }
  if (batch.entries.empty())
  {
    return;
  }
  auto* scroll = output->verticalScrollBar();
  const int previousScroll = scroll->value();
  QString text;
  for (const auto& entry : batch.entries)
  {
    if (!text.isEmpty())
    {
      text += '\n';
    }
    text += QString::fromUtf8(entry.text.c_str());
    sequence = entry.sequence;
  }
  output->appendPlainText(text);
  if (follow->isChecked())
  {
    scroll->setValue(scroll->maximum());
  }
  else
  {
    scroll->setValue(previousScroll);
  }
}
