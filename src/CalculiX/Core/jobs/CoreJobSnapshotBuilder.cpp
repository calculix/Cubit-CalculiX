#include "CoreJobSnapshotBuilder.hpp"
#include "Core/drawing/CalculiXDrawInterface.hpp"
#include "Core/export/CalculiXExportInterface.hpp"
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStringList>
#include <QUuid>
#include <algorithm>
#include <cmath>
#include <exception>
#include <functional>
#include <limits>
#include <set>

namespace
{
void token(QCryptographicHash& hash, const std::string& value)
{
  hash.addData(QByteArray::number(static_cast<qlonglong>(value.size())) + ':');
  hash.addData(value.data(), static_cast<int>(value.size()));
}
void number(QCryptographicHash& hash, double value)
{
  token(hash, QString::number(value, 'g', 17).toStdString());
}
void geometryHash(QCryptographicHash& hash, const DrawingGeometry& geometry)
{
  token(hash, std::to_string(geometry.primitives.size()));
  for (const auto& primitive : geometry.primitives)
  {
    token(hash, std::to_string(static_cast<int>(primitive.kind)));
    token(hash, primitive.color);
    token(hash, primitive.text);
    token(hash, std::to_string(primitive.points.size()));
    for (const auto& point : primitive.points)
    {
      for (double value : point)
      {
        number(hash, value);
      }
    }
  }
}
void sceneHash(QCryptographicHash& hash, const CalculiXJobSetupScene& scene)
{
  token(hash, std::to_string(scene.stepId));
  token(hash, std::to_string(scene.ordinal));
  token(hash, scene.name);
  token(hash, scene.available ? "ready" : "unavailable");
  token(hash, scene.unavailableReason);
  token(hash, std::to_string(scene.symbols.size()));
  for (const auto& item : scene.symbols)
  {
    token(hash, std::to_string(static_cast<int>(item.first.feature)));
    token(hash, std::to_string(item.first.id));
    geometryHash(hash, item.second);
  }
  token(hash, std::to_string(scene.labels.size()));
  for (const auto& item : scene.labels)
  {
    const auto& label = item.second;
    token(hash, std::to_string(static_cast<int>(item.first.feature)));
    token(hash, std::to_string(item.first.id));
    token(hash, std::to_string(label.stepId));
    number(hash, label.symbolSize);
    token(hash, label.typeName);
    token(hash, label.name);
    token(hash, label.amplitude);
    token(hash, label.secondaryAmplitude);
    token(hash, std::to_string(label.values.size()));
    for (const auto& value : label.values)
    {
      token(hash, value.first);
      token(hash, value.second);
    }
    token(hash, std::to_string(label.anchors.size()));
    for (const auto& anchor : label.anchors)
    {
      for (double value : anchor)
      {
        number(hash, value);
      }
    }
  }
}

// Reads exactly the emitted node/connectivity records, never the live Cubit
// mesh. Corner faces approximate high-order geometry, as in normal captures.
bool emittedMesh(const CalculiXJobInputBundle& input, DrawingGeometry& output, double& size, int& stepCount, std::string& error)
{
  std::map<int, std::array<double, 3>> nodes;
  std::vector<std::pair<std::string, std::vector<int>>> elements;
  const std::map<std::string, std::pair<int, std::string>> types = {
    {"C3D4", {4, "tet"}}, {"C3D10", {10, "tet"}}, {"C3D6", {6, "wedge"}}, {"C3D15", {15, "wedge"}},
    {"C3D8", {8, "hex"}}, {"C3D8R", {8, "hex"}}, {"C3D8I", {8, "hex"}}, {"C3D20", {20, "hex"}}, {"C3D20R", {20, "hex"}},
    {"S3", {3, "tri"}}, {"S6", {6, "tri"}}, {"S4", {4, "quad"}}, {"S4R", {4, "quad"}},
    {"S8", {8, "quad"}}, {"S8R", {8, "quad"}}, {"CPS3", {3, "tri"}}, {"CPE3", {3, "tri"}},
    {"CPS4", {4, "quad"}}, {"CPE4", {4, "quad"}}, {"CPS4R", {4, "quad"}}, {"CPE4R", {4, "quad"}},
    {"B31", {2, "edge"}}, {"B32", {3, "edge"}}, {"T3D2", {2, "edge"}}};
  std::set<int> elementIds;
  std::string card, shape;
  int expected = 0;
  std::vector<int> connectivity;
  std::function<bool(const std::string&)> parse = [&](const std::string& file)
  {
    for (auto line : QString::fromUtf8(input.files.at(file)).split('\n'))
    {
      line = line.trimmed();
      if (line.isEmpty() || line.startsWith("**"))
      {
        continue;
      }
      if (line.startsWith('*'))
      {
        if (!connectivity.empty())
        {
          error = "Incomplete exported element connectivity.";
          return false;
        }
        card = line.mid(1).section(',', 0, 0).trimmed().toUpper().toStdString();
        if (card == "STEP")
        {
          ++stepCount;
        }
        if (card == "INCLUDE")
        {
          const auto match = QRegularExpression("(?:INPUT|FILE)\\s*=\\s*(.+)$", QRegularExpression::CaseInsensitiveOption).match(line);
          auto literal = match.captured(1).trimmed();
          if ((literal.startsWith('"') && literal.endsWith('"')) || (literal.startsWith('\'') && literal.endsWith('\'')))
          {
            literal = literal.mid(1, literal.size() - 2);
          }
          if (!parse(input.includes.at(file).at(literal.toStdString())))
          {
            return false;
          }
          continue;
        }
        if (card == "TRANSFORM" || card == "SYSTEM" || card == "MODEL CHANGE" || card == "RESTART" ||
            card == "SUBMODEL" || card == "IMPORT" || card == "NGEN" || card == "NFILL" || card == "ELGEN" ||
            card == "ELCOPY" || card == "NCOPY" || card == "NMAP")
        {
          error = "Setup snapshots do not yet represent exported *" + card + " definitions.";
          return false;
        }
        if (card == "ELEMENT")
        {
          const auto match = QRegularExpression("(?:^|,)\\s*TYPE\\s*=\\s*([^,]+)", QRegularExpression::CaseInsensitiveOption).match(line);
          const auto type = types.find(match.captured(1).trimmed().toUpper().toStdString());
          if (type == types.end())
          {
            error = "Unsupported exported snapshot element type: " + match.captured(1).toStdString();
            return false;
          }
          expected = type->second.first;
          shape = type->second.second;
        }
        continue;
      }
      const auto fields = line.split(',');
      if (card == "NODE")
      {
        bool ok = false;
        const int id = fields.front().trimmed().toInt(&ok);
        if (!ok || id <= 0 || fields.size() != 4 || nodes.count(id) != 0)
        {
          error = "Invalid or repeated exported node.";
          return false;
        }
        std::array<double, 3> point;
        for (int axis = 0; axis < 3; ++axis)
        {
          point[axis] = fields[axis + 1].trimmed().toDouble(&ok);
          if (!ok || !std::isfinite(point[axis]))
          {
            error = "Invalid exported node coordinates.";
            return false;
          }
        }
        nodes.emplace(id, point);
      }
      else if (card == "ELEMENT")
      {
        int start = 0;
        if (connectivity.empty())
        {
          bool ok = false;
          const int id = fields.front().trimmed().toInt(&ok);
          if (!ok || id <= 0 || !elementIds.insert(id).second)
          {
            error = "Invalid exported element ID.";
            return false;
          }
          start = 1;
        }
        for (int index = start; index < fields.size(); ++index)
        {
          if (fields[index].trimmed().isEmpty() && index == fields.size() - 1)
          {
            continue;
          }
          bool ok = false;
          const int node = fields[index].trimmed().toInt(&ok);
          if (!ok || node <= 0)
          {
            error = "Invalid exported connectivity node.";
            return false;
          }
          connectivity.push_back(node);
        }
        if (connectivity.size() > static_cast<std::size_t>(expected))
        {
          error = "Too many exported connectivity nodes.";
          return false;
        }
        if (connectivity.size() == static_cast<std::size_t>(expected))
        {
          elements.push_back({shape, connectivity});
          connectivity.clear();
        }
      }
    }
    return true;
  };
  if (!parse(input.rootFile) || !connectivity.empty())
  {
    if (error.empty())
    {
      error = "Incomplete exported mesh.";
    }
    return false;
  }
  DrawingGeometry geometry;
  const double largest = (std::numeric_limits<double>::max)();
  const double lowest = (std::numeric_limits<double>::lowest)();
  std::array<double, 3> minimum = {largest, largest, largest}, maximum = {lowest, lowest, lowest};
  for (const auto& element : elements)
  {
    std::vector<std::vector<int>> faces;
    if (element.first == "hex")
    {
      faces = {{0, 3, 2, 1}, {4, 5, 6, 7}, {0, 1, 5, 4}, {1, 2, 6, 5}, {2, 3, 7, 6}, {3, 0, 4, 7}};
    }
    else if (element.first == "tet")
    {
      faces = {{0, 2, 1}, {0, 1, 3}, {1, 2, 3}, {2, 0, 3}};
    }
    else if (element.first == "wedge")
    {
      faces = {{0, 2, 1}, {3, 4, 5}, {0, 1, 4, 3}, {1, 2, 5, 4}, {2, 0, 3, 5}};
    }
    else
    {
      faces = element.first == "tri" ? std::vector<std::vector<int>>{{0, 1, 2}} :
        element.first == "quad" ? std::vector<std::vector<int>>{{0, 1, 2, 3}} : std::vector<std::vector<int>>{{0, 1}};
    }
    for (const auto& face : faces)
    {
      std::vector<std::vector<double>> points;
      for (int corner : face)
      {
        const auto found = nodes.find(element.second[corner]);
        if (found == nodes.end())
        {
          error = "Exported element refers to a missing node.";
          return false;
        }
        points.push_back({found->second[0], found->second[1], found->second[2]});
        for (int axis = 0; axis < 3; ++axis)
        {
          minimum[axis] = (std::min)(minimum[axis], found->second[axis]);
          maximum[axis] = (std::max)(maximum[axis], found->second[axis]);
        }
      }
      const bool valid = points.size() == 2 ? geometry.line(points[0], points[1], "lightgray") : geometry.polygon(points, "lightgray");
      if (!valid || geometry.primitives.size() > 1000000)
      {
        error = "Snapshot mesh is invalid or exceeds one million primitives.";
        return false;
      }
    }
  }
  const double extent = (std::max)({maximum[0] - minimum[0], maximum[1] - minimum[1], maximum[2] - minimum[2]});
  if (geometry.primitives.empty() || !std::isfinite(extent) || extent <= 0)
  {
    error = "The exported input has no supported nondegenerate mesh.";
    return false;
  }
  output = std::move(geometry);
  size = extent * 0.1;
  return true;
}
}

