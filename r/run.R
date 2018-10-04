########################################################################################
#                                                                                      #
#                         EU BRAZIL Cloud Connect                                      #
#                                                                                      #
#                                                                                      #
########################################################################################

rm(list=ls())

# Import library
library(raster)
library(maptools)
library(ncdf4)

# Capture the args and validate
args = commandArgs(trailingOnly=TRUE)
if(length(args) < 1 || length(args) > 3){
  print("Missing arguments...")
  quit("no", 1, FALSE)
}

# Import logger
source("src/logger.R")

# Constants
INPUT_DIRECTORY_INDEX <- 1
OUTPUT_DIRECTORY_INDEX <- 2

# File that stores the Image Directories (TIFs, MTL, FMask)
input_path <- args[INPUT_DIRECTORY_INDEX]

# Valid input
if(length(list.files(path = input_path, pattern = "*.TIF|*.tif", full.names = TRUE)) < 3 | length(list.files(path = input_path, pattern = "*.mtl|*.MTL", full.names = TRUE)) < 1){
  print("Missing input data...")
  quit("no", 1, FALSE)
}

# Set paths (meta and output)
meta_path <- list.files(path = input_path, pattern = "*mtl.txt|*MTL.txt", full.names = TRUE)
output_path<-paste(args[OUTPUT_DIRECTORY_INDEX], "NDVI" , ".tif", sep="")

# Load the source code in landsat.R to this code
source("src/landsat.R")

# Import basics configurations
source("src/conf.R")

# Reading image file
tiff_files <- list.files(path = input_path, pattern = "*.TIF|*.tif", full.names = TRUE)

# Capture bands
getBandsPath <- function(number_sensors){
  wanted_bands <- c("b4", "b5", "bqa")

  bands_path <- list()
  for (i in 1:length(wanted_bands)) {
    for (j in 1:length(tiff_files)) {
      if(regexpr(paste(wanted_bands[i], '.tif', sep=""), tolower(tiff_files[[j]])) != -1) {
        bands_path[[i]] <- tiff_files[[j]]
      }
    }
  }
  return(bands_path)
}

# Reading bands 4, 5 and bqa
bands <- stack(as.list(getBandsPath(number_sensors)))

logger("Preprocess end")

################### Identifier of clouds and shadows ##################################

n_fmask <- nlayers(bands)
Fmask <- bands[[n_fmask]]
fmask <- as.vector(Fmask)

setMaskFilter <- function(number_sensors){
  if(number_sensors != 8) return(672)
  else return(2720)
}

mask_filter <- setMaskFilter(number_sensors)

contPixels <- 0
for(i in fmask){
   if(i == mask_filter){
	  contPixels <- contPixels + 1
   }
}

if ((contPixels / (bands@ncols * bands@nrows)) <= 0.01) { 
  print("Image compatible for processing, more than 99% cloud and cloud shadow")
  quit("no", 1, FALSE)
}

################## Set NaN in pixels covered ##################################

for (i in 1:nlayers(bands)) {
  f <- bands[[i]][]
  f[fmask != mask_filter] <- NaN
  bands[[i]][] <- f 
}

logger("AnalisyTiff end")

################## Calculating NDVI ##################################

# Landsat Function
output <- landsat()

logger("NDVICalc end")

################## Write NDVI Tiff ##################################

writeRaster(output, output_path, overwrite=TRUE, format="GTiff", varname=landsat_scene_id, varunit="daily", longname=landsat_scene_id, xname="lon", yname="lat", bylayer=TRUE, suffix="names")
logger("WriteRaster end")
