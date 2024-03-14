#!/bin/bash

# Directory containing test folders
testing_folder="test"
build_folder="build"
# Compile the program
make

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo "Compilation failed. Exiting."
    exit 1
fi

# Run tests in each test folder
for test_folder in "$testing_folder"/*; do
    if [ -d "$test_folder" ]; then
        echo "Running tests in folder: $test_folder"
        ./$build_folder/matMultp.o "$test_folder"/a "$test_folder"/b "$test_folder"/c
        echo "---------------------------------------------------------"
        echo "---------------------------------------------------------"
    fi
done

echo "All tests completed."
