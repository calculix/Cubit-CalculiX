#include "GUI/common/CalculiXUnitPresentation.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Core/units/UnitLabelFormat.hpp"
#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QWidget>
#include <cmath>
#include <iostream>
#include <utility>

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

void unit_label_formats()
{
  const std::pair<std::string, QString> samples[] = {
    {"N^0.5*m^0.5*s^-1", QStringLiteral("\u221a(N\u00b7m)/s")},
    {"N/mm^2", QStringLiteral("N/mm\u00b2")},
    {"kg/m^3", QStringLiteral("kg/m\u00b3")},
    {"N*m", QStringLiteral("N\u00b7m")},
    {"N*m^-2*s^-1", QStringLiteral("N/(m\u00b2\u00b7s)")},
    {"m/s*kg", QStringLiteral("m\u00b7kg/s")},
    {"1/s^0.5", QStringLiteral("1/\u221as")},
    {"degC", QStringLiteral("\u00b0C")},
    {"degF", QStringLiteral("\u00b0F")},
    {"deg", QStringLiteral("\u00b0")},
    {"delta_degC", QStringLiteral("\u0394\u00b0C")},
    {"Hz", QStringLiteral("Hz")},
    {"1", QStringLiteral("1")},
    {"", QString()},
    {"(N/mm^2)^0.5", QStringLiteral("(N/mm^2)^0.5")},
    {"m^0.25", QStringLiteral("m^0.25")},
    {"m//s", QStringLiteral("m//s")},
    {"m s", QStringLiteral("m s")}
  };
  for (const auto& sample : samples)
  {
    check(UnitLabelFormat::format(sample.first) == sample.second, ("Compact unit label: " + sample.first).c_str());
  }
  check(UnitLabelFormat::power("MPa", QStringLiteral("-n")) == QStringLiteral("(MPa)\u207b\u207f"), "Symbolic creep stress exponent uses superscripts");
  check(UnitLabelFormat::power("s", QStringLiteral("-(m+1)")) == QStringLiteral("(s)\u207b\u207d\u1d50\u207a\u00b9\u207e"), "Symbolic creep time exponent preserves grouping");
  check(UnitLabelFormat::power("s", QStringLiteral("alpha")) == QStringLiteral("(s)^(alpha)"), "Unsupported symbolic exponent retains input spelling");
}
}

int main(int argc, char** argv)
{
  QApplication application(argc, argv);
  unit_label_formats();
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
  framework_units().set_display_unit("calculix", "frequency", "", error);
  QTableWidget table(1, 2, &panel);
  table.setHorizontalHeaderLabels({"Time", "Value"});
  table.setItem(0, 0, new QTableWidgetItem("2"));
  units->bind_table(table, 0, "time");
  units->set_table_quantity(table, 0, "frequency");
  check(table.horizontalHeaderItem(0)->text().contains("Frequency"), "Frequency amplitude uses frequency headings");
  check(table.horizontalHeaderItem(0)->text().contains("[Hz]"), "Frequency display defaults to Hz");
  framework_units().set_display_unit("calculix", "frequency", "1/ms", error);
  check(units->refresh(error) && std::abs(table.item(0, 0)->text().toDouble() - 0.002) < 1e-12, "Frequency draft converts without time scaling");
  const std::string modalUnit = "N^0.5*m^0.5*s^-1";
  check(framework_units().set_display_unit("calculix", "modal_force", modalUnit, error), "Set explicit modal unit expression");
  QLabel modalLabel(&panel);
  QLineEdit modalValue(&panel);
  modalValue.setText("2");
  units->bind(modalLabel, modalValue, "DELTMX", "modal_force");
  const QString expectedTitle = QStringLiteral("DELTMX [\u221a(N\u00b7m)/s]");
  check(modalLabel.text() == expectedTitle, "Field label uses compact unit notation");
  check(modalValue.text() == "2" && modalValue.toolTip().contains(QString::fromStdString(modalUnit)), "Formatting leaves the numeric draft unchanged and retains raw input units in its tooltip");
  QTableWidget modalTable(1, 1, &panel);
  modalTable.setHorizontalHeaderLabels({"DELTMX"});
  modalTable.setItem(0, 0, new QTableWidgetItem("2"));
  units->bind_table(modalTable, 0, "modal_force");
  check(modalTable.horizontalHeaderItem(0)->text() == expectedTitle, "Table heading uses the same compact unit notation");
  check(modalTable.horizontalHeaderItem(0)->toolTip().contains(QString::fromStdString(modalUnit)), "Table heading retains raw input units in its tooltip");
  check(CalculiXUnitPresentation::title("DELTMX", "modal_force") == expectedTitle, "Property title uses compact unit notation");
  return failures;
}
