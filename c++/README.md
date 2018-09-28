# NDVI Algorithm in C++

Implementation of NDVI Algorithm in C++ for better performance.

# Build/Run

## Build

To build our project just run ```make``` command in the root directory.

## Run

To run this NDVI algorithm on your machine, you need the following commands must be executed:

```
./setup
./run path/to/input path/to/output
```

The ```path/to/input``` and ```path/to/output``` directories keep the bands (4, 5 and BQA) and meta file of the landsat images. Please, see the example in the ```samples``` directory:

```
./run samples/sample1/input samples/sample1/output
```

The input specs are described in the section ```path/to/input```, while the output of the NDVI algorithm execution will be storage in the ```path/to/output``` directory.