#ifndef CGALTOOL_HPP
#define CGALTOOL_HPP

#include <array>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <vector>

class CGALTool
{
public:
    using Point = std::array<double, 3>;
    using Triangle = std::array<std::size_t, 3>;

    struct TriangleSoup
    {
        std::vector<Point> points;
        std::vector<Triangle> triangles;
    };

    enum class WrapMode
    {
        TriangleSoup,
        PointCloud
    };

    // Writes the alpha wrap as a binary STL.
    // Returns false and prints an error on failure.
    static bool alphawrap(const TriangleSoup& input, double alpha, double offset, const std::filesystem::path& filename, WrapMode mode = WrapMode::TriangleSoup, bool overwrite = false, const std::function<void()>& refresh_output = {});

};

#endif