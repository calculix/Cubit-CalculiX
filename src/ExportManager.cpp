/*!
 *  \file ExportManager.cpp
 *  \brief
 *    Contains the implementation of the ExportManager class.
 *  \author Michael Plooster
 *  \date 27 Mar 2014
 */

#include "ExportManager.hpp"
#include "Claro.hpp"
#include "ClaroFileMenu.hpp"
#include "CommandWindow.hpp"

ExportManager::ExportManager(QObject *parent) :
  QObject(parent)
{}

ExportManager::~ExportManager()
{}

void ExportManager::add_export_types()
{
  Claro *gui = Claro::instance();
  if(gui)
  {
    // Get the file menu
    ClaroFileMenu *file_menu = gui->file_menu();

    // Add my filters to the export dialog. Note that you can add multiple
    // export filters at once by separating the list with semicolons.
    QString filter_names = "MyExport (*.me);;AnotherExport (*.ae)";
    QObject *filter = file_menu->add_export(filter_names.toUtf8().data(),
                                            "mycomp");

    // Connect the filter to the method that will handle the export
    connect(filter, SIGNAL(filterSelected(const QString&, const QString&)),
            this, SLOT(handle_export(const QString&, const QString&)));
  }
}

void ExportManager::remove_export_types()
{
  Claro *gui = Claro::instance();
  if(gui)
  {
    ClaroFileMenu *file_menu = gui->file_menu();
    file_menu->remove_component_items("mycomp");
  }
}

void ExportManager::handle_export(const QString &filename, const QString &filter)
{
  // Handle the export with the given filename and filter. At this point, I
  // could create an export dialog to give the user more options or simply
  // send an export command (provided I have implemented a command plugin for
  // my export command).

  CommandWindow* cw = Claro::instance()->command_line();
  QString message = "The \"" + filter + "\" filter was selected ";
  message += "with the filename \"" + filename + "\"\n";
  cw->print_data(message);

  // Here we can do things specific to the export type, based on the filter
  // that was selected.
  if(filter.contains("MyExport"))
  {
    cw->print_data("MyExport is not implemented.\n");
  }
  else if(filter.contains("AnotherExport"))
  {
    cw->print_data("AnotherExport is not implemented.\n");
  }
}
