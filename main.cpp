#include "BonxaiVisualiser.hpp"

#include <chrono>
#include <iostream>
#include <random>
#include <thread>

// Define color function (RGBA)
std::tuple<unsigned char, unsigned char, unsigned char, unsigned char> getColor(const bool& data)
{
    if (data)
    {
        return { 0, 255, 0, 30 };
    }
    else
    {
        return { 0, 0, 0, 0 };
    }
};

int main()
{
    // Init grid
    constexpr double voxelRes = 1.0;
    Bonxai::VoxelGrid<bool> grid(voxelRes);

    // Create cube
    auto accessor = grid.createAccessor();
    constexpr double cubeLength = 5.0;
    for (double x = -0.5 * cubeLength; x < 0.5 * (cubeLength + voxelRes); x += 0.5 * voxelRes)
    {
        for (double y = -0.5 * cubeLength; y < 0.5 * (cubeLength + voxelRes); y += 0.5 * voxelRes)
        {
            for (double z = 0.0; z < cubeLength + voxelRes; z += 0.5 * voxelRes)
            {
                Bonxai::CoordT coord = grid.posToCoord(x, y, z);
                accessor.setValue(coord, true);
            }
        }
    }

    // Start visualiser
    Bonxai::BonxaiVisualiser vis(grid, getColor);
    vis.start();

    // Update grid, randomly setting cells
    constexpr int sleepIntervalMs = 1000;
    constexpr double randomAreaLength = 40.0;
    constexpr double randomAreaHeight = 20.0;
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<double> xyDist(-0.5 * randomAreaLength, 0.5 * randomAreaLength);
    std::uniform_real_distribution<double> heightDist(0, randomAreaHeight);
    while (true)
    {
        double x = xyDist(rng);
        double y = xyDist(rng);
        double z = heightDist(rng);

        std::cout << x << " " << y << " " << z << std::endl;
        Bonxai::CoordT coord = grid.posToCoord(x, y, z);
        accessor.setValue(coord, true);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepIntervalMs));
    }

    return 0;
}