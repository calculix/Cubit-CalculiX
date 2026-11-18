#include "MeshQualityDefaults.hpp"
#include "Framework/Reporting/ReportTemplate.hpp"
#include <QString>
#include <utility>
MeshQualityDefaults::MeshQualityDefaults() = default;
std::vector<MeshQualityMetric> MeshQualityDefaults::metrics()
{
  std::vector<MeshQualityMetric> result;
  const auto add = [&](const std::string& family, const std::string& id,
    const std::string& native, const std::string& label, std::optional<double> lower,
    std::optional<double> upper, bool enabled = false, bool reference = false)
  {
    MeshQualityMetric metric;
    metric.family = family;
    metric.id = id;
    metric.native_name = native;
    metric.label = label;
    metric.minimum = lower;
    metric.maximum = upper;
    metric.enabled_by_default = enabled;
    metric.requires_group_reference = reference;
    result.push_back(std::move(metric));
  };
  // Hex limits follow the supplied Cubit Quality Defaults screen. Other family
  // limits follow Coreform's published metric tables, not the user's live GUI.
  add("hex", "aspect_ratio", "aspect ratio", "Aspect Ratio", 1, 4);
  add("hex", "condition", "condition", "Condition No.", 1, 8);
  add("hex", "diagonal_ratio", "diagonal", "Diagonal Ratio", .65, 1);
  add("hex", "dimension", "dimension", "Dimension", 0, 1e30);
  result.back().quantity = "length";
  result.back().unit = "mm";
  add("hex", "volume", "volume", "Element Volume", 0, 1e30);
  result.back().quantity = "volume";
  result.back().unit = "mm^3";
  add("hex", "jacobian", "jacobian", "Jacobian", 0, 1e30);
  result.back().quantity = "volume";
  result.back().unit = "mm^3";
  add("hex", "distortion", "distortion", "Distortion", .6, 1);
  add("hex", "scaled_jacobian", "scaled jacobian", "Scaled Jacobian", .5, 1.01, true);
  add("hex", "shape", "shape", "Shape", .3, 1);
  add("hex", "relative_size", "relative size", "Relative Size", .5, 1, false, true);
  add("hex", "shape_size", "shape and size", "Shape and Size", .2, 1, false, true);
  add("hex", "shear", "shear", "Shear", .3, 1);
  add("hex", "shear_size", "shear and size", "Shear and Size", .2, 1, false, true);
  add("hex", "skew", "skew", "Skew", 0, .5);
  add("hex", "stretch", "stretch", "Stretch", .25, 1);
  add("hex", "taper", "taper", "Taper", 0, .4);
  add("tet", "aspect_beta", "aspect ratio beta", "Aspect Ratio Beta", 1, 3);
  add("tet", "aspect_gamma", "aspect ratio gamma", "Aspect Ratio Gamma", 1, 3);
  add("tet", "condition", "condition", "Condition No.", 1, 3);
  add("tet", "scaled_jacobian", "scaled jacobian", "Scaled Jacobian", .2, 1, true);
  add("tet", "shape", "shape", "Shape", .2, 1);
  add("tet", "relative_size", "relative size", "Relative Size", .2, 1, false, true);
  add("tet", "shape_size", "shape and size", "Shape and Size", .2, 1, false, true);
  add("tet", "distortion", "distortion", "Distortion", .6, 1);
  add("tet", "normalized_inradius", "normalized inradius", "Normalized Inradius", .15, 1);
  add("tet", "mean_ratio", "mean ratio", "Mean Ratio", .2, 1);
  add("quad", "aspect_ratio", "aspect ratio", "Aspect Ratio", 1, 4);
  add("quad", "condition", "condition", "Condition No.", 1, 4);
  add("quad", "scaled_jacobian", "scaled jacobian", "Scaled Jacobian", .5, 1, true);
  add("quad", "shape", "shape", "Shape", .3, 1);
  add("quad", "relative_size", "relative size", "Relative Size", .3, 1, false, true);
  add("quad", "shape_size", "shape and size", "Shape and Size", .2, 1, false, true);
  add("quad", "skew", "skew", "Skew", 0, .5);
  add("quad", "taper", "taper", "Taper", 0, .7);
  add("quad", "warpage", "warpage", "Warpage", .9, 1);
  add("quad", "distortion", "distortion", "Distortion", .6, 1);
  add("tri", "aspect_ratio", "aspect ratio", "Aspect Ratio", 1, 3);
  add("tri", "condition", "condition", "Condition No.", 1, 1.3);
  add("tri", "scaled_jacobian", "scaled jacobian", "Scaled Jacobian", .2, 1, true);
  add("tri", "shape", "shape", "Shape", .25, 1);
  add("tri", "relative_size", "relative size", "Relative Size", .25, 1, false, true);
  add("tri", "shape_size", "shape and size", "Shape and Size", .25, 1, false, true);
  add("tri", "distortion", "distortion", "Distortion", .6, 1);
  for (const auto& family : {std::string("tet"), std::string("quad"), std::string("tri")})
  {
    std::string quantity = "area", unit = "mm^2", native = "area", label = "Element Area";
    if (family == "tet")
    {
      quantity = "volume";
      unit = "mm^3";
      native = "volume";
      label = "Element Volume";
    }
    add(family, native, native, label, std::nullopt, std::nullopt);
    result.back().quantity = quantity;
    result.back().unit = unit;
    if (family != "tri")
    {
      add(family, "jacobian", "jacobian", "Jacobian", std::nullopt, std::nullopt);
      result.back().quantity = quantity;
      result.back().unit = unit;
    }
  }
  return result;
}
void MeshQualityDefaults::add_options(ReportTemplate& definition)
{
  definition.content_groups.push_back({"mesh.quality.content", "Mesh quality", "ccx.content.model", "mesh.quality.enabled"});
  const auto add = [&](const std::string& key, const std::string& label, const std::string& kind,
    const std::string& value, const std::string& group)
  {
    ReportOption option;
    option.key = key;
    option.label = label;
    option.kind = kind;
    option.default_value = value;
    option.group_id = group;
    definition.options.push_back(option);
    definition.defaults.values[key] = value;
  };
  add("mesh.quality.enabled", "Mesh quality", "boolean", "true", "ccx.content.model");
  add("mesh.quality.statistics", "Per-block statistics", "boolean", "true", "mesh.quality.content");
  add("mesh.quality.images", "Quality images", "boolean", "true", "mesh.quality.content");
  add("mesh.quality.failed_images", "Failing-element images", "boolean", "true", "mesh.quality.content");
  add("mesh.quality.element_values", "Detailed element values", "boolean", "false", "mesh.quality.content");
  for (const auto& metric : metrics())
  {
    const auto key = metric.key();
    std::string enabled = "false";
    if (metric.enabled_by_default)
    {
      enabled = "true";
    }
    // Dedicated sibling pages own these controls; they are not content-tree leaves.
    add(key + ".unit", metric.label + " limits unit", "quality_unit", metric.unit, "");
    add(key + ".enabled", metric.label, "quality_boolean", enabled, "");
    const auto number = [](const std::optional<double>& value)
    {
      if (value)
      {
        return QString::number(*value, 'g', 15).toStdString();
      }
      return std::string();
    };
    add(key + ".minimum", metric.label + " minimum", "quality_limit", number(metric.minimum), "");
    add(key + ".maximum", metric.label + " maximum", "quality_limit", number(metric.maximum), "");
  }
}
