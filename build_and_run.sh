#!/bin/bash

# Define the project directory
PROJECT_DIR="/home/ashish/dericonsole"
BUILD_DIR="$PROJECT_DIR/build"

# Function to handle errors
handle_error() {
    echo "Error: $1" >&2
    exit 1
}

# Navigate to the project directory
cd "$PROJECT_DIR" || handle_error "Failed to navigate to $PROJECT_DIR"

# Remove the build directory if it exists
if [ -d "$BUILD_DIR" ]; then
    echo "Removing existing build directory..."
    rm -rf "$BUILD_DIR" || handle_error "Failed to remove $BUILD_DIR"
fi

# Create the build directory
echo "Creating build directory..."
mkdir -p "$BUILD_DIR" || handle_error "Failed to create $BUILD_DIR"

# Navigate to the build directory
cd "$BUILD_DIR" || handle_error "Failed to navigate to $BUILD_DIR"

# Run CMake and build the project
echo "Running CMake..."
cmake -DCMAKE_BUILD_TYPE=Debug .. || handle_error "CMake failed"

echo "Building the project..."
cmake --build . || handle_error "Build failed"

# Run the executable
echo "Running the executable..."
./DeriConsole || handle_error "Failed to run the executable"

echo "Script completed successfully!"