#ifndef USEROPTIONS_HPP
#define USEROPTIONS_HPP

#include <QString>

class UserOptions
{
  public:
    UserOptions() {}
    ~UserOptions() {}
  public:
    static int mNumProcesses;
    static QString mProcessName;
    //static int mNumProcessers;
    //static QString mPathSolver;
    //static QString mPathCGX;
    //static QString mPathParaview;
};

#endif // USEROPTIONS_HPP