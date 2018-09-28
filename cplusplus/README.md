# NDVI Algorithm in C++

Implementation of NDVI Algorithm in C++ for better performance.

# Build/Run

## Build

To build our project just run ```make``` command in the root directory.

## Run

To run this NDVI algorithm on your machine, you need the following commands must be executed:

```
./setup
./run path/to/band4 path/to/band5 path/to/bandbqa path/to/mtl
```

The ```path/to/band4 path/to/band5 path/to/bandbqa path/to/mtl``` path keep the bands (4, 5 and BQA) and meta file of the landsat images. Please, see the example:

```
./run b4.tif b5.tif bqa.tif mtl.txt
```
