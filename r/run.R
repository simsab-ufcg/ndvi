########################################################################################
#                                                                                      #
#                         EU BRAZIL Cloud Connect                                      #
#                                                                                      #
#                                                                                      #
########################################################################################

options(echo=TRUE) 
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
source("assets/logger.R")

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
source("assets/landsat2.R")

# Import basics configurations
source("assets/conf.R")

# Reading image file
# The Images are of the type ".tif" that represents each spectral band captured by the satellite
# Depending on the sattelite the number of spectral bands captured are differents
tiff_files <- list.files(path = input_path, pattern = "*.TIF|*.tif", full.names = TRUE)

getBandsPath <- function(number_sensors){
  wanted_bands <- c("b4", "b5")

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

# Reading bands 4 e 5
bands <- stack(as.list(getBandsPath(number_sensors)))

logger("Preprocess end")

################## Fmask ##################################

# Identifier of clouds and shadows
# The FMask serves to identify if exist any cloud or shadow on the image, the existence of clouds or shadow disturbs the results.

n.fmask <- length(tiff_files)
Fmask <- raster(tiff_files[[n.fmask]])
fmask <- as.vector(Fmask)

setMaskFilter <- function(number_sensors){
  if(number_sensors != 8) return(672)
  else return(2720)
}

mask_filter <- setMaskFilter(number_sensors)


for (i in 1:nlayers(bands)) {
  f <- bands[[i]][]
  f[fmask != mask_filter] <- NaN
  bands[[i]][] <- f 
}

if (0.99 <= (sum(is.na(values(bands))) / 7) / (bands@ncols * bands@nrows)) { 
  print("Image compatible for processing, more than 99% cloud and cloud shadow")
  quit("no", 1, FALSE)
}

logger("AnalisyTiff end")

# Changing the projection of the images ( UTM to GEO)
# This operation can be done in a parallel way by Clusters, projectRaster is implemented to naturally be executed by clusters
# The number of used clusters is given by the 'clusters' constant


beginCluster(number_clusters)
bands <- projectRaster(bands, crs = WGS84)
endCluster()

logger("LoadTiffs end")

################## Bounding Box ##################################

# The Bounding Box area that is important and has less noise in the Image
bounding_boxes_path <- "assets/wrs2_asc_desc/wrs2_asc_desc_recorte.shp"
bounding_boxes <- readShapePoly(bounding_boxes_path, proj4string=CRS(WGS84))
bounding_box <- bounding_boxes[bounding_boxes@data$WRSPR == WRSPR, ]

################## Elevation ##################################

# Read the File that stores the Elevation of the image area, this influence on some calculations
tif_elevation <- "assets/elevation/srtm_29_14.tif"
raster_elevation <- raster(tif_elevation)
raster_elevation <- crop(raster_elevation, extent(bounding_box))

# Setting the raster elevation resolution as equals to the Fmask raster resolution
raster_elevation_auxiliar <- raster(raster_elevation)

# The raster elevation aux resolution is the same of raster fmask
res(raster_elevation_auxiliar) <- res(bands)

# Resample images
beginCluster(number_clusters)
raster_elevation <- resample(raster_elevation, raster_elevation_auxiliar, method="ngb")
endCluster()

logger("LoadRasterElevation end")

################## Resampling satellite bands images ##################################

# This block of code resample the image based on the Elevation of the terrain captured by the sat
# The Elevation of the terrain needs to be taken into account
# This block is already Clustered

beginCluster(number_clusters)
image_rec <- resample(bands, raster_elevation, method="ngb")
endCluster()

logger("ResampleImage end")

################## Calculating NDVI ##################################

# Landsat Function
output <- landsat()

logger("NDVICalc end")

################## Masking landsat rasters output #####################################

# This block mask the values in the landsat output rasters that has cloud cells and are inside the Bounding Box required
# This block is already Clustered

#output <- mask(output, bounding_box)
logger("MaskValues end")

################## Write NDVI Tiff ##################################

writeRaster(output, output_path, overwrite=TRUE, format="GTiff", varname=landsat_scene_id, varunit="daily", longname=landsat_scene_id, xname="lon", yname="lat", bylayer=TRUE, suffix="names")
logger("WriteRaster end")
