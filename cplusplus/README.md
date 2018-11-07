# NDVI Algorithm in C++

Implementation of NDVI Algorithm in C++ for better performance.

# Build/Run

## Build

To build our project just run ```make``` command in the root directory.

## Run

To run this NDVI algorithm on your machine, you need the following commands must be executed:

```
./setup
./run path/to/bandred path/to/bandnir path/to/bandbqa path/to/mtl path/to/output_ndvi
```

The input bands depend on the type of landsat that will be processed. In the following table are the bands utilized for each Landsat.

| Type of Landsat | Red Band | Nir Band |
| --------------- | -------- | -------- |
| Landsat 8 | Band 4 | Band 5 |
| Landsat 7 | Band 3 | Band 4 |
| Landsat 4-5 | Band 3 | Band 4 |


The ``` path/to/bandred path/to/bandnir path/to/bandbqa path/to/mtl``` path keep the bands (RED, NIR and BQA) and meta file of the landsat images. Please, see the examples:

```
./run b4.tif b5.tif bqa.tif mtl.txt ndvi_landsat_8.tif
./run b3.tif b4.tif bqa.tif mtl.txt ndvi_landsat_7.tif
./run b3.tif b4.tif bqa.tif mtl.txt ndvi_landsat_4_5.tif
```
