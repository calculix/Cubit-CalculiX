#include "Mp4Animation.hpp"
#include <QProcess>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QElapsedTimer>
#include <QRegularExpression>
#include <algorithm>
namespace
{
  QString encoder(QString& error)
  {
    auto path = qEnvironmentVariable("CUBITSIM_FFMPEG");
    if (path.isEmpty())
    {
      path = QStandardPaths::findExecutable("ffmpeg");
    }
    if (path.isEmpty() || !QFileInfo(path).isExecutable())
    {
      error = "MP4 requires FFmpeg with libx264. Set CUBITSIM_FFMPEG to its executable or add it to PATH.";
      return {};
    }
    return path;
  }
  bool run(const QStringList& arguments,const std::atomic_bool& cancelled,
    std::atomic_int* completed,QByteArray& output,QString& error,int timeout = 0)
  {
    const auto executable = encoder(error);
    if (executable.isEmpty() || cancelled.load())
    {
      return false;
    }
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(executable,arguments);
    if (!process.waitForStarted(5000))
    {
      error = "Cannot start FFmpeg: "+process.errorString();
      return false;
    }
    QElapsedTimer elapsed;
    elapsed.start();
    QByteArray pending;
    while (process.state() != QProcess::NotRunning)
    {
      process.waitForFinished(100);
      const auto bytes = process.readAll();
      output = (output+bytes).right(32768);
      pending += bytes;
      int newline = -1;
      while ((newline = pending.indexOf('\n')) >= 0)
      {
        const auto line = pending.left(newline).trimmed();
        pending.remove(0,newline+1);
        if (completed && line.startsWith("frame="))
        {
          bool valid = false;
          const int frame = line.mid(6).trimmed().toInt(&valid);
          if (valid)
          {
            completed->store(std::max(0,frame));
          }
        }
      }
      pending = pending.right(32768);
      if (cancelled.load() || (timeout > 0 && elapsed.elapsed() > timeout))
      {
        process.kill();
        process.waitForFinished(5000);
        if (!cancelled.load())
        {
          error = "FFmpeg capability check timed out.";
        }
        return false;
      }
    }
    output = (output+process.readAll()).right(32768);
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
    {
      error = "FFmpeg failed: "+QString::fromUtf8(output.right(4096));
      return false;
    }
    return true;
  }
}
namespace Framework::Media
{
  bool Mp4Animation::available(const std::atomic_bool& cancelled,QString& error)
  {
    QByteArray output;
    if (!run({"-hide_banner","-buildconf"},cancelled,nullptr,output,error,30000))
    {
      return false;
    }
    if (output.contains("--enable-nonfree"))
    {
      error = "Use an open-source redistributable FFmpeg build without --enable-nonfree.";
      return false;
    }
    output.clear();
    if (!run({"-hide_banner","-encoders"},cancelled,nullptr,output,error,30000))
    {
      return false;
    }
    const QRegularExpression x264("(?:^|\\n)\\s*V\\S*\\s+libx264\\s");
    if (!x264.match(QString::fromUtf8(output)).hasMatch())
    {
      error = "The selected FFmpeg does not provide the libx264 H.264 encoder.";
      return false;
    }
    return true;
  }
  bool Mp4Animation::write(const QString& frames,const QString& output,int fps,
    const std::atomic_bool& cancelled,std::atomic_int& completed,QString& error)
  {
    QByteArray log;
    return run({"-hide_banner","-nostdin","-n","-framerate",QString::number(fps),"-start_number","0",
      "-i",QDir(frames).filePath("frame_%06d.png"),"-an","-c:v","libx264","-crf","18",
      "-pix_fmt","yuv420p","-movflags","+faststart","-progress","pipe:1","-nostats",output},
      cancelled,&completed,log,error);
  }
}
