#ifndef CUBITWRAPINPUT_HPP
#define CUBITWRAPINPUT_HPP

#include "CGALTool.hpp"

#include <string>
#include <unordered_map>
#include <vector>

using NodeId = int;

struct BlockWrapMesh
{
    // Coordinates for the nodes referenced below.
    std::unordered_map<NodeId, CGALTool::Point> coordinates;

    // Ordered connectivity: three nodes for tris, four for quads.
    // Contains volume boundary faces and explicitly selected surface faces.
    std::vector<std::vector<NodeId>> faces;

    // Curve / vertex nodes and any standalone surface nodes.
    // Never contains volume interior nodes.
    std::vector<NodeId> additional_nodes;
};

struct WrapInput
{
    CGALTool::TriangleSoup soup;
    bool use_point_cloud = false;
};

bool build_wrap_input(const BlockWrapMesh& mesh, WrapInput& input);

bool collect_block_wrap_mesh(const std::vector<int>& block_ids,BlockWrapMesh& mesh);

struct WrapElement
{
    std::string entity;
    int local_id = -1;
};

bool collect_element_wrap_mesh(const std::vector<WrapElement>& elements, BlockWrapMesh& mesh);

#endif
