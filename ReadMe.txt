========================================================
3D cost disatance tool
========================================================

-------------------------
Usage
-------------------------
1. You can run the tool by starting the Visual Studio project (CostDistanceC++.sln) in the code folder, or you can run the executable file (CostDistanceTool_V1.0.exe).

2.You can test the tool by example data according to the description files.

3.You can input your own data, and your TXT data files must follow these rules:
------The amount of data in the TXT file must be accordance with your input number of rows, columns and layers.
------The data in TXT file is arranged in order of voxel's column, row and layer in the 3D raster data, and is separated by spaces.

-------------------------
Core files
-------------------------
Files in CostDistanceC++ are the core files. 

-------------------------
Example data
-------------------------
Two sample data and the description of input parameters can be found in the Example dictionary.

-------------------------
Output files
-------------------------
CostDistance.txt------recording the least accumulative cost from each voxel to its nearest initial source
ShiftNorth.txt---------recording each voxel's row distance from its direct source voxel
ShiftEast.txt-----------recording each voxel's column distance from its direct source voxel
ShiftUp.txt------------recording each voxel's layer distance from its direct source voxel
------v: (i,j,k)
------v's direct source s: (i + ShiftNorth[i][j][k], j - ShiftEast[i][j][k], k - ShiftUp[i][j][k])


