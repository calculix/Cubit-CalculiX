#ifndef RESULTSOURCEREGISTRYINTERFACE_HPP
#define RESULTSOURCEREGISTRYINTERFACE_HPP

#include <string>

class ResultSourceProviderInterface;

struct ResolvedResultSourceProvider
{
  std::string application_id;

  // Borrowed while Framework is initialized; never retain across shutdown.
  ResultSourceProviderInterface* provider = nullptr;
};

class ResultSourceRegistryInterface
{
public:
  virtual ~ResultSourceRegistryInterface() = default;

  // Resolve a command prefix and its canonical application identity together.
  // Success replaces output and clears error; failure leaves output unchanged.
  virtual bool resolve_result_provider(
    const std::string& prefix,
    ResolvedResultSourceProvider& output,
    std::string& error) const = 0;
};

#endif // RESULTSOURCEREGISTRYINTERFACE_HPP
