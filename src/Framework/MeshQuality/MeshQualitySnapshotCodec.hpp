#pragma once
#include "MeshQualitySnapshot.hpp"
#include <QByteArray>
class MeshQualitySnapshotCodec
{
public:
  MeshQualitySnapshotCodec();
  static constexpr int maximum_bytes = 256 * 1024 * 1024;
  static bool encode(const MeshQualitySnapshot& snapshot, QByteArray& output, std::string& error);
  static bool decode(const QByteArray& input, std::shared_ptr<const MeshQualitySnapshot>& output, std::string& error);
};
