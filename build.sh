#!/bin/bash

echo "Building Cloth Simulation..."

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

echo "Configuring with CMake..."
cmake ..

if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    echo "Please ensure you have CMake and required dependencies installed."
    echo "See README.md for installation instructions."
    exit 1
fi

echo "Building project..."
cmake --build .

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Build successful!"
echo ""
echo "To run the simulation:"
echo "  cd build/bin"
echo "  ./ClothSimulation"
echo ""
echo "Controls:"
echo "  R - Reset cloth"
echo "  Space - Toggle render mode"
echo "  W - Toggle wireframe"
echo "  Up/Down - Adjust constraint iterations"
echo ""

