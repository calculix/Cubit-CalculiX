#include "ccxWrapCommand.hpp"
#include <string>
#include <vector>
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "Core/topology/CalculiXTopologyInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CubitWrapInput.hpp"
#include "CGALTool.hpp"
#include <QCoreApplication>
#include <QEventLoop>
#include <QScopedValueRollback>

#include <algorithm>
#include <cmath>
#include <exception>
#include <filesystem>
#include <limits>
#include <set>

namespace
{
bool is_wrap_entity(const std::string& entity)
{
    return entity == "hex" || entity == "tet" || entity == "wedge" || entity == "pyramid" || entity == "tri" || entity == "face" || entity == "quad" || entity == "edge";
}

bool collect_topology_wrap_mesh(int topology_id, int iteration_id, double threshold, BlockWrapMesh& mesh)
{
  CalculiXMeshInterface ccx_mesh_iface;
  CalculiXTopologyInterface ccx_topology_iface;

    mesh = BlockWrapMesh{};

    if (topology_id <= 0 || iteration_id <= 0 || !std::isfinite(threshold) || threshold < 0.0 || threshold > 1.0)
    {
        PRINT_ERROR("Invalid topology ID, iteration, or threshold.\n");
        return false;
    }

    const std::string state = ccx_topology_iface.get_topology_run_state(topology_id);

    if (state.empty())
    {
        PRINT_ERROR("Topology case %d is unavailable.\n", topology_id);
        return false;
    }

    if (state == "running")
    {
        PRINT_ERROR("Cannot wrap a running topology case.\n");
        return false;
    }

    const auto rows = ccx_topology_iface.get_topology_iteration_element_data(topology_id, iteration_id);

    if (rows.empty())
    {
        PRINT_ERROR("No completed topology data for case %d, iteration %d.\n", topology_id, iteration_id);
        return false;
    }

    std::set<int> seen;
    std::vector<WrapElement> selected;

    for (const auto& row : rows)
    {
        if (row.size() != 4)
        {
            PRINT_ERROR("Invalid topology element data.\n");
            return false;
        }

        if (!std::isfinite(row[0]) || row[0] < 1.0 || row[0] > (std::numeric_limits<int>::max)() || std::floor(row[0]) != row[0])
        {
            PRINT_ERROR("Invalid topology element ID.\n");
            return false;
        }

        if (!std::isfinite(row[1]) || row[1] < 0.0 || row[1] > 1.0 || (row[3] != 0.0 && row[3] != 1.0))
        {
            PRINT_ERROR("Invalid topology density or preservation flag.\n");
            return false;
        }

        const int global_id = static_cast<int>(row[0]);

        if (!seen.insert(global_id).second)
        {
            PRINT_ERROR("Duplicate topology element %d.\n", global_id);
            return false;
        }

        // Validate the entire stored design, matching topology projection.
        if (!CubitInterface::get_element_exists(global_id))
        {
            PRINT_ERROR("Topology element %d no longer exists in Cubit.\n", global_id);
            return false;
        }

        const std::string element_type = CubitInterface::get_element_type(global_id);
        std::string entity = element_type;

        if (!is_wrap_entity(entity))
        {
            entity = ccx_mesh_iface.get_cubit_element_type_entity(element_type);
        }

        const int local_id = CubitInterface::get_element_type_id(global_id);

        if (!is_wrap_entity(entity) || local_id <= 0)
        {
            PRINT_ERROR("Unsupported topology mesh element %d.\n", global_id);
            return false;
        }

        const bool preserved = row[3] == 1.0;

        if (preserved || row[1] >= threshold)
        {
            selected.push_back({entity, local_id});
        }
    }

    if (selected.empty())
    {
        PRINT_ERROR("No topology elements remain at threshold %.6g.\n", threshold);
        return false;
    }

    if (!collect_element_wrap_mesh(selected, mesh))
    {
        return false;
    }

    PRINT_INFO("Wrapping %zu of %zu topology elements from case %d, iteration %d, threshold %.6g.\n", selected.size(), rows.size(), topology_id, iteration_id, threshold);
    return true;
}
}

