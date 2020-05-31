
#include "stdafx.h"
#include "CostDistance3D.h"
#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <windows.h>
using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	std::cout << "*********************************************************" << std::endl;
	std::cout << "*-------------------------------------------------------*" << std::endl;
	std::cout << "*------------------- 3D Cost Distance ------------------*" << std::endl;
	std::cout << "*-------------------------------------------------------*" << std::endl;
	std::cout << "*-------------------------------------------------------*" << std::endl;
	std::cout << "*********************************************************" << std::endl;
	std::cout << std::endl;
	cout << "Please enter the number of rows, columns, and layers of input voxter (separated by space):" << endl;
	int tempI, tempJ, tempK;
	while (1){
		cin >> tempI >> tempJ >> tempK;
		if (!cin.fail()){
			break;
		}
		cin.clear();
		cin.sync();
		cout << "Input error. Please re-enter." << endl;
	}
	const int nI = tempI; // Number of rows
	const int nJ = tempJ; // Number of columns
	const int nK = tempK; // Number of layers

	
	// Allocate the heap space for InitialSource, Friction, DirectSource(ShiftNorth, ShiftEast, and ShiftUp) voxters
	// Voxel v's location: (i,j,k)
	// Location of v's direct source s: (i + ShiftNorth[i][j][k], j - ShiftEast[i][j][k], k - ShiftUp[i][j][k])
	double ***InitialSource = new double **[nI];
	for (int i = 0; i < nI; i++)
	{
		InitialSource[i] = new double*[nJ];
		for (int j = 0; j < nJ; j++)
		{
			InitialSource[i][j] = new double[nK];
		}
	}
	double ***Friction = new double **[nI];
	for (int i = 0; i < nI; i++)
	{

		Friction[i] = new double*[nJ];
		for (int j = 0; j < nJ; j++)
		{
			Friction[i][j] = new double[nK];
		}
	}
	int ***ShiftNorth = new int **[nI];
	for (int i = 0; i < nI; i++)
	{
		ShiftNorth[i] = new int*[nJ];

		for (int j = 0; j < nJ; j++)
		{

			ShiftNorth[i][j] = new int[nK];
		}
	}
	int ***ShiftEast = new int **[nI];
	for (int i = 0; i < nI; i++)
	{

		ShiftEast[i] = new int*[nJ];

		for (int j = 0; j < nJ; j++)
		{
			ShiftEast[i][j] = new int[nK];
		}
	}
	int ***ShiftUp = new int **[nI];
	for (int i = 0; i < nI; i++)
	{
		ShiftUp[i] = new int*[nJ];
		for (int j = 0; j < nJ; j++)
		{
			ShiftUp[i][j] = new int[nK];
		}
	}
	// Initialize the DirectSource voxter(ShiftNorth[][][], ShiftEast[][][], and ShiftUp[][][])
	for (int i = 0; i < nI; i++)
	{
		for (int j = 0; j < nJ; j++)
		{
			for (int k = 0; k < nK; k++){

				InitialSource[i][j][k] = -1; // All the voxels default to non-source voxel(-1)
				Friction[i][j][k] = 1; // The friction of each voxel defaults to 1
				ShiftNorth[i][j][k] = 0; // The row distance of each voxel to its direct source(default: 0)
				ShiftEast[i][j][k] = 0; // The column distance of each voxel to its direct source (default: 0)
				ShiftUp[i][j][k] = 0; // The layer distance of each voxel to its direct source (default: 0)
			}
		}
	}
	
	// All the source voxels default to 0
	int SourceNum = 0;
	cout << "Please enter the number of sources:" << endl;
	cin >> SourceNum;
	for (int temp = 1; temp <= SourceNum;temp++){
		cout << "Please enter the row, column, and layer number of source "<< temp << " (separated by space):" << endl;
		while (1){
			cin >> tempI >> tempJ >> tempK;
			if (!cin.fail()){
				break;
			}
			cin.clear();
			cin.sync();
			cout << "Input error. Please re-enter." << endl;
		}
		InitialSource[tempI][tempJ][tempK] = 0;
	}
	
	cout << "Please enter the absolute path of the friction file (use double backslashes to specify a separator):" << endl;
	string FrictionFile = "";
	cin >> FrictionFile;
	ifstream inFriction;
	inFriction.open(FrictionFile);
	if (!inFriction) {
		cerr << "error: unable to open input file: " << endl;
		return -1;
	}
	for (int k = 0; k < nK; k++){
	
		for (int i = 0; i < nI; i++){
		
			for (int j = 0; j < nI; j++){
			
				inFriction >> Friction[i][j][k];

			}
		}
	}
	inFriction.close();
	inFriction.clear();
	cout << "Please enter the maximum distance to be calculated:" << endl;
	double HowFar = 100000.0;
	cin >> HowFar;

	// Start calculation.
	int st = clock();
	cout << "Start time is:" << st << endl;
	double ***CostDistance;
	CostDistance = correctedCD(nI, nJ, nK, HowFar, InitialSource, Friction, ShiftNorth, ShiftEast, ShiftUp);
	int ct = clock();
	cout << "End time is:" << ct << endl;
	cout << "Running time is:" << (ct - st) / 1000.0 << endl;

	// Save CostDistance and DirectSource(ShiftNorth[][][], ShiftEast[][][], and ShiftUp[][][])
	cout << "Please enter the absolute path of output folder (use double backslashes to specify a separator):" << endl;
	string outputFolder = "";
	cin >> outputFolder;
	string cdFilename = outputFolder+"\\CostDistance.txt";
	ofstream outfile(cdFilename);

	for (int k = 0; k < nK; k++){

		for (int i = 0; i < nI; i++){

			for (int j = 0; j < nJ; j++){

				outfile << std::to_string(CostDistance[i][j][k]) << " ";
			}
			outfile << endl;
		}
	}
	outfile.close();
	outfile.clear();
	string northFilename = outputFolder + "\\ShiftNorth.txt";
	ofstream outfilen(northFilename);

	for (int k = 0; k < nK; k++){

		for (int i = 0; i < nI; i++){

			for (int j = 0; j < nJ; j++){

				outfilen << std::to_string(ShiftNorth[i][j][k]) << " ";
			}
			outfilen << endl;
		}
	}
	outfilen.close();
	outfilen.clear();
	string eastFilename = outputFolder + "\\ShiftEast.txt";
	ofstream outfilee(eastFilename);

	for (int k = 0; k < nK; k++){

		for (int i = 0; i < nI; i++){

			for (int j = 0; j < nJ; j++){

				outfilee << std::to_string(ShiftEast[i][j][k]) << " ";
			}
			outfilee << endl;
		}
	}
	outfilee.close();
	outfilee.clear();
	string upFilename = outputFolder + "\\ShiftUp.txt";
	ofstream outfileu(upFilename);

	for (int k = 0; k < nK; k++){

		for (int i = 0; i < nI; i++){

			for (int j = 0; j < nJ; j++){

				outfileu << std::to_string(ShiftUp[i][j][k]) << " ";
			}
			outfileu << endl;
		}
	}
	outfileu.close();
	outfileu.clear();
	cout << "over" << endl;
	delete[] InitialSource;
	delete[] Friction;
	delete[] ShiftNorth;
	delete[] ShiftEast;
	delete[] ShiftUp;
	delete[] CostDistance;
	system("pause");
	return 0;
}