
######################### Reading sensor parameters #####################################

paramSensorTM <- read.csv("/home/thiagao/workspace/ndvi-gen/r/src/sensor/sensorTM.csv", sep=";", stringsAsFactors=FALSE)
paramSensorETM <- read.csv("/home/thiagao/workspace/ndvi-gen/r/src/sensor/sensorETM.csv", sep=";", stringsAsFactors=FALSE)
paramSensorLC <- read.csv("/home/thiagao/workspace/ndvi-gen/r/src/sensor/sensorLC.csv", sep=";", stringsAsFactors=FALSE)

# Function to set the sensor parameters

setParamSensor <- function(number_sensors){
    if (number_sensors == 8) return(paramSensorLC)
    if (number_sensors == 7) return(paramSensorETM)
    if (number_sensors == 5) return(paramSensorTM)
}
