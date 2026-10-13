#ifndef CGALTOOL_HPP
#define CGALTOOL_HPP

#include <array>
#include <cstddef>
#include <filesystem>
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

    // Writes the alpha wrap as an ASCII STL.
    // Returns false and prints an error on failure.
    static bool alphawrap(
        const TriangleSoup& input,
        double alpha,
        double offset,
        const std::filesystem::path& filename);
};

#endif