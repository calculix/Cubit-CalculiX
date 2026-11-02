#ifndef COREDRAWSCENE_HPP
#define COREDRAWSCENE_HPP

#include "CoreDrawGraphics.hpp"
#include "CoreDrawPicker.hpp"

class CoreDrawScene
{
public:
  CoreDrawScene();
  void replace(DrawEntityKey entity, DrawGeometry geometry);
  void clear(DrawGroup group);
  void clearAll();
  bool pickSymbol(int viewId, double xFraction, double yFraction, CalculiXDrawSelection& selection, std::string& error);
  bool pickingActive() const;
  void clearHover() noexcept;
  void setColors(const std::string& hoverColor, const std::string& selectionColor);
  bool setPickingEnabled(bool enabled, std::string& error);
  void setPickCallback(std::function<void(const CalculiXDrawSelection&)> callback);
  bool setSelection(const CalculiXDrawSelection& requested);
  bool synchronize(std::string& error);
  void detach() noexcept;

  CalculiXDrawState state;
  bool workspaceActive = false;
private:
  std::map<DrawEntityKey, DrawGeometry> entities;
  CoreDrawGraphics graphics;
  CoreDrawPicker picker;
  std::size_t pickRevision = 0;
  CalculiXDrawSelection selection;
  std::string selectionColor = "yellow";
  bool isHighlighted(DrawEntityKey entity, const CalculiXDrawSelection& requested) const;
};
#endif
