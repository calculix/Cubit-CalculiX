#ifndef CUBITSIM_FRAMEWORKUSEROPTIONS_HPP
#define CUBITSIM_FRAMEWORKUSEROPTIONS_HPP

#include "Framework/Interfaces/UserOption.hpp"
#include <cstdint>

class UserOptionsInterface;

struct FrameworkOptions
{
  bool monitorEnabled = false;
  int intervalSeconds = 0;
  bool showCpu = false;
  bool showMemory = false;
  bool showCubit = false;
  bool showJobs = false;
  bool showJobAttribution = false;
  std::uint32_t systemColorRgb = 0;
  std::uint32_t cubitColorRgb = 0;
  std::uint32_t jobsColorRgb = 0;
};

class FrameworkUserOptions
{
public:
  static std::vector<UserOption> definitions();
  bool initialize(UserOptionsInterface& store, std::string& error);
  bool values(UserOptionValues& snapshot, std::string& error) const;
  bool apply(const UserOptionValues& changes, std::string& error);
  const FrameworkOptions& current() const noexcept;

private:
  UserOptionsInterface* store = nullptr;
  FrameworkOptions configured;
};

#endif