CoreJobSnapshotBuilder::CoreJobSnapshotBuilder() = default;

std::string CoreJobSnapshotBuilder::inputFingerprint(const CalculiXJobInputBundle& input)
{
  QCryptographicHash hash(QCryptographicHash::Sha256);
  token(hash, "calculix.input.bundle.1");
  token(hash, input.rootFile);
  token(hash, std::to_string(input.order.size()));
  for (const auto& path : input.order)
  {
    token(hash, path);
  }
  token(hash, std::to_string(input.files.size()));
  for (const auto& file : input.files)
  {
    token(hash, file.first);
    token(hash, QCryptographicHash::hash(file.second, QCryptographicHash::Sha256).toHex().toStdString());
  }
  token(hash, std::to_string(input.includes.size()));
  for (const auto& parent : input.includes)
  {
    token(hash, parent.first);
    token(hash, std::to_string(parent.second.size()));
    for (const auto& edge : parent.second)
    {
      token(hash, parent.first);
      token(hash, edge.first);
      token(hash, edge.second);
    }
  }
  return hash.result().toHex().toStdString();
}

std::string CoreJobSnapshotBuilder::setupFingerprint(const CalculiXJobSetupSnapshot& setup)
{
  QCryptographicHash hash(QCryptographicHash::Sha256);
  token(hash, "calculix.setup.1");
  token(hash, setup.id);
  number(hash, setup.symbolSize);
  geometryHash(hash, setup.model);
  sceneHash(hash, setup.modelDefinitions);
  token(hash, std::to_string(setup.steps.size()));
  for (const auto& step : setup.steps)
  {
    sceneHash(hash, step);
  }
  return hash.result().toHex().toStdString();
}

