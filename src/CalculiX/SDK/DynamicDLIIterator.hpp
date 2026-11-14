#pragma once

// Cubit 2026.11's SDK omits this header, although CubitSimCore.hpp includes
// it for pointer return declarations. An incomplete type is sufficient for
// those declarations. This does not define the iterator or its object layout;
// constructing or using one requires the real SDK header.
template<class T>
class DynamicDLIIterator;
