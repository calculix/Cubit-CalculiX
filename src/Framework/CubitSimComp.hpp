#ifndef CUBITSIMCOMP_HPP
#define CUBITSIMCOMP_HPP

#include "Component.hpp"

class CubitSimComp : public Component
{
public:
  CubitSimComp();
  ~CubitSimComp() override;

  void start_up(int withGUI) override;
  void clean_up() override;
  void clean_up_complete();
  void interrupt_progress() override;

private:
  void release_framework() noexcept;

  bool acquired = false;
  bool owns_components = false;
  bool owns_workspace_host = false;
};

#endif // CUBITSIMCOMP_HPP
