#ifndef POSTPROCESSORTOOLBAR_HPP
#define POSTPROCESSORTOOLBAR_HPP
#include <QToolBar>
#include <QIcon>
#include <string>
class Claro;
class PostprocessorToolbar : public QToolBar
{
public:
  explicit PostprocessorToolbar(QWidget* parent);
  QIcon open_icon() const;
  bool bind_native_actions(Claro* host, std::string& error);
private:
  bool nativeBound = false;
  QIcon openIcon;
};
#endif
