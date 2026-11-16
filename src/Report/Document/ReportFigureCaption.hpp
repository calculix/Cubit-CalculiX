#pragma once

#include <QString>
class ReportImage;
class ReportConfiguration;

class ReportFigureCaption
{
public:
  ReportFigureCaption();
  // Captured preferences describe immutable pixels, not regenerated table values.
  static QString text(const ReportImage& image, const ReportConfiguration& configuration);
};
