#include "CGALTool.hpp"
#include "CubitMessage.hpp"

// Temporarily hide Cubit's NT macro while parsing CGAL headers.
#pragma push_macro("NT")
#undef NT

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/alpha_wrap_3.h>
#include <CGAL/boost/graph/helpers.h>
#include <CGAL/boost/graph/IO/STL.h>

#pragma pop_macro("NT")

#ifdef _WIN32
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
    #include <windows.h>
#endif

#include <cmath>
#include <exception>
#include <system_error>
#include <chrono>
#include <fstream>

// helpers
namespace
{
struct WrapProgressState
{
    using Clock = std::chrono::steady_clock;

    Clock::time_point started = Clock::now();
    Clock::time_point last_report = started;

    std::size_t processed_facets = 0;
    std::size_t inserted_points = 0;
    std::size_t callback_checks = 0;

    std::function<void()> refresh_output;

    void report(const char* stage, bool force = false)
    {
        const auto now = Clock::now();

        if (!force && now - last_report < std::chrono::seconds(5))
        {
            return;
        }

        const double elapsed = std::chrono::duration<double>(now - started).count();
        PRINT_INFO("Alpha wrap: %s - %.1f s, %zu facets processed, %zu points inserted.\n", stage, elapsed, processed_facets, inserted_points);
        last_report = now;

        if (refresh_output)
        {
            refresh_output();
        }
    }
};

struct WrapProgressVisitor : CGAL::Alpha_wraps_3::internal::Wrapping_default_visitor
{
    WrapProgressState* state;

    explicit WrapProgressVisitor(WrapProgressState& progress) : state(&progress)
    {
    }

    template <typename Wrapper>
    void on_flood_fill_begin(const Wrapper&)
    {
        state->report("refining", true);
    }

    template <typename Wrapper>
    bool go_further(const Wrapper&)
    {
        ++state->callback_checks;

        // Avoid reading the clock on every CGAL iteration.
        if (state->callback_checks % 256 == 0)
        {
            state->report("refining");
        }

        return true;
    }

    template <typename Wrapper, typename Gate>
    void before_facet_treatment(const Wrapper&, const Gate&)
    {
        ++state->processed_facets;
    }

    template <typename Wrapper, typename VertexHandle>
    void after_Steiner_point_insertion(const Wrapper&, VertexHandle)
    {
        ++state->inserted_points;
    }

    template <typename Wrapper>
    void on_flood_fill_end(const Wrapper&)
    {
        state->report("finalizing surface", true);
    }
};

struct TemporarySTL
{
    std::filesystem::path filename;
    bool cleanup = true;

    ~TemporarySTL()
    {
        if (cleanup)
        {
            std::error_code error;
            std::filesystem::remove(filename, error);

            if (error)
            {
                PRINT_WARNING(
                    "Cannot remove temporary STL '%s': %s\n",
                    filename.generic_u8string().c_str(),
                    error.message().c_str());
            }
        }
    }
};
}