bool CoreJobSnapshotBuilder::captureInput(const std::string& filename, CalculiXJobInputBundle& output, std::string& error)
{
  CalculiXJobInputBundle candidate;
  std::set<std::string> stack;
  qint64 bytes = 0;
  std::function<bool(QString, int)> read = [&](QString path, int depth)
  {
    path = QFileInfo(path).canonicalFilePath();
    const auto key = path.toStdString();
    if (path.isEmpty() || depth > 32 || stack.count(key) != 0)
    {
      error = "Missing input, excessive INCLUDE depth or cyclic dependency.";
      return false;
    }
    if (candidate.files.count(key) != 0)
    {
      return true;
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly) || file.size() < 0 || file.size() > 128 * 1024 * 1024 - bytes || candidate.files.size() >= 4096)
    {
      error = "Cannot archive job input, or input exceeds the 128 MiB/4096-file limit.";
      return false;
    }
    const auto data = file.readAll();
    if (file.error() != QFileDevice::NoError)
    {
      error = "Cannot read job input bytes.";
      return false;
    }
    bytes += data.size();
    if (candidate.rootFile.empty())
    {
      candidate.rootFile = key;
    }
    candidate.order.push_back(key);
    candidate.files.emplace(key, data);
    candidate.fingerprints[key] = QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex().toStdString();
    stack.insert(key);
    for (auto line : QString::fromUtf8(data).split('\n'))
    {
      line = line.trimmed();
      if (!line.startsWith('*') || line.startsWith("**"))
      {
        continue;
      }
      const auto keyword = line.mid(1).section(',', 0, 0).trimmed().toUpper();
      const auto match = QRegularExpression("(?:INPUT|FILE)\\s*=\\s*(.+)$", QRegularExpression::CaseInsensitiveOption).match(line);
      if (keyword != "INCLUDE")
      {
        if (match.hasMatch())
        {
          error = "External input references outside *INCLUDE are not yet archived.";
          return false;
        }
        continue;
      }
      auto literal = match.captured(1).trimmed();
      if ((literal.startsWith('"') && literal.endsWith('"')) || (literal.startsWith('\'') && literal.endsWith('\'')))
      {
        literal = literal.mid(1, literal.size() - 2);
      }
      if (!match.hasMatch() || literal.isEmpty() || literal.contains('<'))
      {
        error = "Unsupported or parameterized INCLUDE path.";
        return false;
      }
      const auto child = QFileInfo(QDir(QFileInfo(path).absolutePath()).absoluteFilePath(literal)).canonicalFilePath();
      candidate.includes[key][literal.toStdString()] = child.toStdString();
      if (!read(child, depth + 1))
      {
        return false;
      }
    }
    stack.erase(key);
    return true;
  };
  if (!read(QString::fromStdString(filename), 0))
  {
    return false;
  }
  candidate.manifestFingerprint = inputFingerprint(candidate);
  output = std::move(candidate);
  error.clear();
  return true;
}

