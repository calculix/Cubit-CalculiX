#include "Hdf5FileTransaction.hpp"
#include "HDF5Tool.hpp"
#include <QFile>
#include <QFileInfo>
#include <QLockFile>
#include <QSaveFile>
#include <QTemporaryDir>
#include <exception>

Hdf5FileTransaction::Hdf5FileTransaction() = default;

bool Hdf5FileTransaction::update(const std::string& path, const Operation& operation, std::string& error) const
{
  error.clear();
  try
  {
    const QString filename = QString::fromStdString(path);
    QLockFile lock(filename + ".lock");
    if (!lock.tryLock(0))
    {
      error = "The cubit-sim HDF5 file is in use: " + path;
      return false;
    }
    QTemporaryDir temporary(QFileInfo(filename).absolutePath() + "/.cubitsim-options-XXXXXX");
    if (!temporary.isValid())
    {
      error = "Cannot create a staged HDF5 directory.";
      return false;
    }
    const QString staged = temporary.filePath("options.h5");
    const bool existed = QFileInfo::exists(filename);
    if (existed && !QFile::copy(filename, staged))
    {
      error = "Cannot stage the existing HDF5 file; it was left unchanged.";
      return false;
    }
    HDF5Tool file(staged.toStdString());
    if (!operation(file, existed, error))
    {
      return false;
    }
    if (!file.close())
    {
      error = "Cannot close the staged HDF5 file.";
      return false;
    }
    QFile input(staged);
    QSaveFile output(filename);
    output.setDirectWriteFallback(false);
    if (!input.open(QIODevice::ReadOnly) || !output.open(QIODevice::WriteOnly))
    {
      error = "Cannot open HDF5 data for atomic replacement.";
      return false;
    }
    while (!input.atEnd())
    {
      const QByteArray bytes = input.read(65536);
      if (bytes.isEmpty() || output.write(bytes) != bytes.size())
      {
        error = "Cannot copy staged HDF5 data.";
        return false;
      }
    }
    if (input.error() != QFileDevice::NoError || !output.commit())
    {
      error = "Cannot commit the HDF5 file.";
      return false;
    }
    return true;
  }
  catch (const H5::Exception& exception)
  {
    error = exception.getDetailMsg();
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
  }
  return false;
}
