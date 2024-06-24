#include "bonxai/bonxai.hpp"

#include "raylib.h"
#include "rlgl.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <atomic>
#include <chrono>
#include <future>
#include <thread>
#include <tuple>

namespace Bonxai
{
template <typename DataT>
class BonxaiVisualiser
{
public:
    BonxaiVisualiser(
        const VoxelGrid<DataT>& grid,
        std::function<std::tuple<unsigned char, unsigned char, unsigned char, unsigned char>(const DataT&)> func)
        : mGrid(grid), mGetColor(func)
    {}

    ~BonxaiVisualiser()
    {
        mRunning = false;
    }

    // Starts live visualisation of Bonxai grid
    void start();

private:
    void run();
    void renderGrid();
    void renderCell(const DataT& data, const Point3D& pos);
    Vector3 toVisFrame(const float x, const float y, const float z);
    void drawAxes();
    void drawArrow(const Color& color);

    std::atomic_bool mRunning;
    std::future<void> mVisThread;
    const VoxelGrid<DataT>& mGrid;
    std::function<std::tuple<unsigned char, unsigned char, unsigned char, unsigned char>(const DataT&)> mGetColor;
};

template <typename DataT>
void BonxaiVisualiser<DataT>::start()
{
    mRunning = true;
    mVisThread = std::async(std::launch::async, &BonxaiVisualiser::run, this);
}

template <typename DataT>
void BonxaiVisualiser<DataT>::run()
{
    // Init
    constexpr int screenWidth = 1280;
    constexpr int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "BonxaiVisualiser");

    // Define the camera to look into our 3d world
    Camera3D camera;
    camera.position = { 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = { 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = { 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                       // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;    // Camera projection type

    constexpr int fps = 30.0;
    SetTargetFPS(fps);

    // Main game loop
    while (mRunning && !WindowShouldClose()) // Detect window close button or ESC key
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            UpdateCamera(&camera, CAMERA_FREE);
        }
        
        // Reset camera view
        if (IsKeyPressed('Z'))
        {
            camera.target = { 0.0f, 0.0f, 0.0f };
        }

        // Draw
        BeginDrawing();

            ClearBackground(DARKGRAY);

            BeginMode3D(camera);

                DrawGrid(100, 1.0f);
                drawAxes();
                renderGrid();

            EndMode3D();
        EndDrawing();
    }

    // Clean up
    CloseWindow(); // Close window and OpenGL context
}

template <typename DataT>
void BonxaiVisualiser<DataT>::renderGrid()
{
    // Adapted from Bonxai - VoxelGrid<DataT>::forEachCell(VisitorFunction func)
    const int32_t MASK_LEAF = ((1 << mGrid.LEAF_BITS) - 1);
    const int32_t MASK_INNER = ((1 << mGrid.INNER_BITS) - 1);

    for (const auto& map_it : mGrid.root_map)
    {
        const auto& [xA, yA, zA] = (map_it.first);
        const VoxelGrid<DataT>::InnerGrid& inner_grid = map_it.second;
        auto& mask2 = inner_grid.mask();

        for (auto inner_it = mask2.beginOn(); inner_it; ++inner_it)
        {
            const int32_t inner_index = *inner_it;
            const int32_t INNER_BITS_2 = mGrid.INNER_BITS * 2;
            // clang-format off
            int32_t xB = xA | ((inner_index & MASK_INNER) << mGrid.LEAF_BITS);
            int32_t yB = yA | (((inner_index >> mGrid.INNER_BITS) & MASK_INNER) << mGrid.LEAF_BITS);
            int32_t zB = zA | (((inner_index >> (INNER_BITS_2)) & MASK_INNER) << mGrid.LEAF_BITS);
            // clang-format on

            auto& leaf_grid = inner_grid.cell(inner_index);
            auto& mask1 = leaf_grid->mask();

            for (auto leaf_it = mask1.beginOn(); leaf_it; ++leaf_it)
            {
                const int32_t leaf_index = *leaf_it;
                const int32_t LEAF_BITS_2 = mGrid.LEAF_BITS * 2;
                CoordT coord = { xB | (leaf_index & MASK_LEAF),
                            yB | ((leaf_index >> mGrid.LEAF_BITS) & MASK_LEAF),
                            zB | ((leaf_index >> (LEAF_BITS_2)) & MASK_LEAF) };
                
                // render each cell
                renderCell(leaf_grid->cell(leaf_index), CoordToPos(coord, mGrid.resolution));
            }
        }
    }
}

template <typename DataT>
void BonxaiVisualiser<DataT>::renderCell(const DataT& data, const Point3D& pos)
{
    auto color = mGetColor(data);
    DrawCube(
        toVisFrame(static_cast<float>(pos.x), static_cast<float>(pos.y), static_cast<float>(pos.z)),
        static_cast<float>(mGrid.resolution),
        static_cast<float>(mGrid.resolution),
        static_cast<float>(mGrid.resolution),
        { std::get<0>(color), std::get<1>(color), std::get<2>(color), std::get<3>(color) }
    );
}

template <typename DataT>
Vector3 BonxaiVisualiser<DataT>::toVisFrame(const float x, const float y, const float z)
{
    return {x, z, -y};
}

template <typename DataT>
void BonxaiVisualiser<DataT>::drawAxes()
{
    // x-axis
    {
        rlPushMatrix();
        Vector3 rotationAxis = toVisFrame(0.0f, 1.0f, 0.0f);
        rlRotatef(90.0f, rotationAxis.x, rotationAxis.y, rotationAxis.z);
        drawArrow(RED);
        rlPopMatrix();
    }

    // y-axis
    {
        rlPushMatrix();
        Vector3 rotationAxis = toVisFrame(-1.0f, 0.0f, 0.0f);
        rlRotatef(90.0f, rotationAxis.x, rotationAxis.y, rotationAxis.z);
        drawArrow(GREEN);
        rlPopMatrix();
    }

    // z-axis
    // No need for rotation
    drawArrow(BLUE);
}

template <typename DataT>
void BonxaiVisualiser<DataT>::drawArrow(const Color& color)
{
    constexpr float axesRadius = 0.07f;
    constexpr float axesLength = 1.0f;
    constexpr float arrowTipRadius = 0.01f;
    constexpr float arrowRadius = 0.15f;
    constexpr float arrowLength = 0.2f;
    constexpr int cylinderSlices = 8;

    DrawCylinder(
        { 0.0f, 0.0f, 0.0f },
        axesRadius,
        axesRadius,
        axesLength,
        cylinderSlices,
        color
    );
    DrawCylinder(
        toVisFrame(0.0f, 0.0f, 1.0f),
        arrowTipRadius,
        arrowRadius,
        arrowLength,
        cylinderSlices,
        color
    );
}
} // namespace Bonxai