#ifndef FEA_DOCUMENTATIONMANAGER_HPP
#define FEA_DOCUMENTATIONMANAGER_HPP

#include <QObject>
#include <QPointer>
#include <string>

class QAction;
class Claro;
class QMenu;

// Owns only Framework entries; native Cubit menus remain owned by Cubit.
class DocumentationManager : public QObject
{
public:
  DocumentationManager();
  ~DocumentationManager() override;
  bool attach(Claro* host, std::string& error);
  void set_native_workspace(bool native);
  void detach() noexcept;

private:
  QPointer<Claro> host;
  QPointer<QMenu> nativeHelp;
  QPointer<QMenu> frameworkHelp;
  QPointer<QAction> action;
};

#endif
