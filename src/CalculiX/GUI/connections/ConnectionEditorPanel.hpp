#pragma once
#include <QWidget>

class ConnectionEditorPanel : public QWidget
{
public:
  explicit ConnectionEditorPanel(const QString& feature, QWidget* parent = nullptr);
};
