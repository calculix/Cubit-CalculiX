#include "PostprocessorSessionCodec.hpp"
#include "../plots/PlotCodec.hpp"
#include "../annotations/AnnotationCodec.hpp"
#include "../views/CameraOperations.hpp"
#include <QJsonArray>
#include <QDir>
#include <QFileInfo>
#include <algorithm>
#include <cmath>
#include <limits>
#include <set>
namespace
{
  using namespace Postprocessor;
  using Object = QJsonObject;
  using Array = QJsonArray;
  QString q(const std::string& value)
  {
    return QString::fromUtf8(value.c_str());
  }
  bool foreign_absolute(const QString& path)
  {
    const bool drive = path.size() >= 3 && ((path[0] >= QLatin1Char('A') && path[0] <= QLatin1Char('Z')) ||
      (path[0] >= QLatin1Char('a') && path[0] <= QLatin1Char('z'))) && path[1] == QLatin1Char(':') &&
      (path[2] == QLatin1Char('/') || path[2] == QLatin1Char('\\'));
    return drive || path.startsWith(QStringLiteral("\\\\"));
  }
  bool text(const Object& object, const char* key, std::string& output)
  {
    const auto value = object.value(QLatin1String(key));
    if (!value.isString())
    {
      return false;
    }
    output = value.toString().toUtf8().toStdString();
    return output.find('\0') == std::string::npos;
  }
  bool integer(const Object& object, const char* key, int& output)
  {
    const auto value = object.value(QLatin1String(key));
    if (!value.isDouble())
    {
      return false;
    }
    const double number = value.toDouble();
    if (!std::isfinite(number) || std::floor(number) != number || number < std::numeric_limits<int>::min() || number > std::numeric_limits<int>::max())
    {
      return false;
    }
    output = static_cast<int>(number);
    return true;
  }
  bool number(const Object& object, const char* key, double& output)
  {
    const auto value = object.value(QLatin1String(key));
    if (!value.isDouble() || !std::isfinite(value.toDouble()))
    {
      return false;
    }
    output = value.toDouble();
    return true;
  }
  bool boolean(const Object& object, const char* key, bool& output)
  {
    const auto value = object.value(QLatin1String(key));
    if (!value.isBool())
    {
      return false;
    }
    output = value.toBool();
    return true;
  }
  bool size(const QJsonValue& value, std::size_t& output)
  {
    if (!value.isString())
    {
      return false;
    }
    const auto text = value.toString();
    if (text.isEmpty() || std::any_of(text.begin(),text.end(),[](QChar c)
      {
        return c < QLatin1Char('0') || c > QLatin1Char('9');
      }))
    {
      return false;
    }
    bool valid = false;
    const auto parsed = text.toULongLong(&valid);
    if (!valid || parsed > std::numeric_limits<std::size_t>::max())
    {
      return false;
    }
    output = static_cast<std::size_t>(parsed);
    return true;
  }
  template<std::size_t N> Array vector_json(const std::array<double,N>& value)
  {
    Array result;
    for (double component : value)
    {
      result.append(component);
    }
    return result;
  }
  template<std::size_t N> bool vector_read(const QJsonValue& json, std::array<double,N>& output)
  {
    if (!json.isArray() || json.toArray().size() != N)
    {
      return false;
    }
    const auto array = json.toArray();
    for (std::size_t i = 0; i < N; ++i)
    {
      if (!array[static_cast<int>(i)].isDouble() || !std::isfinite(array[static_cast<int>(i)].toDouble()))
      {
        return false;
      }
      output[i] = array[static_cast<int>(i)].toDouble();
    }
    return true;
  }
  Object dataset_json(const SavedDatasetReference& value)
  {
    return {{"kind",q(value.kind)},{"name",q(value.name)},{"model",value.model_id}};
  }
  bool dataset_read(const QJsonValue& json, SavedDatasetReference& value)
  {
    const auto object = json.toObject();
    return json.isObject() && text(object,"kind",value.kind) && text(object,"name",value.name) &&
      integer(object,"model",value.model_id) && value.model_id >= -1;
  }
  Object frame_json(const SavedFrameReference& value)
  {
    return {{"valid",value.valid},{"step",value.step},{"increment",value.increment},{"analysis",value.analysis_type},
      {"mode",value.mode},{"subcase",value.subcase},{"nodal_diameter",value.nodal_diameter},{"time",value.time}};
  }
  bool frame_read(const QJsonValue& json, SavedFrameReference& value)
  {
    const auto object = json.toObject();
    return json.isObject() && boolean(object,"valid",value.valid) && integer(object,"step",value.step) &&
      integer(object,"increment",value.increment) && integer(object,"analysis",value.analysis_type) &&
      integer(object,"mode",value.mode) && integer(object,"subcase",value.subcase) &&
      integer(object,"nodal_diameter",value.nodal_diameter) && number(object,"time",value.time);
  }
  QJsonValue field_json(const std::optional<FieldSelection>& value)
  {
    if (!value)
    {
      return QJsonValue(QJsonValue::Null);
    }
    return Object{{"array",q(value->array_name)},{"association",static_cast<int>(value->association)},
      {"component",q(value->component)},{"magnitude",value->magnitude},{"group",q(value->group_key)}};
  }
  bool field_read(const QJsonValue& json, std::optional<FieldSelection>& output)
  {
    if (json.isNull())
    {
      output.reset();
      return true;
    }
    FieldSelection value;
    int association = -1;
    const auto object = json.toObject();
    if (!json.isObject() || !text(object,"array",value.array_name) || !integer(object,"association",association) ||
        association < 0 || association > 2 || !text(object,"component",value.component) ||
        !boolean(object,"magnitude",value.magnitude) || !text(object,"group",value.group_key))
    {
      return false;
    }
    value.association = static_cast<SimResults::Association>(association);
    output = value;
    return true;
  }
  Object limit_json(const ColorLimit& value)
  {
    return {{"enabled",value.enabled},{"value",value.value},{"color",vector_json(value.color)}};
  }
  bool limit_read(const QJsonValue& json, ColorLimit& value)
  {
    const auto object = json.toObject();
    return json.isObject() && boolean(object,"enabled",value.enabled) && number(object,"value",value.value) &&
      vector_read(object.value("color"),value.color);
  }
  bool records(const Object& object, const char* key, Array& output)
  {
    const auto value = object.value(QLatin1String(key));
    if (!value.isArray() || value.toArray().size() > PersistenceDefaults::maximum_records)
    {
      return false;
    }
    output = value.toArray();
    return true;
  }
}
namespace Postprocessor
{
  QJsonObject project_to_json(const PostprocessorProjectState& project, const std::string& directory)
  {
    const auto& state = project.session;
    Array sources, views, displays, layout, catalogues;
    const QDir base(q(directory));
    for (const auto& item : state.sources)
    {
      const auto& source = item.second;
      Object object{{"id",q(source.id)},{"name",q(source.name)}};
      if (const auto* job = std::get_if<JobSourceReference>(&source.reference))
      {
        object.insert("reference",Object{{"type","job"},{"application",q(job->application_id)},{"project",q(job->project_id)},
          {"job",job->job_id},{"run",q(job->run_identity)},{"identity",q(job->result_identity)}});
      }
      else if (const auto* files = std::get_if<FileSourceReference>(&source.reference))
      {
        Array paths;
        for (const auto& path : files->paths)
        {
          const auto relative = base.relativeFilePath(q(path));
          const bool native = QDir::isAbsolutePath(q(path));
          const bool local = native && !QDir::isAbsolutePath(relative) && relative != ".." && !relative.startsWith("../");
          paths.append(Object{{"path",local ? relative : q(path)},{"relative",local}});
        }
        object.insert("reference",Object{{"type","file"},{"provider",q(files->provider_id)},
          {"identity",q(files->content_identity)},{"paths",paths}});
      }
      else
      {
        const auto& geometry = std::get<CubitGeometryReference>(source.reference);
        Array volumes, surfaces;
        for (int id : geometry.volume_ids)
        {
          volumes.append(id);
        }
        for (int id : geometry.surface_ids)
        {
          surfaces.append(id);
        }
        object.insert("reference",Object{{"type","cubit"},{"model",q(geometry.model_identity)},{"revision",q(geometry.model_revision)},
          {"volumes",volumes},{"surfaces",surfaces}});
      }
      sources.append(object);
    }
    for (const auto& item : state.views)
    {
      const auto& view = item.second;
      Object object{{"id",q(view.id)},{"name",q(view.name)},{"pick_mode",static_cast<int>(view.selection.mode)}};
      object.insert("annotations",annotations_to_json(view.annotations));
      object.insert("plot",view.plot_id ? QJsonValue(q(*view.plot_id)) : QJsonValue(QJsonValue::Null));
      object.insert("plot_active",view.plot_active);
      QJsonValue camera(QJsonValue::Null), table(QJsonValue::Null);
      if (view.camera)
      {
        const auto& value = *view.camera;
        camera = Object{{"position",vector_json(value.position)},{"focal",vector_json(value.focal_point)},
          {"up",vector_json(value.view_up)},{"parallel",value.parallel_projection},{"angle",value.vertical_view_angle_degrees},
          {"scale",value.parallel_scale}};
      }
      if (view.table)
      {
        const auto found = project.table_frames.find(view.id);
        table = Object{{"source",q(view.table->source_id)},{"key",q(view.table->key)},{"field",view.table->field},
          {"frame",frame_json(found == project.table_frames.end() ? SavedFrameReference{} : found->second)}};
      }
      object.insert("camera",camera); object.insert("table",table); views.append(object);
    }
    for (const auto& item : state.representations)
    {
      const auto& display = item.second;
      const auto found = project.bindings.find(display.id);
      const SavedRepresentationBinding binding = found == project.bindings.end() ? SavedRepresentationBinding{} : found->second;
      Array hidden;
      for (const auto& value : binding.hidden)
      {
        hidden.append(dataset_json(value));
      }
      const auto& color = display.coloring;
      const auto& deform = display.deformation;
      Object coloring{{"field",field_json(color.field)},{"palette",static_cast<int>(color.palette)},
        {"range_mode",static_cast<int>(color.range_mode)},{"range",vector_json(color.fixed_range)},
        {"reversed",color.reversed},{"legend",color.legend_visible},{"bands",color.bands ? QJsonValue(static_cast<int>(*color.bands)) : QJsonValue(QJsonValue::Null)},
        {"minimum",limit_json(color.minimum)},{"maximum",limit_json(color.maximum)}};
      Object deformation{{"enabled",deform.enabled},{"field",field_json(deform.field)},{"mode",static_cast<int>(deform.field_mode)},
        {"scale",deform.scale},{"overlay",deform.overlay}};
      displays.append(Object{{"id",q(display.id)},{"name",q(display.name)},{"source",q(display.input.object_id)},
        {"view",q(display.view_id)},{"visible",display.visible},{"frame",frame_json(binding.frame)},{"hidden",hidden},
        {"color_dataset",binding.color_dataset ? QJsonValue(dataset_json(*binding.color_dataset)) : QJsonValue(QJsonValue::Null)},
        {"deformation_dataset",binding.deformation_dataset ? QJsonValue(dataset_json(*binding.deformation_dataset)) : QJsonValue(QJsonValue::Null)},
        {"coloring",coloring},{"deformation",deformation},{"fps",static_cast<int>(display.animation.frames_per_second)},{"loop",display.animation.loop}});
    }
    for (const auto& item : state.layout.nodes)
    {
      Object object{{"id",q(item.second.id)}};
      if (const auto* pane = std::get_if<ViewPane>(&item.second.content))
      {
        object.insert("type","pane"); object.insert("view",q(pane->view_id));
      }
      else
      {
        const auto& split = std::get<ViewSplit>(item.second.content);
        object.insert("type","split"); object.insert("direction",static_cast<int>(split.direction));
        object.insert("ratio",split.ratio); object.insert("first",q(split.first_node_id)); object.insert("second",q(split.second_node_id));
      }
      layout.append(object);
    }
    for (const auto& item : project.catalogues)
    {
      Array datasets;
      for (const auto& data : item.second)
      {
        datasets.append(Object{{"id",QString::number(static_cast<qulonglong>(data.id))},{"name",q(data.name)},
          {"kind",q(data.kind)},{"model",data.model_id},{"element_type",q(data.element_type)}});
      }
      catalogues.append(Object{{"source",q(item.first)},{"datasets",datasets}});
    }
    return {{"plots",plots_to_json(state.plots)},{"active_plot",q(state.active_plot_id)},{"sources",sources},{"views",views},{"representations",displays},{"layout",layout},{"root",q(state.layout.root_node_id)},
      {"active_source",q(state.active_source_id)},{"active_view",q(state.active_view_id)},
      {"active_representation",q(state.active_representation_id)},{"catalogues",catalogues}};
  }

