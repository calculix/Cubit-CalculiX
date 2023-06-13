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
    QString mPathCGX;
    QString mPathCGXName;
    QString mPathParaView;
    QString mPathParaViewName;
    QString mPathccx2paraview;
    QString mPathccx2paraviewName;
    QString mPathIcons;
    QString mPathIconsName;
};

#endif // USEROPTIONS_HPP