bool CGALTool::alphawrap(const TriangleSoup& input, double alpha, double offset, const std::filesystem::path& filename, WrapMode mode, bool overwrite, const std::function<void()>& refresh_output)
{
    try
    {
        if (!std::isfinite(alpha) || alpha <= 0.0)
        {
            PRINT_ERROR("Alpha must be finite and positive.\n");
            return false;
        }

        if (!std::isfinite(offset) || offset <= 0.0)
        {
            PRINT_ERROR("Offset must be finite and positive.\n");
            return false;
        }

        if (input.points.empty())
        {
            PRINT_ERROR("No input points for alpha wrapping.\n");
            return false;
        }

        if (mode == WrapMode::TriangleSoup && input.triangles.empty())
        {
            PRINT_ERROR("No input triangles for alpha wrapping.\n");
            return false;
        }

        if (filename.empty())
        {
            PRINT_ERROR("Output filename is empty.\n");
            return false;
        }

        std::error_code path_error;

        const bool file_exists = std::filesystem::exists(filename, path_error);

        if (path_error)
        {
            PRINT_ERROR("Cannot check output file: %s\n", path_error.message().c_str());
            return false;
        }

        if (file_exists && !overwrite)
        {
            PRINT_ERROR("Output file already exists.\n");
            return false;
        }

        WrapProgressState progress;
        progress.refresh_output = refresh_output;

        PRINT_INFO("Alpha wrap input: %zu points, %zu triangles; alpha=%g, offset=%g.\n", input.points.size(), input.triangles.size(), alpha, offset);
        progress.report("preparing input", true);

        using Kernel = CGAL::Exact_predicates_inexact_constructions_kernel;
        using CGALPoint = Kernel::Point_3;
        using CGALMesh = CGAL::Surface_mesh<CGALPoint>;

        std::vector<CGALPoint> points;
        points.reserve(input.points.size());

        for (const Point& point : input.points)
        {
            for (double coordinate : point)
            {
                if (!std::isfinite(coordinate))
                {
                    PRINT_ERROR("Input contains a non-finite coordinate.\n");
                    return false;
                }
            }
            points.emplace_back(point[0], point[1], point[2]);
        }

        if (mode == WrapMode::TriangleSoup)
        {
            for (const Triangle& triangle : input.triangles)
            {
                for (std::size_t index : triangle)
                {
                    if (index >= points.size())
                    {
                        PRINT_ERROR(
                            "Triangle contains an invalid point index.\n");
                        return false;
                    }
                }
            }
        }

        progress.report("initializing", true);

        WrapProgressVisitor visitor(progress);
        CGALMesh wrap;

        if (mode == WrapMode::PointCloud)
        {
            CGAL::alpha_wrap_3(points, alpha, offset, wrap, CGAL::parameters::visitor(visitor));
        }
        else
        {
            CGAL::alpha_wrap_3(points, input.triangles, alpha, offset, wrap, CGAL::parameters::visitor(visitor));
        }

        progress.report("validating surface", true);

        if (wrap.number_of_faces() == 0 ||
            !CGAL::is_valid_polygon_mesh(wrap) ||
            !CGAL::is_triangle_mesh(wrap) ||
            !CGAL::is_closed(wrap))
        {
            PRINT_ERROR("Alpha wrapping did not produce a valid closed triangle mesh.\n");
            return false;
        }

        const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();

        std::filesystem::path temporary_filename = filename;
        temporary_filename += ".tmp-" + std::to_string(timestamp);

        std::error_code file_error;

        const bool temporary_exists =
            std::filesystem::exists(temporary_filename, file_error);

        if (file_error)
        {
            PRINT_ERROR(
                "Cannot check temporary STL: %s\n",
                file_error.message().c_str());

            return false;
        }

        if (temporary_exists)
        {
            PRINT_ERROR("Temporary STL filename already exists.\n");
            return false;
        }

        // Declared before the stream so the stream closes before cleanup.
        TemporarySTL temporary_file{temporary_filename};

        PRINT_INFO("Alpha wrap output: %zu vertices, %zu triangles.\n", static_cast<std::size_t>(wrap.number_of_vertices()), static_cast<std::size_t>(wrap.number_of_faces()));
        progress.report("writing binary STL", true);

        {
            std::ofstream output(temporary_filename,std::ios::out | std::ios::binary);

            if (!output.is_open())
            {
                PRINT_ERROR("Cannot open temporary STL for writing.\n");
                return false;
            }

            CGAL::IO::set_mode(output, CGAL::IO::BINARY);

            const bool written = CGAL::IO::write_STL(
                output,
                wrap,
                CGAL::parameters::stream_precision(17));

            if (!written)
            {
                PRINT_ERROR("Failed to write the temporary STL.\n");
                return false;
            }

            output.close();

            if (output.fail())
            {
                PRINT_ERROR("Failed to finish writing the temporary STL.\n");
                return false;
            }
        }

        progress.report("publishing STL", true);

        // Recheck because wrapping may have taken some time.
        const bool destination_exists =
            std::filesystem::exists(filename, file_error);

        if (file_error)
        {
            PRINT_ERROR(
                "Cannot check destination STL: %s\n",
                file_error.message().c_str());

            return false;
        }

        if (destination_exists && !overwrite)
        {
            PRINT_ERROR("Destination STL already exists.\n");
            return false;
        }

        #ifdef _WIN32
            DWORD move_flags = 0;

            if (overwrite)
            {
                move_flags |= MOVEFILE_REPLACE_EXISTING;
            }

            if (!MoveFileExW(
                    temporary_filename.c_str(),
                    filename.c_str(),
                    move_flags))
            {
                const DWORD error = GetLastError();

                file_error = std::error_code(
                    static_cast<int>(error),
                    std::system_category());
            }
            else
            {
                file_error.clear();
            }
        #else
            std::filesystem::rename(
                temporary_filename,
                filename,
                file_error);
        #endif

        if (file_error)
        {
            PRINT_ERROR("Cannot publish wrapped STL: %s\n", file_error.message().c_str());
            return false;
        }

        temporary_file.cleanup = false;
        progress.report("STL completed", true);

        PRINT_INFO(
            "Alpha wrap written to: %s\n",
            filename.generic_u8string().c_str());

        return true;
    }
    catch (const std::exception& error)
    {
        PRINT_ERROR("Alpha wrapping failed: %s\n",error.what());
        return false;
    }
    catch (...)
    {
        PRINT_ERROR("Alpha wrapping failed with an unknown error.\n");
        return false;
    }
}