
# Number of clusters used in image processing
number_clusters <- 1

# Read relative distance from Sun to Earth
load("assets/d_sun_earth.RData")

# Set projection and spatial resolution
WGS84 <- "+proj=longlat +datum=WGS84 +ellps=WGS84"

######################### Image Information ######################################

meta_file <- read.table(meta_path, skip=0, nrows=140, sep="=", quote = "''", as.is=TRUE)   #Reading meta_file File
landsat_scene_id <- substr(meta_file$V2[meta_file$V1 == grep(pattern="LANDSAT_SCENE_ID", meta_file$V1, value=T)], 3, 23)

# Load some data
number_sensors <- as.numeric(substr(landsat_scene_id, 3, 3))    #Sensor Number
WRSPR <- substr(landsat_scene_id, 4, 9)                         #WRSPR
year <- as.numeric(substr(landsat_scene_id, 10, 13))            #Images year
julianDay <- as.numeric(substr(landsat_scene_id, 14, 16))	    #Julian Day

# Reading meta_file File for Sensor number 8
if (number_sensors == 8) meta_file <- read.table(meta_path, skip=0, nrows=-1, sep="=", quote="''", as.is=TRUE, fill=TRUE)

# Getting the sum elevation at the time of Image Capture
sun_elevation <- as.numeric(meta_file$V2[meta_file$V1 == grep(pattern="SUN_ELEVATION", meta_file$V1, value=TRUE)])
costheta <- sin(sun_elevation * pi / 180) # From SUN ELEVATION

# Import data from the sensor parameters
source("assets/parameters.R")

# Setting the sensor parameter by the Sattelite sensor type and data
param_sensor = setParamSensor(number_sensors, year)
