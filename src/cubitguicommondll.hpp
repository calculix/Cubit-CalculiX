/*!
 * \file cubitguicommondll.hpp
 * \brief
 *   Used to switch between exporting and importing the interface
 *   on windows.
 *
 * \author Mark Richardson
 * \date   9/23/2005
 */

#ifndef CUBIT_GUI_COMMON_DLL_HPP
#define CUBIT_GUI_COMMON_DLL_HPP

//#ifdef WIN32
//#  ifdef cubitguicommon19_EXPORTS
//#    define CGCOMMON_EXPORT _declspec(dllexport)
//#  else
//#    define CGCOMMON_EXPORT _declspec(dllimport)
//#  endif
//#else
#  define CGCOMMON_EXPORT 
//#endif

#endif
