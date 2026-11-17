#pragma once
#include "Framework/Reporting/ReportOption.hpp"
#include <vector>

class ReportUserOptions
{
public:
  ReportUserOptions();
  static std::vector<ReportOption> definitions();
  static int initial_preview_page();
  static int initial_preview_zoom();
  static std::string initial_preview_mode();
  static int preview_margin();
  static int preview_gap();
  static int preview_batch_rows();
  static int preview_batch_characters();
  static int preview_batch_cells();
  static int export_checkpoint_cells();
  static int export_chunk_bytes();
  static double figure_max_height();
  static int export_image_max_bytes();
  static int export_image_max_side();
  static int export_image_max_pixels();
};
