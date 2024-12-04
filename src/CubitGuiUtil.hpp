/*!
 * \file CubitGuiUtil.hpp
 * \brief
 *   The CubitGuiUtil class provides access to several common features
 *   for the Cubit GUI.
 *
 * \author Mark Richardson
 * \date   10/4/2005
 */

#ifndef CUBIT_GUI_UTIL_HPP
#define CUBIT_GUI_UTIL_HPP


#include "cubitguicommondll.hpp"
#include <QObject>
#include <vector>
#include <string>


class CubitGuiEventBridge;
class CubitPickHandler;
class HelpMap;
//class MeshErrorMap;


//! \class CubitGuiUtil
//! \brief
//!   The CubitGuiUtil class provides access to several common features
//!   for the Cubit GUI.
class CGCOMMON_EXPORT CubitGuiUtil : public QObject
{
  Q_OBJECT

  public:
    CubitGuiUtil();
    virtual ~CubitGuiUtil();

    void initialize();
    void cleanup();

    CubitGuiEventBridge *get_event_bridge() const {return mEventBridge;}

    CubitPickHandler *get_pick_handler() const {return mPickHandler;}

    HelpMap *get_help_map() const {return mHelpMap;}

  signals:
//    void checkPointNeeded();
    void command_issued(QString cmd);

  public:

    static CubitGuiUtil *instance(){return &mInstance;}

    //! \brief
    //!   Sends a cubit command to the framework to be processed.
    //!
    //! The command string is translated to Python syntax before
    //! sending it. The command is sent through the Claro interface,
    //! which ensures that the command will be played even if command
    //! playback is paused.
    //!
    //! \param command The cubit command string.
    static void send_cubit_command(const char *command);

    //! \brief
    //!   Sends cubit commands to the framework to be processed.
    //! \param commands The list of cubit commands.
    //! \sa CubitGuiUtil::send_cubit_command(const char *)
    static void send_cubit_commands(std::vector<std::string> &commands);


    //! \brief
    //!   Sends a silent directly to cubit.
    //!
    //!   Pass a command string into Cubit and have it executed without
    //!   being verbose at the command prompt.
    //!
    //!   Passing a command into Cubit using this method will result in an
    //!   immediate execution of the command. The command is passed directly
    //!   to Cubit without any validation or other checking.
    //!
    //! \param command The cubit command string.
    static void send_silent_cubit_command(const char *command);

    //! Returns true if the FEA portions of the GUI should be enabled.
    static bool fea_enabled();

    //! Returns true if the CFD portions of the GUI should be enabled.
    static bool cfd_enabled();
  
  private:
    static void set_bc_modules_from_settings();
    CubitGuiEventBridge *mEventBridge;
    CubitPickHandler *mPickHandler;
    HelpMap *mHelpMap;
    static bool mCFDEnabled;
    static bool mFEAEnabled;
    static bool mBCModulesStateValid;
//    MeshErrorMap *mMeshErrorMap;
    static CubitGuiUtil mInstance; //!< The class instance.
};

#endif