  bool project_from_json(const QJsonObject& json, const std::string& directory, PostprocessorProjectState& output, std::string& error)
  {
    error = "Invalid Postprocessor project definitions.";
    PostprocessorProjectState candidate;
    auto& state = candidate.session;
    state.views.clear(); state.layout.nodes.clear();
    Array sources, views, displays, layout, catalogues;
    if (!records(json,"sources",sources) || !records(json,"views",views) || !records(json,"representations",displays) ||
        !records(json,"layout",layout) || !records(json,"catalogues",catalogues) || !text(json,"root",state.layout.root_node_id) ||
        !text(json,"active_source",state.active_source_id) || !text(json,"active_view",state.active_view_id) ||
        !text(json,"active_representation",state.active_representation_id))
    {
      return false;
    }
    if (!plots_from_json(json.value("plots"),state.plots) ||
        (json.contains("active_plot") && !text(json,"active_plot",state.active_plot_id)))
    {
      return false;
    }
    for (const auto& value : sources)
    {
      if (!value.isObject())
      {
        return false;
      }
      const auto object = value.toObject();
      DataSource source;
      if (!text(object,"id",source.id) || !text(object,"name",source.name) || !object.value("reference").isObject())
      {
        return false;
      }
      const auto reference = object.value("reference").toObject();
      std::string type;
      if (!text(reference,"type",type))
      {
        return false;
      }
      if (type == "job")
      {
        JobSourceReference job;
        if (!text(reference,"application",job.application_id) || !text(reference,"project",job.project_id) ||
            !integer(reference,"job",job.job_id) || !text(reference,"run",job.run_identity) || !text(reference,"identity",job.result_identity) ||
            job.application_id.empty() || job.job_id <= 0)
        {
          return false;
        }
        source.reference = job;
      }
      else if (type == "file")
      {
        FileSourceReference files;
        Array paths;
        if (!text(reference,"provider",files.provider_id) || files.provider_id.empty() || !text(reference,"identity",files.content_identity) ||
            !records(reference,"paths",paths) || paths.empty())
        {
          return false;
        }
        for (const auto& path : paths)
        {
          std::string name;
          bool relative = false;
          if (!path.isObject() || !text(path.toObject(),"path",name) || name.empty() || !boolean(path.toObject(),"relative",relative) ||
              (relative && (QDir::isAbsolutePath(q(name)) || foreign_absolute(q(name)))) ||
              (!relative && !QDir::isAbsolutePath(q(name)) && !foreign_absolute(q(name))))
          {
            return false;
          }
          files.paths.push_back(relative || QDir::isAbsolutePath(q(name)) ?
            QDir(q(directory)).absoluteFilePath(q(name)).toUtf8().toStdString() : name);
        }
        source.reference = files;
      }
      else if (type == "cubit")
      {
        CubitGeometryReference geometry;
        Array volumes, surfaces;
        if (!text(reference,"model",geometry.model_identity) || !text(reference,"revision",geometry.model_revision) ||
            !records(reference,"volumes",volumes) || !records(reference,"surfaces",surfaces))
        {
          return false;
        }
        for (const auto& id : volumes)
        {
          int number = 0;
          if (!integer(Object{{"id",id}},"id",number) || number <= 0)
          {
            return false;
          }
          geometry.volume_ids.push_back(number);
        }
        for (const auto& id : surfaces)
        {
          int number = 0;
          if (!integer(Object{{"id",id}},"id",number) || number <= 0)
          {
            return false;
          }
          geometry.surface_ids.push_back(number);
        }
        source.reference = geometry;
      }
      else
      {
        return false;
      }
      if (!state.sources.emplace(source.id,std::move(source)).second)
      {
        return false;
      }
    }
    for (const auto& value : views)
    {
      if (!value.isObject())
      {
        return false;
      }
      const auto object = value.toObject();
      ViewDefinition view;
      int mode = -1;
      if (!text(object,"id",view.id) || !text(object,"name",view.name) || !integer(object,"pick_mode",mode) || mode < 0 || mode > 2)
      {
        return false;
      }
      view.selection.mode = static_cast<EntityKind>(mode);
      if (object.contains("plot_active") && !boolean(object,"plot_active",view.plot_active))
      {
        return false;
      }
      if (object.contains("plot") && !object.value("plot").isNull())
      {
        std::string id;
        if (!text(object,"plot",id) || state.plots.count(id) == 0)
        {
          return false;
        }
        view.plot_id = id;
      }
      if (!annotations_from_json(object.value("annotations"),view.annotations))
      {
        return false;
      }
      for (const auto& label : view.annotations.labels)
      {
        if (state.sources.count(label.second.entity.source_id) == 0)
        {
          return false;
        }
      }
      if (!object.value("camera").isNull())
      {
        if (!object.value("camera").isObject())
        {
          return false;
        }
        const auto camera = object.value("camera").toObject();
        CameraDefinition definition;
        if (!vector_read(camera.value("position"),definition.position) || !vector_read(camera.value("focal"),definition.focal_point) ||
            !vector_read(camera.value("up"),definition.view_up) || !boolean(camera,"parallel",definition.parallel_projection) ||
            !number(camera,"angle",definition.vertical_view_angle_degrees) || !number(camera,"scale",definition.parallel_scale))
        {
          return false;
        }
        view.camera = definition;
      }
      view.fit_pending = !view.camera.has_value();
      if (!object.value("table").isNull())
      {
        if (!object.value("table").isObject())
        {
          return false;
        }
        const auto table = object.value("table").toObject();
        TableReference reference;
        SavedFrameReference frame;
        if (!text(table,"source",reference.source_id) || !text(table,"key",reference.key) || !boolean(table,"field",reference.field) ||
            !frame_read(table.value("frame"),frame))
        {
          return false;
        }
        view.table = reference;
        candidate.table_frames.emplace(view.id,frame);
      }
      if (!state.views.emplace(view.id,std::move(view)).second)
      {
        return false;
      }
    }
    for (const auto& value : displays)
    {
      if (!value.isObject())
      {
        return false;
      }
      const auto object = value.toObject();
      Representation display;
      SavedRepresentationBinding binding;
      Array hidden;
      if (!text(object,"id",display.id) || !text(object,"name",display.name) || !text(object,"source",display.input.object_id) ||
          !text(object,"view",display.view_id) || !boolean(object,"visible",display.visible) || !frame_read(object.value("frame"),binding.frame) ||
          !records(object,"hidden",hidden))
      {
        return false;
      }
      for (const auto& item : hidden)
      {
        SavedDatasetReference data;
        if (!dataset_read(item,data))
        {
          return false;
        }
        binding.hidden.push_back(data);
      }
      for (const auto* key : {"color_dataset","deformation_dataset"})
      {
        if (!object.value(QLatin1String(key)).isNull())
        {
          SavedDatasetReference data;
          if (!dataset_read(object.value(QLatin1String(key)),data))
          {
            return false;
          }
          (std::string(key) == "color_dataset" ? binding.color_dataset : binding.deformation_dataset) = data;
        }
      }
      if (!object.value("coloring").isObject() || !object.value("deformation").isObject())
      {
        return false;
      }
      const auto color = object.value("coloring").toObject();
      int palette = -1, range = -1;
      if (!field_read(color.value("field"),display.coloring.field) || !integer(color,"palette",palette) || palette < 0 || palette > 4 ||
          !integer(color,"range_mode",range) || range < 0 || range > 3 || !vector_read(color.value("range"),display.coloring.fixed_range) ||
          !boolean(color,"reversed",display.coloring.reversed) || !boolean(color,"legend",display.coloring.legend_visible) ||
          !limit_read(color.value("minimum"),display.coloring.minimum) || !limit_read(color.value("maximum"),display.coloring.maximum))
      {
        return false;
      }
      display.coloring.palette = static_cast<ColorPalette>(palette); display.coloring.range_mode = static_cast<ColorRangeMode>(range);
      if (!color.value("bands").isNull())
      {
        int bands = 0;
        if (!integer(color,"bands",bands) || bands < 2 || bands > static_cast<int>(ColoringDefaults::maximum_bands))
        {
          return false;
        }
        display.coloring.bands = static_cast<unsigned int>(bands);
      }
      const auto deform = object.value("deformation").toObject();
      int deformMode = -1, fps = 0;
      if (!boolean(deform,"enabled",display.deformation.enabled) || !field_read(deform.value("field"),display.deformation.field) ||
          !integer(deform,"mode",deformMode) || deformMode < 0 || deformMode > 1 || !number(deform,"scale",display.deformation.scale) ||
          !boolean(deform,"overlay",display.deformation.overlay) || !integer(object,"fps",fps) || fps < 1 || fps > 120 ||
          !boolean(object,"loop",display.animation.loop))
      {
        return false;
      }
      display.deformation.field_mode = static_cast<DeformationFieldMode>(deformMode);
      display.animation.frames_per_second = static_cast<unsigned int>(fps);
      display.animation.playing = false;
      candidate.bindings.emplace(display.id,std::move(binding));
      if (!state.representations.emplace(display.id,std::move(display)).second)
      {
        return false;
      }
    }
    for (const auto& value : layout)
    {
      if (!value.isObject())
      {
        return false;
      }
      const auto object = value.toObject();
      LayoutNode node;
      std::string type;
      if (!text(object,"id",node.id) || !text(object,"type",type))
      {
        return false;
      }
      if (type == "pane")
      {
        ViewPane pane;
        if (!text(object,"view",pane.view_id))
        {
          return false;
        }
        node.content = pane;
      }
      else if (type == "split")
      {
        ViewSplit split;
        int direction = -1;
        if (!integer(object,"direction",direction) || direction < 0 || direction > 1 || !number(object,"ratio",split.ratio) ||
            !text(object,"first",split.first_node_id) || !text(object,"second",split.second_node_id))
        {
          return false;
        }
        split.direction = static_cast<SplitDirection>(direction); node.content = split;
      }
      else
      {
        return false;
      }
      if (!state.layout.nodes.emplace(node.id,std::move(node)).second)
      {
        return false;
      }
    }
    for (const auto& value : catalogues)
    {
      if (!value.isObject())
      {
        return false;
      }
      std::string source;
      Array datasets;
      if (!text(value.toObject(),"source",source) || !records(value.toObject(),"datasets",datasets) ||
          candidate.catalogues.count(source))
      {
        return false;
      }
      auto& output = candidate.catalogues[source];
      std::set<std::size_t> seen;
      for (const auto& item : datasets)
      {
        if (!item.isObject())
        {
          return false;
        }
        const auto object = item.toObject();
        SimResults::Dataset data;
        if (!size(object.value("id"),data.id) || !text(object,"name",data.name) || !text(object,"kind",data.kind) ||
            !integer(object,"model",data.model_id) || data.model_id < -1 || !text(object,"element_type",data.element_type) ||
            !seen.insert(data.id).second)
        {
          return false;
        }
        output.push_back(std::move(data));
      }
    }
    if (!validate_project(candidate,error))
    {
      return false;
    }
    output = std::move(candidate);
    error.clear();
    return true;
  }
  bool validate_project(const PostprocessorProjectState& project, std::string& error)
  {
    error = "Invalid Postprocessor project definitions or references.";
    const auto& state = project.session;
    if (state.views.empty() || state.views.size() > PersistenceDefaults::maximum_records || state.sources.size() > PersistenceDefaults::maximum_records ||
        state.views.count(state.active_view_id) == 0 || (!state.active_source_id.empty() && state.sources.count(state.active_source_id) == 0) ||
        (!state.active_representation_id.empty() && state.representations.count(state.active_representation_id) == 0))
    {
      return false;
    }
    if (!state.active_representation_id.empty())
    {
      const auto& active = state.representations.at(state.active_representation_id);
      if (active.input.object_id != state.active_source_id || active.view_id != state.active_view_id)
      {
        return false;
      }
    }
    if (state.views.at(state.active_view_id).table &&
        state.views.at(state.active_view_id).table->source_id != state.active_source_id)
    {
      return false;
    }
    std::set<std::string> names;
    for (const auto& item : state.sources)
    {
      if (item.first.empty() || item.first != item.second.id || item.second.name.find_first_not_of(" \t\r\n") == std::string::npos ||
          !names.insert(item.second.name).second)
      {
        return false;
      }
    }
    for (const auto& item : state.views)
    {
      if (item.first.empty() || item.first != item.second.id || (item.second.camera && !CameraOperations::valid(*item.second.camera)) ||
          (item.second.table && (state.sources.count(item.second.table->source_id) == 0 || item.second.table->key.empty())))
      {
        return false;
      }
    }
    std::map<std::string,PlotDefinition> validatedPlots;
    if (!plots_from_json(plots_to_json(state.plots),validatedPlots))
    {
      return false;
    }
    if (state.plots.size() > PlotDefaults::maximum_plots ||
        (!state.active_plot_id.empty() && state.plots.count(state.active_plot_id) == 0))
    {
      return false;
    }
    for (const auto& entry : state.plots)
    {
      if (entry.first != entry.second.id || !valid_plot_name(entry.second.name) || !valid_plot_axes(entry.second.axes) ||
          entry.second.curves.size() > PlotDefaults::maximum_curves)
      {
        return false;
      }
      for (const auto& curve : entry.second.curves)
      {
        if (!valid_plot_name(curve.name) || state.sources.count(plot_curve_source(curve)) == 0)
        {
          return false;
        }
      }
    }
    for (const auto& entry : state.views)
    {
      if ((entry.second.plot_id && state.plots.count(*entry.second.plot_id) == 0) || (entry.second.plot_active && !entry.second.plot_id))
      {
        return false;
      }
    }
    std::set<std::pair<std::string,std::string>> connections;
    for (const auto& item : state.views)
    {
      const auto& annotations = item.second.annotations;
      if (annotations.labels.size() > PersistenceDefaults::maximum_records ||
          !valid_annotation_position(annotations.minimum_position) || !valid_annotation_position(annotations.maximum_position))
      {
        return false;
      }
      for (const auto& entry : annotations.labels)
      {
        const auto& label = entry.second;
        if (entry.first != label.name || !valid_annotation_name(label.name) || !valid_annotation_position(label.position) ||
            state.sources.count(label.entity.source_id) == 0 || label.entity.entity_id < 0)
        {
          return false;
        }
      }
    }
    for (const auto& item : state.representations)
    {
      const auto& value = item.second;
      if (item.first.empty() || item.first != value.id || value.input.owner != OutputOwner::Source || value.input.port != 0 ||
          state.sources.count(value.input.object_id) == 0 || state.views.count(value.view_id) == 0 ||
          !connections.emplace(value.input.object_id,value.view_id).second || project.bindings.count(value.id) == 0 ||
          !valid_coloring(value.coloring,error) || !valid_deformation(value.deformation,error))
      {
        return false;
      }
    }
    for (const auto& item : project.bindings)
    {
      if (state.representations.count(item.first) == 0 || !std::isfinite(item.second.frame.time) ||
          item.second.hidden.size() > PersistenceDefaults::maximum_records)
      {
        return false;
      }
    }
    for (const auto& item : state.views)
    {
      if (item.second.table && (project.table_frames.count(item.first) == 0 ||
          !std::isfinite(project.table_frames.at(item.first).time)))
      {
        return false;
      }
    }
    for (const auto& item : project.catalogues)
    {
      if (state.sources.count(item.first) == 0)
      {
        return false;
      }
    }
    std::vector<std::string> pending{state.layout.root_node_id};
    std::set<std::string> visited, views;
    while (!pending.empty())
    {
      const auto id = pending.back(); pending.pop_back();
      const auto found = state.layout.nodes.find(id);
      if (found == state.layout.nodes.end() || !visited.insert(id).second || found->second.id != id)
      {
        return false;
      }
      if (const auto* pane = std::get_if<ViewPane>(&found->second.content))
      {
        if (state.views.count(pane->view_id) == 0 || !views.insert(pane->view_id).second)
        {
          return false;
        }
      }
      else
      {
        const auto& split = std::get<ViewSplit>(found->second.content);
        if (!std::isfinite(split.ratio) || split.ratio <= 0 || split.ratio >= 1 || split.first_node_id == split.second_node_id)
        {
          return false;
        }
        pending.push_back(split.first_node_id); pending.push_back(split.second_node_id);
      }
    }
    if (visited.size() != state.layout.nodes.size() || views.size() != state.views.size())
    {
      return false;
    }
    error.clear();
    return true;
  }
}
