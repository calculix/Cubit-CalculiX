#ifndef USEROPTIONS_HPP
#define USEROPTIONS_HPP

#include <QString>
#include "DrawingOptions.hpp"

class UserOptions
{
  public:
    UserOptions();
    ~UserOptions();
  public:
    DrawingOptions mDrawing;
    QString mPathSolver;
    QString mPathSolverName;
    int mSolverThreads;
    QString mSolverThreadsName;
    int mConverterThreads;
    QString mConverterThreadsName;
    int mTopologyQuadratureCacheMiB;
    QString mTopologyQuadratureCacheMiBName;
    QString mPathIcons;
    QString mPathIconsName;
    QString mPathPythonInterface;
    QString mPathPythonInterfaceName;
    bool mSaveLoadedResults;
    QString mSaveLoadedResultsName;
    QString mPathMaterialLibrary;
    QString mPathMaterialLibraryName;
};

#endif // USEROPTIONS_HPP
