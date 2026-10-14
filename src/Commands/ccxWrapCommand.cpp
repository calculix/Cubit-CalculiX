#include "ccxWrapCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CubitWrapInput.hpp"
#include "CGALTool.hpp"

#include <algorithm>
#include <cmath>
#include <exception>
#include <filesystem>

std::vector<std::string> ccxWrapCommand::get_syntax()
{
    return {
        "ccx wrap block <value:label='block_ids'>... "
        "alpha <value:label='alpha'> "
        "offset <value:label='offset'> "
        "file <string:label='filename'> "
        "[no_import] [overwrite]"
    };
}

std::vector<std::string> ccxWrapCommand::get_syntax_help()
{
    return {
        "ccx wrap block <block_ids>... alpha <alpha> "
        "offset <offset> file \"<filename.stl>\" [no_import] [overwrite]"
    };
}

std::vector<std::string> ccxWrapCommand::get_help()
{
    return {
        "Wrap the combined mesh of the requested blocks.",
        "Every selected entity must be meshed.",
        "Volume input uses only boundary faces and nodes.",
        "Quads are split into triangles.",
        "Additional curve or vertex nodes select point-cloud wrapping.",
        "The STL is imported automatically unless no_import is specified."
    };
}

bool ccxWrapCommand::execute(CubitCommandData& data)
{
    try
    {
        std::vector<int> block_ids;
        double alpha = 0.0;
        double offset = 0.0;
        std::string filename;

        if (!data.get_values("block_ids", block_ids) ||
            block_ids.empty())
        {
            PRINT_ERROR("Specify at least one block ID.\n");
            return false;
        }

        if (!data.get_value("alpha", alpha) ||
            !data.get_value("offset", offset) ||
            !data.get_string("filename", filename))
        {
            PRINT_ERROR("Missing wrapping parameters.\n");
            return false;
        }

        if (!std::isfinite(alpha) || alpha <= 0.0 ||
            !std::isfinite(offset) || offset <= 0.0)
        {
            PRINT_ERROR("Alpha and offset must be finite and positive.\n");
            return false;
        }

        if (filename.empty())
        {
            PRINT_ERROR("Output filename is empty.\n");
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
        block_ids.erase(
            std::unique(block_ids.begin(), block_ids.end()),
            block_ids.end());

        const bool no_import = data.find_keyword("no_import");
        const bool overwrite = data.find_keyword("overwrite");

        const std::filesystem::path output_path =
            std::filesystem::absolute(
                std::filesystem::u8path(filename));

        const std::string import_filename =
            output_path.generic_u8string();

        // The filename will be inserted into a quoted Cubit command.
        if (!no_import &&
            import_filename.find_first_of("\"\r\n") != std::string::npos)
        {
            PRINT_ERROR(
                "Filename cannot be quoted safely for automatic import.\n");
            return false;
        }

        BlockWrapMesh mesh;

        if (!collect_block_wrap_mesh(block_ids, mesh))
        {
            return false;
        }

        WrapInput input;
        if (!build_wrap_input(mesh, input))
        {
          return false;
        }

        CGALTool::WrapMode mode = CGALTool::WrapMode::TriangleSoup;

        if (input.use_point_cloud)
        {
            mode = CGALTool::WrapMode::PointCloud;
        }

        if (!CGALTool::alphawrap(
                input.soup,
                alpha,
                offset,
                output_path,
                mode,
                overwrite))
        {
            return false;
        }

        if (no_import)
        {
            return true;
        }

        const std::string import_command =
            "import stl \"" + import_filename + "\"";

        if (!CubitInterface::cmd_single(import_command.c_str()))
        {
            PRINT_ERROR(
                "STL was created, but Cubit import failed: %s\n",
                import_filename.c_str());
            return false;
        }

        return true;
    }
    catch (const std::exception& error)
    {
        PRINT_ERROR("Wrapping command failed: %s\n", error.what());
        return false;
    }
}