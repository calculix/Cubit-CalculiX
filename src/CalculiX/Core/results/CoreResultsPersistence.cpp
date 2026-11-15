#include "Core/application/CalculiXModelSchema.hpp"
#include "CoreResultsPersistence.hpp"
#include "CoreResults.hpp"
#include "Core/results/frd/CoreResultsFrd.hpp"
#include "Core/results/frd/CoreResultsFrdPersistence.hpp"
#include "Core/results/dat/CoreResultsDat.hpp"
#include "Core/results/dat/CoreResultsDatPersistence.hpp"
#include "Core/jobs/CoreJobs.hpp"
#include "Core/loadCalculiXCore.hpp"
#include "HDF5Tool.hpp"
#include "PersistenceTable.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <set>
#include <charconv>
#include <algorithm>
#include <QJsonDocument>
#include <QJsonParseError>

namespace
{
bool validRegistry(const std::vector<std::vector<int>>& data) {
  if (!PersistenceTable::rows(data, 4) || !PersistenceTable::uniqueIds(data)) {
    return false;
  }
  std::set<int> seenJobs;
  for (const auto& row : data) {
    if (!seenJobs.insert(row[1]).second || ccx_core.jobs->get_jobs_data_id_from_job_id(row[1]) < 0) {
      return false;
    }
  }
  return true;
}
}

