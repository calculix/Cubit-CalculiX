#ifndef TOPOLOGYANIMATION_HPP
#define TOPOLOGYANIMATION_HPP

#include <QObject>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QElapsedTimer>
#include <atomic>
#include <future>
#include <memory>
#include <vector>

class QTemporaryDir;

// Owns one export. Cubit access is confined to timer callbacks on the GUI thread.
class TopologyAnimation : public QObject
{
public:
  explicit TopologyAnimation(QObject* parent = nullptr);
  ~TopologyAnimation();
  bool start(int topology_id, double threshold, int fps, const QString& output, bool overwrite);
  void cancel();
  bool active() const;

private:
  enum class Phase { Idle, Capturing, Encoding };
  struct EncodingState
  {
    std::atomic_bool cancelled{false};
    std::atomic_int completed_frames{0};
  };
  struct EncodingResult
  {
    bool succeeded = false;
    bool cancelled = false;
    QString error;
    QString cleanup_warning;
  };

  QTimer timer;
  QElapsedTimer busy_wait;
  Phase phase = Phase::Idle;
  int topology_id = -1;
  double threshold = 0.5;
  int fps = 5;
  bool overwrite = false;
  QString output;
  std::vector<int> iterations;
  std::size_t next_frame = 0;
  int reported_frames = -1;
  QSize frame_size;
  QStringList frame_paths;
  QString capture_error;
  std::unique_ptr<QTemporaryDir> temporary;
  std::shared_ptr<EncodingState> encoding_state;
  std::future<EncodingResult> encoding;

  bool prepare();
  bool capture_next_frame();
  void start_encoding();
  void advance();
  void finish(const EncodingResult& result);
};
#endif
