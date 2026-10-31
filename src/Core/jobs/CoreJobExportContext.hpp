#ifndef COREJOBEXPORTCONTEXT_HPP
#define COREJOBEXPORTCONTEXT_HPP

struct CoreJobExportContext
{
  int job_id = -1;
  int topology_id = -1;
  int iteration_id = -1;
  bool adjoint = false;
};

#endif
