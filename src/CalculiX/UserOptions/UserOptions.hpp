#ifndef USEROPTIONS_HPP
#define USEROPTIONS_HPP

#include <QString>

class UserOptions
{
  public:
    UserOptions();
    ~UserOptions();
  public:
    QString mPathSolver;
    QString mPathSolverName;
    int mSolverThreads;
    QString mSolverThreadsName;
    int mConverterThreads;
    QString mConverterThreadsName;
    QString mPathCGX;
    QString mPathCGXName;
    QString mPathParaView;
    QString mPathParaViewName;
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