#include "CoreResultsIdentity.hpp"
#include "frd/CoreResultsFrd.hpp"
#include "dat/CoreResultsDat.hpp"
#include "vtk/CoreResultsVtkStructs.hpp"
#include <QCryptographicHash>
#include <QByteArrayView>
#include <cstring>
#include <cmath>
#include <limits>
namespace
{
  class Digest
  {
  public:
    QCryptographicHash hash{QCryptographicHash::Sha256};
    QByteArray buffer;
    void flush()
    {
      hash.addData(buffer);
      buffer.clear();
    }
    void number(std::uint64_t value)
    {
      for (int byte = 0; byte < 8; ++byte)
      {
        buffer.append(static_cast<char>(value & 255));
        value >>= 8;
      }
      if (buffer.size() >= 65536)
      {
        flush();
      }
    }
    void add(int value)
    {
      number(static_cast<std::uint64_t>(static_cast<std::int64_t>(value)));
    }
    void add(double value)
    {
      static_assert(sizeof(double) == 8 && std::numeric_limits<double>::is_iec559,"Result fingerprints require IEEE-754 binary64.");
      std::uint64_t bits = 0;
      if (std::isnan(value))
      {
        bits = 0x7ff8000000000000ULL;
      }
      else if (value != 0)
      {
        std::memcpy(&bits,&value,sizeof(bits));
      }
      number(bits);
    }
    void add(const std::string& value)
    {
      number(value.size());
      flush();
      hash.addData(QByteArrayView(value.data(),static_cast<qsizetype>(value.size())));
    }
    template<class T> void add(const std::vector<T>& values)
    {
      number(values.size());
      for (const auto& value : values)
      {
        add(value);
      }
    }
    void add(const FrdAnalysis& value)
    {
      add(value.type); add(value.mode); add(value.nodal_diameter); add(value.subcase); add(value.parameters);
    }
    void add(const DatResultTable& value)
    {
      add(static_cast<int>(value.scope)); add(value.key); add(value.name); add(value.set_name);
      add(value.slave_set); add(value.master_set); add(value.coordinate_system);
      add(value.step); add(value.increment); add(value.time); add(value.columns); add(value.rows);
    }
  };
}
std::string retained_result_identity(const CoreResultsFrd& frd, const CoreResultsDat& dat, const CoreResultsVtkPlan& plan)
{
  Digest digest;
  digest.add(std::string("CubitSim-result-identity-v1"));
  digest.add(frd.header); digest.add(frd.materials); digest.add(frd.nodes); digest.add(frd.nodes_coords);
  digest.add(frd.elements); digest.add(frd.elements_connectivity); digest.add(frd.result_blocks);
  digest.add(frd.result_analysis); digest.add(frd.total_times); digest.add(frd.result_block_components);
  digest.add(frd.result_block_type); digest.add(frd.result_block_data); digest.add(frd.result_block_node_data);
  digest.add(dat.result_blocks); digest.add(dat.total_times); digest.add(dat.result_block_components);
  digest.add(dat.result_block_type); digest.add(dat.result_block_set); digest.add(dat.result_block_data);
  digest.add(dat.result_block_c1_data); digest.add(dat.buckle_data); digest.add(dat.result_section);
  digest.add(dat.result_section_set); digest.add(dat.result_section_data); digest.add(dat.result_section_label);
  digest.add(dat.result_tables);
  digest.add(static_cast<int>(plan.link_source));
  digest.add(static_cast<int>(plan.options.include_frd)); digest.add(static_cast<int>(plan.options.include_dat));
  digest.add(static_cast<int>(plan.options.interpolate_ip_displacement));
  digest.number(plan.mesh.nodes.size());
  for (const auto& node : plan.mesh.nodes)
  {
    digest.add(node.id);
    for (double coordinate : node.coordinates)
    {
      digest.add(coordinate);
    }
  }
  digest.number(plan.mesh.elements.size());
  for (const auto& element : plan.mesh.elements)
  {
    digest.add(element.id); digest.add(element.frd_type); digest.add(element.calculix_type); digest.add(element.connectivity);
  }
  digest.number(plan.mesh.groups.size());
  for (const auto& group : plan.mesh.groups)
  {
    digest.add(static_cast<int>(group.kind)); digest.add(group.model_id); digest.add(group.name); digest.add(group.ids);
    digest.number(group.sides.size());
    for (const auto& side : group.sides)
    {
      digest.add(side.element_id); digest.add(side.side);
    }
  }
  digest.flush();
  return "sha256-v1:"+digest.hash.result().toHex().toStdString();
}
