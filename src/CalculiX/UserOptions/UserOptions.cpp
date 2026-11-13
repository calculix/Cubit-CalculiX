#include "UserOptions.hpp"

UserOptions::UserOptions()
  : mDrawing{}, mSolverThreads(0), mConverterThreads(0), mTopologyQuadratureCacheMiB(0), mSaveLoadedResults(false)
{
}

UserOptions::~UserOptions()
{

}
