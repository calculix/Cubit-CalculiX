#ifndef POSTPROCESSOR_LAYOUTDEFINITION_HPP
#define POSTPROCESSOR_LAYOUTDEFINITION_HPP

#include <map>
#include <string>
#include <variant>

namespace Postprocessor
{
  enum class SplitDirection
  {
    LeftRight,
    TopBottom
  };

  struct ViewPane
  {
    std::string view_id;
  };

  struct ViewSplit
  {
    SplitDirection direction = SplitDirection::LeftRight;

    // Fraction assigned to the first child: left or top, respectively.
    double ratio = 0.5;
    std::string first_node_id;
    std::string second_node_id;
  };

  struct LayoutNode
  {
    std::string id;
    std::variant<ViewPane, ViewSplit> content;
  };

  // A connected, acyclic tree rooted at root_node_id. Each non-root node has
  // exactly one parent; split children are distinct existing nodes. Ratios must
  // be finite and strictly between zero and one. Map keys equal node IDs.
  // Each view occurs in exactly one leaf; panes reference existing session views.
  // Maximization is temporary presentation state and does not alter this tree.
  struct LayoutDefinition
  {
    std::string root_node_id = "pane1";
    std::map<std::string, LayoutNode> nodes =
    {
      {"pane1", LayoutNode{"pane1", ViewPane{"view1"}}}
    };
  };
}

#endif // POSTPROCESSOR_LAYOUTDEFINITION_HPP
