#include "CubitWrapInput.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>

bool build_wrap_input(
    const BlockWrapMesh& mesh,
    WrapInput& input)
{
    input = WrapInput{};

    WrapInput result;
    std::unordered_map<NodeId, std::size_t> point_indices;

    const auto add_point = [&](
        NodeId node_id,
        std::size_t& index) -> bool
    {
        const auto existing = point_indices.find(node_id);

        if (existing != point_indices.end())
        {
            index = existing->second;
            return true;
        }

        const auto coordinate = mesh.coordinates.find(node_id);

        if (coordinate == mesh.coordinates.end())
        {
            PRINT_ERROR(
                "Missing coordinates for mesh node %d.\n", node_id);
            return false;
        }

        index = result.soup.points.size();

        result.soup.points.push_back(coordinate->second);
        point_indices.emplace(node_id, index);

        return true;
    };

    for (const auto& face : mesh.faces)
    {
        if (face.size() != 3 && face.size() != 4)
        {
            PRINT_ERROR(
                "Expected a linear triangle or quadrilateral; "
                "received %zu nodes.\n",
                face.size());
            return false;
        }

        std::array<std::size_t, 4> indices{};

        for (std::size_t i = 0; i < face.size(); ++i)
        {
            if (!add_point(face[i], indices[i]))
            {
                return false;
            }
        }

        result.soup.triangles.push_back(
            {indices[0], indices[1], indices[2]});

        if (face.size() == 4)
        {
            result.soup.triangles.push_back(
                {indices[0], indices[2], indices[3]});
        }
    }

    for (NodeId node_id : mesh.additional_nodes)
    {
        if (point_indices.find(node_id) == point_indices.end())
        {
            result.use_point_cloud = true;
        }

        std::size_t index = 0;

        if (!add_point(node_id, index))
        {
            return false;
        }
    }

    if (result.soup.points.empty())
    {
        PRINT_ERROR("Block contains no wrapping input.\n");
        return false;
    }

    if (result.soup.triangles.empty())
    {
        result.use_point_cloud = true;
    }

    input = std::move(result);
    return true;
}

