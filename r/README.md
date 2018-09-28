# NDVI Algorithm in R

Implementation of NDVI Algorithm in R.

## Run

To run this NDVI algorithm on your machine, you need the following commands must be executed:

```
./setup
Rscript run.R path/to/input path/to/output
```

The ```path/to/input``` and ```path/to/output``` directories keep the bands (4, 5 and BQA) and meta file of the landsat images. Please, see the example in the ```samples``` directory:

```
Rscript run.R samples/sample1/input samples/sample1/output
```

The input specs are described in the section ```path/to/input```, while the output of the NDVI algorithm execution will be storage in the ```path/to/output``` directory.