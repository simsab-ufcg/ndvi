landsat <- function(){
  if (number_sensors < 8){
    # Radiance
    radiance <- list()

    for(i in 1:2){
      radiance[[i]] <- bands[[i]][] * param_sensor$Grescale[i] + param_sensor$Brescale[i]
      radiance_subset <- radiance[[i]] < 0 & !is.na(radiance[[i]])
      radiance[[i]][radiance_subset] <- 0
    }
    
    # Reflectance
    reflectance <- list()
    for(i in 1:2){
      reflectance[[i]] <- pi * radiance[[i]] * d_sun_earth$dist[julianDay]^2 / (param_sensor$ESUN[i] * costheta)
    }
  } else {
    # Reflectance
    reflectance <- list()
    for(i in 1:2){
      reflectance[[i]] <- (bands[[i]][] * 0.00002 - 0.1) / costheta
    }
  } 
  
  NDVI_temp<-(reflectance[[2]] - reflectance[[1]]) / (reflectance[[2]] + reflectance[[1]])

  NDVI <- Fmask
  NDVI[]<-NDVI_temp

  return(NDVI)
}