bool CoreResultsPersistence::read_registry(HDF5Tool& file, CoreResults& results) {
  if (!results.can_change_readers()) {
    return false;
  }
  try {
    decltype(results.results_data) saved;
    if (file.nameExists(CalculiXModelSchema::path("Results/results_data")) && !file.read_dataset_int_rank_2("results_data", CalculiXModelSchema::path("Results"), saved)) {
      PRINT_ERROR("Cannot read CalculiX result registry: %s\n", file.last_error().c_str());
      return false;
    }
    if (!validRegistry(saved)) {
      PRINT_ERROR("Cannot read CalculiX result registry: invalid records or missing jobs.\n");
      return false;
    }
    results.results_data.swap(saved);
    results.source_metadata.clear();
    results.clear_vtk_cache();
    return true;
  }
  catch (const H5::Exception& error) {
    PRINT_ERROR("Cannot read CalculiX result registry: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error) {
    PRINT_ERROR("Cannot read CalculiX result registry: %s\n", error.what());
    return false;
  }
}

bool CoreResultsPersistence::write_registry(HDF5Tool& file, const CoreResults& results) {
  try {
    if (!validRegistry(results.results_data)) {
      PRINT_ERROR("Cannot save CalculiX result registry: invalid records or missing jobs.\n");
      return false;
    }
    if (!file.ensureGroup(CalculiXModelSchema::path("Results/Frd")) || !file.ensureGroup(CalculiXModelSchema::path("Results/Dat")) || !file.write_dataset_int_rank_2("results_data", CalculiXModelSchema::path("Results"), results.results_data)) {
      PRINT_ERROR("Cannot save CalculiX result registry: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error) {
    PRINT_ERROR("Cannot write CalculiX result registry: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error) {
    PRINT_ERROR("Cannot write CalculiX result registry: %s\n", error.what());
    return false;
  }
}

bool CoreResultsPersistence::initialize_readers(CoreResults& results) {
  if (!results.can_change_readers()) {
    return false;
  }
  try {
    if (!validRegistry(results.results_data)) {
      PRINT_ERROR("Cannot initialize CalculiX result readers: invalid job references.\n");
      return false;
    }
    std::vector<CoreResultsFrd> frd;
    std::vector<CoreResultsDat> dat;
    frd.reserve(results.results_data.size());
    dat.reserve(results.results_data.size());
    for (const auto& row : results.results_data) {
      frd.emplace_back();
      dat.emplace_back();
      if (!frd.back().init(row[1]) || !dat.back().init(row[1])) {
        PRINT_ERROR("Cannot initialize CalculiX result readers for job %d.\n", row[1]);
        return false;
      }
    }
    results.frd_data.swap(frd);
    results.dat_data.swap(dat);
    results.clear_vtk_cache();
    return true;
  }
  catch (const H5::Exception& error) {
    PRINT_ERROR("Cannot initialize CalculiX result readers: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error) {
    PRINT_ERROR("Cannot initialize CalculiX result readers: %s\n", error.what());
    return false;
  }
}

bool CoreResultsPersistence::read_loaded_results(HDF5Tool& file, CoreResults& results) {
  if (!results.can_change_readers()) {
    return false;
  }
  try {
    CoreResults saved;
    saved.results_data = results.results_data;
    if (!initialize_readers(saved)) {
      return false;
    }
    for (auto& frd : saved.frd_data) {
      if (!CoreResultsFrdPersistence::read(file, frd)) {
        return false;
      }
    }
    for (auto& dat : saved.dat_data) {
      if (!CoreResultsDatPersistence::read(file, dat)) {
        return false;
      }
    }
    if (file.nameExists(CalculiXModelSchema::path("Results/result_identities"))) {
      std::vector<std::string> records;
      if (!file.read_dataset_string_rank_1("result_identities",CalculiXModelSchema::path("Results"),records) || records.size() % 2 != 0) {
        return false;
      }
      for (std::size_t index = 0; index < records.size(); index += 2) {
        int job = -1;
        const auto& text = records[index];
        const auto parsed = std::from_chars(text.data(),text.data()+text.size(),job);
        if (parsed.ec != std::errc{} || parsed.ptr != text.data()+text.size() || job <= 0 ||
            records[index+1].size() != 74 || records[index+1].rfind("sha256-v1:",0) != 0 ||
            std::any_of(records[index+1].begin()+10,records[index+1].end(),[](char value) {
              return !((value >= '0' && value <= '9') || (value >= 'a' && value <= 'f'));
            }) || std::none_of(saved.results_data.begin(),saved.results_data.end(),[&](const auto& row) {
              return row.size() >= 2 && row[1] == job;
            }) || !saved.retained_identities.emplace(job,records[index+1]).second) {
          return false;
        }
      }
    }
    const auto metadataPath = CalculiXModelSchema::path("Results/source_metadata");
    if (file.nameExists(metadataPath)) {
      auto dataset = file.file->openDataSet(metadataPath);
      auto space = dataset.getSpace();
      hsize_t extent = 0;
      if (space.getSimpleExtentNdims() != 1 || space.getSimpleExtentDims(&extent) != 1 || extent > saved.results_data.size() * 2 || extent % 2 != 0) {
        PRINT_ERROR("Invalid saved result provenance dimensions.\n");
        return false;
      }
      std::vector<std::string> records;
      if (!file.read_dataset_string_rank_1("source_metadata", CalculiXModelSchema::path("Results"), records)) {
        PRINT_ERROR("Cannot read saved result provenance: %s\n", file.last_error().c_str());
        return false;
      }
      for (std::size_t i = 0; i < records.size(); i += 2) {
        int job = 0;
        const auto parsed = std::from_chars(records[i].data(), records[i].data() + records[i].size(), job);
        if (parsed.ec != std::errc{} || parsed.ptr != records[i].data() + records[i].size() || saved.get_results_data_id_from_job_id(job) < 0 || records[i + 1].size() > 4 * 1024 * 1024) {
          PRINT_ERROR("Invalid saved result provenance record.\n");
          return false;
        }
        QJsonParseError parse;
        const auto json = QJsonDocument::fromJson(QByteArray::fromStdString(records[i + 1]), &parse);
        CoreResultsSourceMetadata metadata;
        std::string error;
        if (parse.error != QJsonParseError::NoError || !json.isObject() || !CoreResultsSourceMetadata::from_json(json.object(), metadata, error) || !saved.source_metadata.emplace(job, std::move(metadata)).second) {
          PRINT_ERROR("Cannot restore result unit provenance: %s\n", error.c_str());
          return false;
        }
      }
    }
    results.source_metadata.swap(saved.source_metadata);
    results.frd_data.swap(saved.frd_data);
    results.dat_data.swap(saved.dat_data);
    results.retained_identities.swap(saved.retained_identities);
    results.clear_vtk_cache();
    return true;
  }
  catch (const H5::Exception& error) {
    PRINT_ERROR("Cannot read CalculiX loaded results: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error) {
    PRINT_ERROR("Cannot read CalculiX loaded results: %s\n", error.what());
    return false;
  }
}

bool CoreResultsPersistence::read(HDF5Tool& file, CoreResults& results) {
  if (!results.can_change_readers()) {
    return false;
  }
  try {
    CoreResults saved;
    if (!read_registry(file, saved) || !read_loaded_results(file, saved)) {
      return false;
    }
    results.results_data.swap(saved.results_data);
    results.source_metadata.swap(saved.source_metadata);
    results.frd_data.swap(saved.frd_data);
    results.dat_data.swap(saved.dat_data);
    results.retained_identities.swap(saved.retained_identities);
    results.clear_vtk_cache();
    return true;
  }
  catch (const H5::Exception& error) {
    PRINT_ERROR("Cannot read CalculiX results: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error) {
    PRINT_ERROR("Cannot read CalculiX results: %s\n", error.what());
    return false;
  }
}

bool CoreResultsPersistence::write(HDF5Tool& file, const CoreResults& results, bool save_loaded_results) {
  try {
    if (!write_registry(file, results)) {
      return false;
    }
    std::vector<std::string> identities;
    std::vector<std::string> metadata;
    if (save_loaded_results) {
      for (const auto& entry : results.source_metadata) {
        metadata.push_back(std::to_string(entry.first));
        metadata.push_back(QJsonDocument(entry.second.to_json()).toJson(QJsonDocument::Compact).toStdString());
      }
    }
    if (!file.write_dataset_string_rank_1("source_metadata", CalculiXModelSchema::path("Results"), metadata)) {
      PRINT_ERROR("Cannot save result unit provenance: %s\n", file.last_error().c_str());
      return false;
    }
    for (const auto& item : results.retained_identities) {
      if (save_loaded_results) {
        identities.push_back(std::to_string(item.first));
        identities.push_back(item.second);
      }
    }
    if (!file.write_dataset_string_rank_1("result_identities",CalculiXModelSchema::path("Results"),identities)) {
      return false;
    }
    if (!save_loaded_results) {
      return true;
    }
    for (const auto& frd : results.frd_data) {
      if (!CoreResultsFrdPersistence::write(file, frd)) {
        return false;
      }
    }
    for (const auto& dat : results.dat_data) {
      if (!CoreResultsDatPersistence::write(file, dat)) {
        return false;
      }
    }
    return true;
  }
  catch (const H5::Exception& error) {
    PRINT_ERROR("Cannot write CalculiX results: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error) {
    PRINT_ERROR("Cannot write CalculiX results: %s\n", error.what());
    return false;
  }
}
