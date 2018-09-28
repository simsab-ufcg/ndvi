
######################### Reading sensor parameters #####################################

paramSensorTM1 <- read.csv("src/sensor/sensorTM1.csv", sep=";", stringsAsFactors=FALSE)
paramSensorTM2 <- read.csv("src/sensor/sensorTM2.csv", sep=";", stringsAsFactors=FALSE)
paramSensorETM <- read.csv("src/sensor/sensorETM.csv", sep=";", stringsAsFactors=FALSE)
paramSensorLC <- read.csv("src/sensor/sensorLC.csv", sep=";", stringsAsFactors=FALSE)

# Function to set the sensor parameters

setParamSensor <- function(number_sensors, year){
    if (number_sensors == 8) return(paramSensorLC)
    if (number_sensors == 7) return(paramSensorETM)
    if (number_sensors == 5){
        if(year < 1992) return(paramSensorTM1)
        if(year > 1992) return(paramSensorTM2)
    }
}
