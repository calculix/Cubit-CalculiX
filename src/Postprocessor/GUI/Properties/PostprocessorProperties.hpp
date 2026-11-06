#ifndef POSTPROCESSORPROPERTIES_HPP
#define POSTPROCESSORPROPERTIES_HPP

#include "Postprocessor/Core/sources/DataSource.hpp"
#include <QWidget>

class QLabel;

class PostprocessorProperties : public QWidget
{
public:
  explicit PostprocessorProperties(QWidget* parent = nullptr);
  void show_source(const Postprocessor::DataSource* source);

private:
  QLabel* name;
  QLabel* origin;
};

#endif // POSTPROCESSORPROPERTIES_HPP