bool collect_block_wrap_mesh(
    const std::vector<int>& block_ids,
    BlockWrapMesh& mesh)
{
    mesh = BlockWrapMesh{};

    if (block_ids.empty())
    {
        PRINT_ERROR("No blocks specified for wrapping.\n");
        return false;
    }

    struct BoundaryFace
    {
        std::vector<int> nodes;
        int count = 0;
    };

    // Sorted connectivity identifies matching faces.
    std::map<std::vector<int>, BoundaryFace> volume_faces;
    std::map<std::vector<int>, std::vector<int>> surface_faces;

    std::set<int> volume_nodes;
    std::set<int> candidate_nodes;
    std::set<int> additional_nodes;

    // Prevent elements shared by requested blocks from being counted twice.
    std::set<std::pair<std::string, int>> visited_elements;

    const auto available_blocks = CubitInterface::get_block_id_list();

    const auto query = [](const std::string& type, int block_id)
    {
        return CubitInterface::parse_cubit_list(
            type, "all in block " + std::to_string(block_id));
    };

    const auto check_meshed = [](
        const std::string& type,
        const std::vector<int>& ids,
        int block_id) -> bool
    {
        for (int id : ids)
        {
            if (!CubitInterface::is_meshed(type, id))
            {
                PRINT_ERROR(
                    "Block %d contains unmeshed %s %d.\n",
                    block_id, type.c_str(), id);
                return false;
            }
        }

        return true;
    };

    const auto add_surface_face = [&](
        const std::string& type,
        int element_id,
        std::size_t expected_nodes) -> bool
    {
        const auto nodes =
            CubitInterface::get_expanded_connectivity(type, element_id);

        if (nodes.size() != expected_nodes)
        {
            PRINT_ERROR(
                "Wrapping requires a linear %s: element %d has %zu nodes.\n",
                type.c_str(), element_id, nodes.size());
            return false;
        }

        auto key = nodes;
        std::sort(key.begin(), key.end());

        if (std::adjacent_find(key.begin(), key.end()) != key.end())
        {
            PRINT_ERROR(
                "%s %d contains repeated node IDs.\n",
                type.c_str(), element_id);
            return false;
        }

        surface_faces.emplace(key, nodes);
        return true;
    };

    const auto add_volume = [&](
        const std::string& type,
        int element_id,
        std::size_t expected_nodes,
        const std::vector<std::vector<int>>& local_faces) -> bool
    {
        if (!visited_elements.emplace(type, element_id).second)
        {
            return true;
        }

        const auto nodes =
            CubitInterface::get_expanded_connectivity(type, element_id);

        if (nodes.size() != expected_nodes)
        {
            PRINT_ERROR(
                "Wrapping requires a linear %s: element %d has %zu nodes.\n",
                type.c_str(), element_id, nodes.size());
            return false;
        }

        std::set<int> unique_nodes(nodes.begin(), nodes.end());

        if (unique_nodes.size() != nodes.size())
        {
            PRINT_ERROR(
                "%s %d contains repeated node IDs.\n",
                type.c_str(), element_id);
            return false;
        }

        volume_nodes.insert(nodes.begin(), nodes.end());

        for (const auto& local_face : local_faces)
        {
            std::vector<int> face;

            for (int local_index : local_face)
            {
                face.push_back(nodes[local_index]);
            }

            auto key = face;
            std::sort(key.begin(), key.end());

            auto& entry = volume_faces[key];

            if (entry.count == 0)
            {
                entry.nodes = face;
            }

            ++entry.count;

            if (entry.count > 2)
            {
                PRINT_ERROR(
                    "Selected volume mesh has a face shared by "
                    "more than two elements.\n");
                return false;
            }
        }

        return true;
    };

    // Zero-based Exodus corner-node connectivity.
    const std::vector<std::vector<int>> tet_faces = {
        {0, 1, 2}, {0, 3, 1}, {1, 3, 2}, {2, 3, 0}
    };

    const std::vector<std::vector<int>> hex_faces = {
        {0, 1, 2, 3}, {4, 5, 6, 7},
        {0, 1, 5, 4}, {1, 2, 6, 5},
        {2, 3, 7, 6}, {3, 0, 4, 7}
    };

    const std::vector<std::vector<int>> wedge_faces = {
        {0, 1, 2}, {3, 4, 5},
        {0, 1, 4, 3}, {1, 2, 5, 4}, {2, 0, 3, 5}
    };

    const std::vector<std::vector<int>> pyramid_faces = {
        {0, 1, 2, 3},
        {0, 1, 4}, {1, 2, 4}, {2, 3, 4}, {3, 0, 4}
    };

    for (int block_id : block_ids)
    {
        if (std::find(
                available_blocks.begin(),
                available_blocks.end(),
                block_id) == available_blocks.end())
        {
            PRINT_ERROR("Block %d does not exist.\n", block_id);
            return false;
        }

        // Inspect direct membership so unmeshed geometry is not overlooked.
        std::vector<int> groups, nodes, spheres, edges, tris, faces;
        std::vector<int> pyramids, tets, hexes, wedges;
        std::vector<int> volumes, surfaces, curves, vertices;

        CubitInterface::get_block_children(
            block_id,
            groups, nodes, spheres, edges, tris, faces,
            pyramids, tets, hexes, wedges,
            volumes, surfaces, curves, vertices);

        if (!groups.empty() || !spheres.empty())
        {
            PRINT_ERROR(
                "Block %d contains groups or spheres, which this "
                "extractor does not yet support.\n",
                block_id);
            return false;
        }

        if (!check_meshed("volume", volumes, block_id) ||
            !check_meshed("surface", surfaces, block_id) ||
            !check_meshed("curve", curves, block_id))
        {
            return false;
        }

        // Explicit node membership is additional point input.
        additional_nodes.insert(nodes.begin(), nodes.end());

        for (int vertex_id : vertices)
        {
            const int node_id = CubitInterface::get_vertex_node(vertex_id);

            if (node_id <= 0)
            {
                PRINT_ERROR(
                    "Block %d contains unmeshed vertex %d.\n",
                    block_id, vertex_id);
                return false;
            }

            additional_nodes.insert(node_id);
        }

        for (int curve_id : curves)
        {
            const auto curve_nodes =
                CubitInterface::get_curve_nodes(curve_id);

            additional_nodes.insert(
                curve_nodes.begin(), curve_nodes.end());
        }

        // Include standalone nodes belonging to selected surfaces.
        for (int surface_id : surfaces)
        {
            const auto surface_nodes =
                CubitInterface::get_surface_nodes(surface_id);

            additional_nodes.insert(
                surface_nodes.begin(), surface_nodes.end());
        }

        const auto block_nodes = query("node", block_id);
        candidate_nodes.insert(block_nodes.begin(), block_nodes.end());

        for (int edge_id : query("edge", block_id))
        {
            const auto edge_nodes =
                CubitInterface::get_expanded_connectivity("edge", edge_id);

            if (edge_nodes.empty())
            {
                PRINT_ERROR("Cannot read edge %d connectivity.\n", edge_id);
                return false;
            }

            additional_nodes.insert(edge_nodes.begin(), edge_nodes.end());
        }

        for (int tri_id : query("tri", block_id))
        {
            if (!add_surface_face("tri", tri_id, 3))
            {
                return false;
            }
        }

        for (int face_id : query("face", block_id))
        {
            if (!add_surface_face("face", face_id, 4))
            {
                return false;
            }
        }

        for (int tet_id : query("tet", block_id))
        {
            if (!add_volume("tet", tet_id, 4, tet_faces))
            {
                return false;
            }
        }

        for (int hex_id : query("hex", block_id))
        {
            if (!add_volume("hex", hex_id, 8, hex_faces))
            {
                return false;
            }
        }

        for (int wedge_id : query("wedge", block_id))
        {
            if (!add_volume("wedge", wedge_id, 6, wedge_faces))
            {
                return false;
            }
        }

        for (int pyramid_id : query("pyramid", block_id))
        {
            if (!add_volume("pyramid", pyramid_id, 5, pyramid_faces))
            {
                return false;
            }
        }
    }

    // Keep only volume boundary faces.
    for (const auto& item : volume_faces)
    {
        if (item.second.count == 1)
        {
            surface_faces.emplace(item.first, item.second.nodes);
        }
    }

    // Nodes not belonging to volume elements remain eligible as points.
    for (int node_id : candidate_nodes)
    {
        if (volume_nodes.find(node_id) == volume_nodes.end())
        {
            additional_nodes.insert(node_id);
        }
    }

    BlockWrapMesh collected;
    std::set<int> required_nodes = additional_nodes;

    for (const auto& item : surface_faces)
    {
        collected.faces.push_back(item.second);
        required_nodes.insert(item.second.begin(), item.second.end());
    }

    collected.additional_nodes.assign(
        additional_nodes.begin(), additional_nodes.end());

    if (required_nodes.empty())
    {
        PRINT_ERROR("Selected blocks contain no wrapping input.\n");
        return false;
    }

    for (int node_id : required_nodes)
    {
        if (!CubitInterface::get_node_exists(node_id))
        {
            PRINT_ERROR("Mesh node %d does not exist.\n", node_id);
            return false;
        }

        const auto coordinates =
            CubitInterface::get_nodal_coordinates(node_id);

        for (double coordinate : coordinates)
        {
            if (!std::isfinite(coordinate))
            {
                PRINT_ERROR(
                    "Mesh node %d has invalid coordinates.\n", node_id);
                return false;
            }
        }

        collected.coordinates.emplace(node_id, coordinates);
    }

    mesh = std::move(collected);
    return true;
}