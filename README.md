# BonxaiVisualiser

Minimal visualiser for facontidavide's Bonxai, an implementation of the VDB voxel grid structure. The visualiser is a single-header include, and provides live visualisation of the VDB voxels with simple camera movements.

## Getting Started

### Dependencies
* [Bonxai](https://github.com/facontidavide/Bonxai/tree/main)
* [raylib](https://github.com/raysan5/raylib)
* [raygui](https://github.com/raysan5/raygui)

### Installing

* Clone this repo and its dependencies.
```
git clone --recurse-submodules https://github.com/chiaxr/BonxaiVisualiser.git
```
* Refer to CMakeLists.txt for example inclusion in your CMake project.

### Usage
The visualiser is instantiated with a reference to the grid and a colouring function.
```cpp
// Init grid
constexpr double voxelRes = 1.0;
Bonxai::VoxelGrid<bool> grid(voxelRes);

// Init visualiser
Bonxai::BonxaiVisualiser<bool> vis(grid, getColor);

// Opens visualiser window and begins rendering contents of grid
vis.start();
```

The coloring function is applied to each voxel value in the grid, and must return a RGBA tuple.
```cpp
std::tuple<unsigned char, unsigned char, unsigned char, unsigned char> getColor(const bool& data)
{
    if (data)
    {
        // Translucent green if voxel value is true
        return { 0, 255, 0, 30 };
    }
    else
    {
        // Transparent otherwise
        return { 0, 0, 0, 0 };
    }
};
```