### LASToPointCloud
Convert LAZ file to tileset to use in Cesium. Run the program on Linux.

The program can do these task:

1. Change the input file's epsg and geoid

2. Croping the input file

3. Convert the input file from .laz to tileset
   

## Prerequisites

Make sure you have the following packages installed first:

1. A Linux OS

2. [aws-lambda-runtime](https://github.com/awslabs/aws-lambda-cpp#prerequisites)

## Build and run program

In a terminal, run the following commands:

```
$ git clone https://github.com/Ngunhatnha/LASToPointCloud.git
$ cd src
$ mkdir build
$ cd build
$ cmake ../
$ make
```

Add the file you want to convert to the bin folder.Then on the bin folder run the program with correct input. For example:

```
./LASToPointCloud ./file.laz -T 1234,0000,5678,2000
```

And the result file will appear in the bin/Output folder

## How to run the program

The program accept 3 different input based on 3 different task that it can do:

```
-T source_epsg,source_geoId,dest_epsg,dest_geoid  (LASTOLAS change file's epsg and geoId)
-s s3 -C source_epsg,source_geoId,x_min,y_min,x_max,y_max (Cropping)
-s s3 -P source_epsg,source_geoId,cell_size,geometry_scale (convert from LAS to Pnts and make tileset)

```

The program only support geoid N60 and N2000 so input for source_geoId and dest_geoid only accept "60" and "2000" 

If your file don't have geoId, set it to 0000 or leave it empty



For example you want to convert the xxx.laz file having epsg code 3879 and geoId 2000 to tileset having cell size of 15m and geometryscale of 0.1:

```
./LASToPointCloud ./xxx.laz -P 3879,2000,15,0.1 
```



If xxx.laz file having epsg code 4978 and don't have geoId and you need to change it epsg and geoid to 3879 and 60:

```
./LASToPointCloud ./xxx.laz -T 4978,0000,3879,60 
```