std::vector<std::string> ccxWrapCommand::get_syntax()
{
    return {
        "ccx wrap block <value:label='block_ids'>... [alpha <value:label='alpha'>] [offset <value:label='offset'>] file <string:label='filename'> [no_import] [overwrite]",
        "ccx wrap topology <value:label='topology_id'> iteration <value:label='iteration_id'> threshold <value:label='threshold'> [alpha <value:label='alpha'>] [offset <value:label='offset'>] file <string:label='filename'> [no_import] [overwrite]"
    };
}

std::vector<std::string> ccxWrapCommand::get_syntax_help()
{
    return {
        "ccx wrap block <block_ids>... [alpha <alpha>] [offset <offset>] file \"<filename.stl>\" [no_import] [overwrite]",
        "ccx wrap topology <topology_id> iteration <iteration_id> threshold <threshold> [alpha <alpha>] [offset <offset>] file \"<filename.stl>\" [no_import] [overwrite]"
    };
}

std::vector<std::string> ccxWrapCommand::get_help()
{
    return {
        "Wrap the combined mesh of the requested blocks or a topology iteration.",
        "Topology wrapping keeps elements with density greater than or equal to threshold.",
        "Preserved topology elements are always included.",
        "Threshold must be between zero and one.",
        "Topology wrapping requires a completed iteration and an idle topology case.",
        "Wrapping uses the current Cubit mesh coordinates.",
        "If alpha is omitted, it defaults to the wrapping input bounding-box diagonal divided by 40.",
        "If offset is omitted, it defaults to the resolved alpha divided by 30.",
        "Explicit alpha and offset values must be finite and positive.",
        "Every selected entity must be meshed.",
        "Volume input uses only boundary faces and nodes.",
        "Higher-order solids and shells use a corner-node surface approximation.",
        "Beam and truss nodes use point-cloud wrapping.",
        "Quads are split into triangles.",
        "Additional curve or vertex nodes select point-cloud wrapping.",
        "The STL is imported automatically unless no_import is specified."
    };
}

