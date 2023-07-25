/*!
 *  \file ExportManager.hpp
 *  \brief
 *    Provides an example showing how to add custom file filters
 *    to the export dialog in the file menu.
 *  \author Michael Plooster
 *  \date 27 Mar 2014
 */

#ifndef EXPORTMANAGER_HPP
#define EXPORTMANAGER_HPP

#include <QObject>

class ExportManager : public QObject
{
  Q_OBJECT

public:
  ExportManager(QObject* parent = 0);
  ~ExportManager();

  //! Add my custom export file types to the file menu
  void add_export_types();

  //! Remove my custom export file types from the file menu
  void remove_export_types();

public slots:
  //! Called when one of my export filters has been selected
  //! \param filename The filename selected by the user. By the time this function
  //!   is called, if a file with the given name already exists, the user has
  //!   already indicated they wish to overwrite it.
  //! \param filter The specific filter that was selected.
  void handle_export(const QString& filename, const QString& filter);
};

#endif // EXPORTMANAGER_HPP
