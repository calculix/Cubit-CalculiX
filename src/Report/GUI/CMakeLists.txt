target_sources(report_module PRIVATE
  ReportGui.cpp
  ReportGui.hpp
  ReportGuiObserver.cpp
  ReportGuiObserver.hpp
  ReportIconLoader.cpp
  ReportIconLoader.hpp
)

add_subdirectory(Content)
add_subdirectory(Export)
add_subdirectory(Preview)
add_subdirectory(Progress)
add_subdirectory(Settings)
add_subdirectory(Workspace)