bool ccxWrapCommand::execute(CubitCommandData& data)
{
    static bool wrapping = false;

    if (wrapping)
    {
        PRINT_ERROR("An alpha wrapping command is already running.\n");
        return false;
    }

    const QScopedValueRollback<bool> wrapping_guard(wrapping, true);

    try
    {
        double alpha = 0.0;
        double offset = 0.0;
        std::string filename;

        const bool automatic_alpha = !data.find_keyword("alpha");
        const bool automatic_offset = !data.find_keyword("offset");

        if (!data.get_string("filename", filename))
        {
            PRINT_ERROR("Missing output filename.\n");
            return false;
        }

        if (!automatic_alpha)
        {
            if (!data.get_value("alpha", alpha) || !std::isfinite(alpha) || alpha <= 0.0)
            {
                PRINT_ERROR("Alpha must be finite and positive.\n");
                return false;
            }
        }

        if (!automatic_offset)
        {
            if (!data.get_value("offset", offset) || !std::isfinite(offset) || offset <= 0.0)
            {
                PRINT_ERROR("Offset must be finite and positive.\n");
                return false;
            }
        }

        if (filename.empty())
        {
            PRINT_ERROR("Output filename is empty.\n");
            return false;
        }

        const bool no_import = data.find_keyword("no_import");
        const bool overwrite = data.find_keyword("overwrite");
        const std::filesystem::path output_path = std::filesystem::absolute(std::filesystem::u8path(filename));
        const std::string import_filename = output_path.generic_u8string();

        if (!no_import && import_filename.find_first_of("\"\r\n") != std::string::npos)
        {
            PRINT_ERROR("Filename cannot be quoted safely for automatic import.\n");
            return false;
        }

        BlockWrapMesh mesh;

        if (data.find_keyword("topology"))
        {
            int topology_id = -1;
            int iteration_id = -1;
            double threshold = 0.5;

            if (!data.get_value("topology_id", topology_id) || !data.get_value("iteration_id", iteration_id) || !data.get_value("threshold", threshold))
            {
                PRINT_ERROR("Missing topology wrapping parameters.\n");
                return false;
            }

            if (!collect_topology_wrap_mesh(topology_id, iteration_id, threshold, mesh))
            {
                return false;
            }
        }
        else
        {
            std::vector<int> block_ids;

            if (!data.get_values("block_ids", block_ids) || block_ids.empty())
            {
                PRINT_ERROR("Specify at least one block ID.\n");
                return false;
            }

            for (int block_id : block_ids)
            {
                if (block_id <= 0)
                {
                    PRINT_ERROR("Invalid block ID: %d.\n", block_id);
                    return false;
                }
            }

            std::sort(block_ids.begin(), block_ids.end());
            block_ids.erase(std::unique(block_ids.begin(), block_ids.end()), block_ids.end());

            if (!collect_block_wrap_mesh(block_ids, mesh))
            {
                return false;
            }
        }

        WrapInput input;

        if (!build_wrap_input(mesh, input))
        {
            return false;
        }

        if (automatic_alpha)
        {
            if (input.soup.points.empty())
            {
                PRINT_ERROR("Cannot calculate alpha without input points.\n");
                return false;
            }

            CGALTool::Point minimum = input.soup.points.front();
            CGALTool::Point maximum = minimum;

            for (const auto& point : input.soup.points)
            {
                for (std::size_t axis = 0; axis < 3; ++axis)
                {
                    if (!std::isfinite(point[axis]))
                    {
                        PRINT_ERROR("Cannot calculate alpha from non-finite coordinates.\n");
                        return false;
                    }

                    minimum[axis] = (std::min)(minimum[axis], point[axis]);
                    maximum[axis] = (std::max)(maximum[axis], point[axis]);
                }
            }

            const double dx = maximum[0] - minimum[0];
            const double dy = maximum[1] - minimum[1];
            const double dz = maximum[2] - minimum[2];
            const double diagonal = std::hypot(dx, dy, dz);

            if (!std::isfinite(diagonal) || diagonal <= 0.0)
            {
                PRINT_ERROR("Cannot calculate alpha: input bounding-box diagonal must be finite and positive.\n");
                return false;
            }

            alpha = diagonal / 40.0;
            PRINT_INFO("Automatic alpha: input bounding-box diagonal %.9g / 40.\n", diagonal);
        }

        if (automatic_offset)
        {
            offset = alpha / 30.0;
        }

        if (!std::isfinite(alpha) || alpha <= 0.0 || !std::isfinite(offset) || offset <= 0.0)
        {
            PRINT_ERROR("Calculated wrapping parameters are outside the supported numeric range. Specify alpha and offset explicitly.\n");
            return false;
        }

        PRINT_INFO("Alpha wrap parameters used: alpha=%.17g (%s), offset=%.17g (%s), in model length units.\n", alpha, automatic_alpha ? "automatic" : "specified", offset, automatic_offset ? "automatic: alpha / 30" : "specified");
        PRINT_INFO("To override, specify alpha <value> and/or offset <value> in the wrap command, or enter positive values in the wrapping widget.\n");
        PRINT_INFO("Smaller alpha retains finer detail; larger alpha gives a coarser wrap. Smaller offset gives a tighter fit; larger offset gives a looser fit.\n");

        CGALTool::WrapMode mode = CGALTool::WrapMode::TriangleSoup;

        if (input.use_point_cloud)
        {
            mode = CGALTool::WrapMode::PointCloud;
        }

        const auto refresh_output = []()
        {
            if (QCoreApplication::instance() != nullptr)
            {
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers, 10);
            }
        };

        if (!CGALTool::alphawrap(input.soup, alpha, offset, output_path, mode, overwrite, refresh_output))
        {
            return false;
        }

        if (no_import)
        {
            return true;
        }

        PRINT_INFO("Alpha wrap: importing STL into Cubit.\n");
        refresh_output();

        const std::string import_command = "import stl \"" + import_filename + "\"";

        if (!CubitInterface::cmd_single(import_command.c_str()))
        {
            PRINT_ERROR("STL was created, but Cubit import failed: %s\n", import_filename.c_str());
            return false;
        }

        PRINT_INFO("Alpha wrap: import completed.\n");
        return true;
    }
    catch (const std::exception& error)
    {
        PRINT_ERROR("Wrapping command failed: %s\n", error.what());
        return false;
    }
}
