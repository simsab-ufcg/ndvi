#include <iostream>
#include <dirent.h>
#include <vector>
#include <algorithm>
#include "readMeta.h"
#include "readSunEarth.h"
#include "landsat/landsat_function.h"
#include "landsat/landsat8.h"
#include "landsat/landsat7.h"
#include "landsat/landsat5.h"

using namespace std;

int setMask(int number_sensor){
    if(number_sensor != 8) return 672;
    else return 2720;
}

bool analisyShadow(string path_tiff_band_bqa, int number_sensor){
    int fmask = setMask(number_sensor);

    Tiff bandBqa = TIFFOpen(path_tiff_band_bqa.c_str(), "rm");

    uint32 heightTiffBqa, widthTiffBqa;

    TIFFGetField(bandBqa, TIFFTAG_IMAGELENGTH, &heightTiffBqa);
    TIFFGetField(bandBqa, TIFFTAG_IMAGEWIDTH, &widthTiffBqa);

    ldouble *line = (ldouble*) malloc(sizeof(ldouble) * widthTiffBqa);

    long long quantPixelsInvalid = 0;

    for(int i = 0; i < heightTiffBqa; i++){
        TIFFReadScanline(bandBqa, line, i, 1);
        for(int j = 0; j < widthTiffBqa; j++){
            if(line[j] != fmask) quantPixelsInvalid++;
        }
    }

    return ((ldouble)quantPixelsInvalid/(heightTiffBqa*widthTiffBqa)) >= 0.99;
}

LandsatFunction* setLandsatFunction(int number_sensor, ldouble sun_elevation, ldouble dist_sun_earth){
    if(number_sensor == 8) return new Landsat8(sun_elevation);
    if(number_sensor == 7) return new Landsat7(sun_elevation, dist_sun_earth);
    if(number_sensor == 5) return new Landsat5(sun_elevation, dist_sun_earth);
}

int main(int argc, char *argv[]){
    
    //lambdas
    auto validDir = [](string directory){
        if(directory.size() >= 1 && directory.back() != '/')
            return directory + '/';
        return directory;
    };

    auto endsWith = [](string text, string pattern){
        bool result = false;
        if(text.size() > pattern.size()){ 
        string match = text.substr(text.size() - pattern.size(), pattern.size());
        if(match == pattern)
            result = true;
        }
        return result;
    };

    auto readFiles = [](DIR* dir){
        vector<string> fileNames;
        struct dirent * directory;
        while((directory = readdir(dir)) != NULL){
            string fileName(directory->d_name);
            fileNames.push_back(fileName);
        }
        return fileNames;
    };

    auto filterDirectoryFiles = [readFiles, endsWith](string directoryName, string suffix){
        DIR* dirp = opendir(directoryName.c_str());
        vector<string> files;
        if(dirp){
            files = readFiles(dirp);
            
            auto iterator = remove_if(files.begin(), files.end(), [endsWith, suffix](string fileName){
                for(int i=0; i<fileName.size(); i++) fileName[i] = tolower(fileName[i]);
                return !endsWith(fileName, suffix);
            });
            
            files.erase(iterator, files.end());

        } else {
            cerr << "Directory " << directoryName.substr(0, directoryName.size() - 1) << " not found" << endl;
            exit(0);
        }
        
        closedir(dirp);
        return files;
    };

    //consts
    const string INPUT_FILE_TIFF_SUFFIX = ".tif";
    const string INPUT_FILE_META_SUFFIX = ".txt";

    const int POS_BAND_4 = 0;
    const int POS_BAND_5 = 1;
    const int POS_BAND_BQA = 2;
    const int POS_META_FILE = 0;

    const int INPUT_DIRECTORY_INDEX = 1;
    const int OUTPUT_DIRECTORY_INDEX = 2;

    //valid output directory
    const string PATH_TO_OUTPUT_DIRECTORY(validDir(argv[OUTPUT_DIRECTORY_INDEX]));

    //valid arguments
    if(argc < 2 || argc > 4){
        cerr << "Invalid arguments";
        exit(0);
    }

    //variables
    vector<string> tiffFiles, metaFile;

    //input directory (tiff and meta)
    string inputDirectory = validDir(argv[INPUT_DIRECTORY_INDEX]);
    tiffFiles = filterDirectoryFiles(inputDirectory, INPUT_FILE_TIFF_SUFFIX);
    metaFile = filterDirectoryFiles(inputDirectory, INPUT_FILE_META_SUFFIX);

    //valid inputs
    if(tiffFiles.size() < 3 || !metaFile.size()){
        cerr << "Missing arguments";
        exit(0);
    }

    //load meta file
    string path_meta_file = inputDirectory + metaFile[POS_META_FILE];
    ReadMeta readerMeta = ReadMeta(path_meta_file);
    ldouble sun_elevation = readerMeta.getSunElevation();
    int number_sensor = readerMeta.getNumberSensor();
    int julian_day = readerMeta.getJulianDay();
    int year = readerMeta.getYear();

    //load distance between sun and earth
    string path_d_sun_earth = "./assets/d_sun_earth";
    ReadSunEarth readerSunEarth = ReadSunEarth(path_d_sun_earth);
    ldouble dist_sun_earth = readerSunEarth.getDistance(julian_day);

    //output directory
    string outputDirectory(validDir(argv[OUTPUT_DIRECTORY_INDEX]));

    //sorting tiff files
    sort(tiffFiles.begin(), tiffFiles.end());

    //verify quantity snow and shadows
    string path_tiff_band_bqa = inputDirectory + tiffFiles[POS_BAND_BQA];
    if(analisyShadow(path_tiff_band_bqa, number_sensor)){
        cerr << "Image invalid";
        exit(0);
    }

    //load band 4 (tiff)
    string path_tiff_band_4 = inputDirectory + tiffFiles[POS_BAND_4];
    Tiff band4 = TIFFOpen(path_tiff_band_4.c_str(), "rm");

    //load band 5 (tiff)
    string path_tiff_band_5 = inputDirectory + tiffFiles[POS_BAND_5];
    Tiff band5 = TIFFOpen(path_tiff_band_5.c_str(), "rm");

    //load tiff ndvi
    string path_output_tiff_ndvi = outputDirectory + "ndvi.tif"; 
    Tiff ndvi = TIFFOpen(path_output_tiff_ndvi.c_str(), "w8");

    //process NDVI
    LandsatFunction* landsat;
    landsat = setLandsatFunction(number_sensor, sun_elevation, dist_sun_earth);
    (*landsat).processNDVI(band4, band5, ndvi);

    return 0;
}