bool CoreJobSnapshotBuilder::prepare(int jobId, const std::string& filename,
  CalculiXJobRunRecord& output, std::string& error)
{
  try
  {
    CalculiXJobRunRecord candidate;
    candidate.jobId = jobId;
    if (!captureInput(filename, candidate.input, error))
    {
      return false;
    }
    if (!CalculiXExportInterface().get_customline_ids().empty())
    {
      error = "Setup snapshots are unavailable for exports containing custom input definitions.";
      return false;
    }
    auto setup = std::make_shared<CalculiXJobSetupSnapshot>();
    setup->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    int emittedSteps = 0;
    if (!emittedMesh(candidate.input, setup->model, setup->symbolSize, emittedSteps, error))
    {
      return false;
    }
    setup->modelDefinitions.unavailableReason = "A job setup picture requires a frozen analysis step; unassigned model definitions are not substituted.";
    if (!CalculiXDrawInterface().freeze_setup_scenes(setup->symbolSize, setup->steps, error))
    {
      return false;
    }
    if (setup->steps.size() != static_cast<std::size_t>(emittedSteps))
    {
      error = "Frozen step order does not match the emitted analysis steps.";
      return false;
    }
    setup->fingerprint = setupFingerprint(*setup);
    candidate.setup = std::move(setup);
    output = std::move(candidate);
    error.clear();
    return true;
  }
  catch (const std::exception& exception)
  {
    error = std::string("Cannot freeze job setup: ") + exception.what();
    return false;
  }
}

