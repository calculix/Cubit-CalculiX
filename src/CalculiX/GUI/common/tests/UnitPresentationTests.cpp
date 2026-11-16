#include "GUI/common/CalculiXUnitPresentation.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QWidget>
#include <cmath>
#include <iostream>

namespace
{
int failures = 0;
void check(bool good, const char* message)
{
  if (!good)
  {
    ++failures;
    std::cerr << message << '\n';
  }
}
}

int main(int argc, char** argv)
{
  QApplication application(argc, argv);
  std::string error;
  framework_units().set_display_unit("calculix", "length", "mm", error);
  framework_units().set_display_unit("calculix", "temperature", "degC", error);
  QWidget panel;
  QLabel lengthLabel(&panel);
  QLabel temperatureLabel(&panel);
  QLineEdit length(&panel);
  QLineEdit temperature(&panel);
  QPushButton apply("Apply", &panel);
  length.setText("1000 2000 3000");
  temperature.setText("20");
  auto* units = new CalculiXUnitPresentation(&panel);
  units->bind(lengthLabel, length, "Coordinate", "length");
  units->bind(temperatureLabel, temperature, "Temperature", "temperature");
  check(lengthLabel.text().contains("mm"), "Length label shows input units");
  framework_units().set_display_unit("calculix", "length", "m", error);
  framework_units().set_display_unit("calculix", "temperature", "K", error);
  check(units->refresh(error), "Refresh all dimensional drafts");
  check(length.text() == "1 2 3", "Vector coordinates preserve physical length");
  check(std::abs(temperature.text().toDouble() - 293.15) < 1e-10, "Absolute-temperature draft preserves its affine physical value");
  check(temperatureLabel.text().contains("K"), "Temperature label changes with values");
  length.setText("unfinished");
  const auto savedLabel = temperatureLabel.text();
  const auto savedTemperature = temperature.text();
  framework_units().set_display_unit("calculix", "length", "mm", error);
  framework_units().set_display_unit("calculix", "temperature", "degC", error);
  check(!units->refresh(error) && !apply.isEnabled(), "Invalid draft blocks Apply under changed preferences");
  check(temperatureLabel.text() == savedLabel && temperature.text() == savedTemperature, "Failure does not partially relabel or convert other fields");
  length.setText("1 2 3");
  check(units->refresh(error) && apply.isEnabled(), "Completing the old-unit draft recovers Apply");
  check(length.text() == "1000 2000 3000" && std::abs(temperature.text().toDouble() - 20.0) < 1e-10, "Recovered draft uses the new presentation units");
  length.setText("250");
  CalculiXUnitPresentation::accept_loaded_values(panel);
  check(length.text() == "250", "Current-unit settings reload is not converted twice");
  QTableWidget table(1, 2, &panel);
  table.setHorizontalHeaderLabels({"Time", "Value"});
  table.setItem(0, 0, new QTableWidgetItem("2"));
  units->bind_table(table, 0, "time");
  units->set_table_quantity(table, 0, "frequency");
  check(table.horizontalHeaderItem(0)->text().contains("Frequency"), "Frequency amplitude uses frequency headings");
  framework_units().set_display_unit("calculix", "frequency", "1/ms", error);
  check(units->refresh(error) && std::abs(table.item(0, 0)->text().toDouble() - 0.002) < 1e-12, "Frequency draft converts without time scaling");
  return failures;
}
