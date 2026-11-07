#include "PostprocessorUserOptions.hpp"
#include "Framework/Interfaces/UserOptionsInterface.hpp"

#include <QDir>
#include <QFileInfo>
#include <exception>

namespace
{
bool validPath(const UserOptionValue& value)
{
  const auto* path = std::get_if<std::string>(&value);
  if (path == nullptr || path->empty() || path->find('\0') != std::string::npos || path->find_first_of("\"\r\n") != std::string::npos)
  {
    return false;
  }
  const QString normalized = QDir::cleanPath(QDir::fromNativeSeparators(QString::fromUtf8(path->c_str())));
  return QFileInfo(normalized).isAbsolute() ||
    (normalized != ".." && !normalized.startsWith("../"));
}

QString resolvedPath(const std::string& directory, const std::string& path)
{
  return QDir::cleanPath(QDir(QString::fromUtf8(directory.c_str())).absoluteFilePath(
    QDir::fromNativeSeparators(QString::fromUtf8(path.c_str()))));
}
}

std::vector<UserOption> Postprocessor::PostprocessorUserOptions::definitions()
{
  // Authoritative defaults live in Core. Add future display defaults here;
  // application preferences supply defaults, while individual views own overrides.
  return {{"paths/icons", UserOptionKind::ComponentPath, std::string("Icons/"), validPath}};
}

bool Postprocessor::PostprocessorUserOptions::initialize(UserOptionsInterface& options, std::string& error)
{
  error.clear();
  try
  {
    UserOptionValues values;
    if (!options.load("postprocessor", definitions(), values, error))
    {
      return false;
    }
    auto path = std::get<std::string>(values.at("paths/icons"));
    auto active = resolvedPath(options.component_directory(), path).toUtf8().toStdString();
    configured.iconsPath.swap(path);
    activeIconsPath.swap(active);
    store = &options;
    return true;
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
    return false;
  }
}

void Postprocessor::PostprocessorUserOptions::detach() noexcept
{
  store = nullptr;
}

const Postprocessor::UserOptions& Postprocessor::PostprocessorUserOptions::current() const noexcept
{
  return configured;
}

const std::string& Postprocessor::PostprocessorUserOptions::active_icons_path() const noexcept
{
  return activeIconsPath;
}

bool Postprocessor::PostprocessorUserOptions::restart_required() const
{
  if (store == nullptr)
  {
    return false;
  }
  const auto saved = resolvedPath(store->component_directory(), configured.iconsPath);
  const auto active = QString::fromUtf8(activeIconsPath.c_str());
#ifdef _WIN32
  return saved.compare(active, Qt::CaseInsensitive) != 0;
#else
  return saved != active;
#endif
}

bool Postprocessor::PostprocessorUserOptions::apply(const UserOptionValues& changes, std::string& error)
{
  error.clear();
  if (store == nullptr)
  {
    error = "Postprocessor user options are unavailable.";
    return false;
  }
  try
  {
    const auto schema = definitions();
    UserOptionValues values;
    if (!store->load("postprocessor", schema, values, error) ||
      !merge_user_option_changes(schema, changes, values, error))
    {
      return false;
    }
    auto path = std::get<std::string>(values.at("paths/icons"));
    const auto normalized = QDir::fromNativeSeparators(QString::fromUtf8(path.c_str()));
    if (QFileInfo(normalized).isAbsolute())
    {
      const auto relative = QDir(QString::fromUtf8(store->component_directory().c_str())).relativeFilePath(QDir::cleanPath(normalized));
      if (relative != ".." && !relative.startsWith("../") && !QFileInfo(relative).isAbsolute())
      {
        path = relative.toUtf8().toStdString();
      }
    }
    values["paths/icons"] = path;
    if (!store->save("postprocessor", schema, values, error))
    {
      return false;
    }
    // Non-throwing publication after persistence; active icons wait for restart.
    configured.iconsPath.swap(path);
    return true;
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
    return false;
  }
}

bool Postprocessor::PostprocessorUserOptions::values(UserOptionValues& snapshot, std::string& error) const
{
  if (store == nullptr)
  {
    error = "Postprocessor user options are unavailable.";
    return false;
  }
  return store->load("postprocessor", definitions(), snapshot, error);
}
