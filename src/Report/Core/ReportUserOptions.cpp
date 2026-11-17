#include "ReportUserOptions.hpp"
#include <utility>

ReportUserOptions::ReportUserOptions() = default;

int ReportUserOptions::initial_preview_page()
{
  return 1;
}

int ReportUserOptions::initial_preview_zoom()
{
  return 100;
}

std::vector<ReportOption> ReportUserOptions::definitions()
{
  std::vector<ReportOption> result;
  for (const auto& entry : std::vector<std::vector<std::string>>
    {{"title", "Report title", "text", "Engineering report"},
     {"author", "Author", "text", ""}, {"project", "Project", "text", ""},
     {"revision", "Revision", "text", ""}, {"precision", "Significant digits", "precision", "5"},
     {"page_size", "Page size", "page_size", "A4"},
     {"cover", "Cover page", "boolean", "true"}, {"contents", "Contents", "boolean", "true"},
     {"company_name", "Company name", "text", "Company Name"},
     {"company_details", "Company details", "text", "Address | Telephone | Email"},
     {"header_left", "Header left", "decoration", "{company_name}"},
     {"header_center", "Header centre", "decoration", "{title}"},
     {"header_right", "Header right", "decoration", "{project}"},
     {"footer_left", "Footer left", "decoration", "{company_details}"},
     {"footer_center", "Footer centre", "decoration", "Revision {revision}"},
     {"footer_right", "Footer right", "decoration", "Page {page} of {pages}"},
     {"header_line", "Header line", "boolean", "true"},
     {"footer_line", "Footer line", "boolean", "true"},
     {"decoration_cover", "Header/footer on cover", "boolean", "true"},
     {"logo_placement", "Logo placement", "logo_placement", "header_left"},
     {"logo_width_mm", "Logo maximum width (mm)", "logo_width", "30"}})
  {
    ReportOption option;
    option.key = entry[0];
    option.label = entry[1];
    option.kind = entry[2];
    option.default_value = entry[3];
    if (option.kind == "boolean")
    {
      option.group_id = "report.content.document";
      if (option.key == "header_line" || option.key == "footer_line" || option.key == "decoration_cover")
      {
        option.group_id = "report.content.decoration";
      }
    }
    result.push_back(std::move(option));
  }
  return result;
}

std::string ReportUserOptions::initial_preview_mode()
{
  return "page";
}

int ReportUserOptions::preview_margin()
{
  return 16;
}

int ReportUserOptions::preview_gap()
{
  return 16;
}

int ReportUserOptions::preview_batch_rows()
{
  return 128;
}

int ReportUserOptions::preview_batch_characters()
{
  return 32768;
}

int ReportUserOptions::preview_batch_cells()
{
  return 2048;
}

int ReportUserOptions::export_checkpoint_cells()
{
  return 256;
}

int ReportUserOptions::export_chunk_bytes()
{
  return 65536;
}

double ReportUserOptions::figure_max_height()
{
  return 300.0;
}

int ReportUserOptions::export_image_max_bytes()
{
  return 32 * 1024 * 1024;
}

int ReportUserOptions::export_image_max_side()
{
  return 8192;
}

int ReportUserOptions::export_image_max_pixels()
{
  return 32000000;
}