bool CoreJobSnapshotBuilder::validate(const CalculiXJobRunRecord& record, std::string& error)
{
  const auto fail = [&](const char* message)
  {
    error = message;
    return false;
  };
  if (record.jobId <= 0 || record.launchId.empty() || !QDateTime::fromString(QString::fromStdString(record.launchedAt), Qt::ISODate).isValid())
  {
    return fail("Invalid job launch identity or timestamp.");
  }
  const auto& input = record.input;
  if (input.files.empty())
  {
    if (record.setup || record.setupUnavailableReason.empty() || !input.order.empty() || !input.fingerprints.empty() || !input.includes.empty() ||
        !input.rootFile.empty() || !input.manifestFingerprint.empty())
    {
      return fail("A setup snapshot requires archived input; unavailable archives must be empty.");
    }
    error.clear();
    return true;
  }
  if (input.order.empty() || input.order.front() != input.rootFile || input.order.size() != input.files.size() ||
      std::set<std::string>(input.order.begin(), input.order.end()).size() != input.order.size() || input.fingerprints.size() != input.files.size())
  {
    return fail("Invalid archived input manifest.");
  }
  qint64 bytes = 0;
  for (const auto& path : input.order)
  {
    const auto file = input.files.find(path);
    const auto digest = input.fingerprints.find(path);
    if (file == input.files.end() || digest == input.fingerprints.end() ||
        QCryptographicHash::hash(file->second, QCryptographicHash::Sha256).toHex().toStdString() != digest->second)
    {
      return fail("Archived input fingerprint mismatch.");
    }
    bytes += file->second.size();
  }
  if (bytes > 128 * 1024 * 1024 || input.files.size() > 4096 || inputFingerprint(input) != input.manifestFingerprint)
  {
    return fail("Invalid input bundle digest or resource limit.");
  }
  for (const auto& parent : input.includes)
  {
    if (parent.second.empty())
    {
      return fail("Archived INCLUDE graphs must not contain empty parent entries.");
    }
    for (const auto& edge : parent.second)
    {
      if (input.files.count(parent.first) == 0 || edge.first.empty() || input.files.count(edge.second) == 0)
      {
        return fail("Invalid archived INCLUDE reference.");
      }
    }
  }
  if (!record.setup)
  {
    error.clear();
    return true;
  }
  const auto& setup = *record.setup;
  if (setup.id.empty() || !std::isfinite(setup.symbolSize) || setup.symbolSize <= 0 || setup.model.primitives.empty() || setup.steps.size() > 256)
  {
    return fail("Invalid setup snapshot metadata.");
  }
  if (setup.modelDefinitions.available || !setup.modelDefinitions.symbols.empty() || !setup.modelDefinitions.labels.empty())
  {
    return fail("Job snapshots require step-scoped conditions, not unassigned model definitions.");
  }
  std::size_t scalars = 0;
  const auto geometry = [&](const DrawingGeometry& geometry)
  {
    if (geometry.primitives.size() > 1000000)
    {
      return false;
    }
    for (const auto& primitive : geometry.primitives)
    {
      const auto count = primitive.points.size();
      if ((primitive.kind == DrawingPrimitiveKind::Point && count != 1) || (primitive.kind == DrawingPrimitiveKind::Line && count != 2) ||
          (primitive.kind == DrawingPrimitiveKind::Polygon && count < 3) || (primitive.kind == DrawingPrimitiveKind::Label && count != 1) ||
          static_cast<int>(primitive.kind) < 0 || static_cast<int>(primitive.kind) > 3 || primitive.color.empty() || primitive.text.size() > 65536)
      {
        return false;
      }
      for (const auto& point : primitive.points)
      {
        scalars += point.size();
        if (point.size() != 3 || scalars > 24000000)
        {
          return false;
        }
        for (double value : point)
        {
          if (!std::isfinite(value))
          {
            return false;
          }
        }
      }
    }
    return true;
  };
  if (!geometry(setup.model))
  {
    return fail("Invalid frozen model geometry.");
  }
  std::set<int> stepIds;
  int ordinal = 0;
  for (const auto& scene : setup.steps)
  {
    if (scene.stepId <= 0 || !stepIds.insert(scene.stepId).second || scene.ordinal != ++ordinal || (!scene.available && scene.unavailableReason.empty()))
    {
      return fail("Invalid frozen step identity or coverage.");
    }
    for (const auto& entity : scene.symbols)
    {
      if (entity.first.id <= 0 || static_cast<int>(entity.first.feature) < 0 || static_cast<int>(entity.first.feature) > 11 || !geometry(entity.second))
      {
        return fail("Invalid frozen condition geometry.");
      }
    }
    for (const auto& item : scene.labels)
    {
      const auto& label = item.second;
      if (scene.symbols.count(item.first) == 0 || label.entity.id != item.first.id || label.entity.feature != item.first.feature ||
          label.stepId != scene.stepId || !std::isfinite(label.symbolSize) || label.symbolSize <= 0 || label.anchors.size() > 1000000)
      {
        return fail("Invalid frozen label ownership.");
      }
      for (const auto& point : label.anchors)
      {
        for (double value : point)
        {
          if (!std::isfinite(value))
          {
            return fail("Invalid frozen label anchor.");
          }
        }
      }
    }
  }
  if (setupFingerprint(setup) != setup.fingerprint)
  {
    return fail("Setup snapshot fingerprint mismatch.");
  }
  error.clear();
  return true;
}
