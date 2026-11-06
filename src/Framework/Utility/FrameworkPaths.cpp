#include "FrameworkPaths.hpp"
#include "LoadedLibraryDirectory.hpp"
#include "CubitFileUtil.hpp"
#include "CubitMessage.hpp"
#include <QDir>
#include <QFileInfo>
#include <exception>
#include <utility>

bool FrameworkPaths::resolve(const void* componentAnchor, FrameworkPaths& paths, std::string& error)
{
  error.clear();
  try
  {
    FrameworkPaths candidate;
    candidate.componentDirectory = LoadedLibraryDirectory::from_address(componentAnchor);
    const auto standard = CubitFileUtil::get_standard_path(CubitFileUtil::APPSETTINGS);
    QString directory = QString::fromUtf8(standard.c_str());
    // APPSETTINGS is expected to return a directory. Validate this at runtime;
    // an existing non-directory must never be treated as a directory.
    if (directory.isEmpty() || (QFileInfo::exists(directory) && !QFileInfo(directory).isDir()))
    {
      directory = QDir::currentPath();
      PRINT_WARNING("Cubit settings directory is unavailable; using the working directory for cubit-sim options.\n");
    }
    directory = QDir(directory).absolutePath();
    if (!QDir().mkpath(directory))
    {
      error = "Cannot create the cubit-sim options directory: " + directory.toStdString();
      return false;
    }
    candidate.optionsFilename = QDir(directory).absoluteFilePath("cubitsim_user_options.h5").toUtf8().toStdString();
    paths = std::move(candidate);
    return true;
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
    return false;
  }
}
