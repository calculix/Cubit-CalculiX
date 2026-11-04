// Reader container lifecycle has no solver/GUI dependencies. Keeping these
// definitions separate lets dataset conversion be tested without a live model.
#include "frd/CoreResultsFrd.hpp"
#include "dat/CoreResultsDat.hpp"

CoreResultsFrd::CoreResultsFrd()
{}

CoreResultsFrd::~CoreResultsFrd()
{}

CoreResultsDat::CoreResultsDat()
{}

CoreResultsDat::~CoreResultsDat()
{}
