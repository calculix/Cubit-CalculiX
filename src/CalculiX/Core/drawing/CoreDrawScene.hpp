#ifndef COREDRAWSCENE_HPP
#define COREDRAWSCENE_HPP

#include "CoreDrawTypes.hpp"
#include "DrawLabelData.hpp"
#include "Framework/Interfaces/DrawingInterface.hpp"

class CoreDrawScene
{
public:
  CoreDrawScene();
  void bind(DrawingInterface* service) noexcept;
  void replace(DrawEntityKey entity, DrawingGeometry geometry, const DrawLabelData* label = nullptr);
  void clear(DrawGroup group);
  void clearAll();
  bool pickSymbol(int viewId, double xFraction, double yFraction, CalculiXDrawSelection& selection, std::string& error);
  bool pickingActive() const;
  void clearHover() noexcept;
  void setColors(const std::string& hoverColor, const std::string& selectionColor);
  bool setPickingEnabled(bool enabled, std::string& error);
  void setPickCallback(std::function<void(const CalculiXDrawSelection&)> callback);
  bool setSelection(const CalculiXDrawSelection& requested);
  bool synchronize(std::string& error, const DrawLabelSettings* labelsOverride = nullptr);
  bool capture(const DrawingFrame& frame, const DrawingCaptureRequest& request, QByteArray& png, std::string& error) const;
  void detach() noexcept;

  CalculiXDrawState state;
  bool workspaceActive = false;
private:
  bool isHighlighted(DrawEntityKey entity, const CalculiXDrawSelection& requested) const;
  std::map<DrawEntityKey, DrawingGeometry> entities;
  std::map<DrawEntityKey, DrawLabelData> labels;
  DrawingInterface* drawings = nullptr;
  CalculiXDrawSelection selection;
  std::function<void(const CalculiXDrawSelection&)> pickCallback;
  std::string hoverColor = "red";
  std::string selectionColor = "yellow";
};
#endif
