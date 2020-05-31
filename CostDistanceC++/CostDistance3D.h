#include "xlHeaps.h"
#include "xlHashTable.h"
#include "Node.h"

#ifndef COSTDISTANCE3D_H_
#define COSTDISTANCE3D_H_

//// 'A fast voxel traversal algorithm for ray tracing (John Amanatides, 1987)'
//// Determine if the propagation from the source voxel to the adjacent voxel is blocked by a voxel with different friction
bool IsBlocked(int ActiveRow, int ActiveColumn, int ActiveLayer, int InwardShiftNorth, int InwardShiftEast, int InwardShiftUp, int NeighborRow, int NeighborColumn, int NeighborLayer, double ***Friction){
	bool Blockage = 0;
	int x1 = int(ActiveRow + InwardShiftNorth);
	int y1 = int(ActiveColumn - InwardShiftEast);
	int z1 = int(ActiveLayer - InwardShiftUp);
	int	x0 = NeighborRow;
	int	y0 = NeighborColumn;
	int	z0 = NeighborLayer;
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
	int dz = abs(z1 - z0);
	int stepX = x0 < x1 ? 1 : -1;
	int stepY = y0 < y1 ? 1 : -1;
	int stepZ = z0 < z1 ? 1 : -1;
	double hypotenuse = sqrt(pow(dx, 2) + pow(dy, 2) + pow(dz, 2));
	double tMaxX = hypotenuse*0.5 / dx;
	double tMaxY = hypotenuse*0.5 / dy;
	double tMaxZ = hypotenuse*0.5 / dz;
	double tDeltaX = hypotenuse / dx;
	double tDeltaY = hypotenuse / dy;
	double tDeltaZ = hypotenuse / dz;
	while (x0 != x1 || y0 != y1 || z0 != z1){
		if (tMaxX < tMaxY) {
			if (tMaxX < tMaxZ) {
				x0 = x0 + stepX;
				tMaxX = tMaxX + tDeltaX;
			}
			else if (tMaxX > tMaxZ){
				z0 = z0 + stepZ;
				tMaxZ = tMaxZ + tDeltaZ;
			}
			else{
				x0 = x0 + stepX;
				tMaxX = tMaxX + tDeltaX;
				z0 = z0 + stepZ;
				tMaxZ = tMaxZ + tDeltaZ;
			}
		}
		else if (tMaxX > tMaxY){
			if (tMaxY < tMaxZ) {
				y0 = y0 + stepY;
				tMaxY = tMaxY + tDeltaY;
			}
			else if (tMaxY > tMaxZ){
				z0 = z0 + stepZ;
				tMaxZ = tMaxZ + tDeltaZ;
			}
			else{
				y0 = y0 + stepY;
				tMaxY = tMaxY + tDeltaY;
				z0 = z0 + stepZ;
				tMaxZ = tMaxZ + tDeltaZ;
			}
		}
		else{
			if (tMaxY < tMaxZ) {
				y0 = y0 + stepY;
				tMaxY = tMaxY + tDeltaY;
				x0 = x0 + stepX;
				tMaxX = tMaxX + tDeltaX;
			}
			else if (tMaxY > tMaxZ){
				z0 = z0 + stepZ;
				tMaxZ = tMaxZ + tDeltaZ;
			}
			else{
				x0 = x0 + stepX;
				tMaxX = tMaxX + tDeltaX;
				y0 = y0 + stepY;
				tMaxY = tMaxY + tDeltaY;
				z0 = z0 + stepZ;
				tMaxZ = tMaxZ + tDeltaZ;
			}
		}
		if (Friction[x0][y0][z0] != Friction[NeighborRow][NeighborColumn][NeighborLayer]){
			Blockage = 1;
			break;
		}
	}
	return Blockage;
}


//// Calculate 3D least cost
// HowManyRows, HowManyColumns, and HowManyLayers: the size of the input data
// HowFar: maximum distance to be calculated
// InitialSource[][][]: source voxels(>=0), non-source voxels(-1)
// Friction[][][]: the travel cost per unit distance within the voxel (positive)
// ShiftNorth[][][], ShiftEast[][][], and ShiftUp[][][]:the row, column, and layer distance to its direct source
double ***correctedCD(int HowManyRows, int HowManyColumns, int HowManyLayers, double HowFar, double ***InitialSource, double ***Friction, int ***ShiftNorth, int ***ShiftEast, int ***ShiftUp){
	double SqRtOf2 = sqrt(2);
	double SqRtOf3 = sqrt(3);
	double DistanceToNeighbor[26] = { 1, 1, 1, 1, 1, 1, SqRtOf2, SqRtOf2, SqRtOf2, SqRtOf2, SqRtOf2, SqRtOf2, SqRtOf2, SqRtOf2, SqRtOf2, SqRtOf2, SqRtOf2, SqRtOf2, SqRtOf3, SqRtOf3, SqRtOf3, SqRtOf3, SqRtOf3, SqRtOf3, SqRtOf3, SqRtOf3 };
	int RowShift[26] = { 0, -1, 0, 1, 0, 0, -1, 0, 1, 0, -1, -1, 1, 1, -1, 0, 1, 0, -1, -1, 1, 1, -1, -1, 1, 1 };
	int ColumnShift[26] = { 0, 0, 1, 0, -1, 0, 0, 1, 0, -1, -1, 1, 1, -1, 0, 1, 0, -1, -1, 1, 1, -1, -1, 1, 1, -1 };
	int LayerShift[26] = { 1, 0, 0, 0, 0, -1, 1, 1, 1, 1, 0, 0, 0, 0, -1, -1, -1, -1, 1, 1, 1, 1, -1, -1, -1, -1 };
	int InwardShiftNorth = 0;//row distance of min-cost voxel to it's source
	int InwardShiftEast = 0;//column distance of min-cost voxel to it's source
	int InwardShiftUp = 0;//layer distance of min-cost voxel to it's source
	double ActiveCost = 1.0;
	double NewDistance = 1.0;
	int NeighborShiftNorth;
	int NeighborShiftEast;
	int NeighborShiftUp;
	double NeighborDistance;
	xlMinHeap<Node> ActiveList(HowManyColumns*HowManyRows*HowManyLayers / 10);
	// step 1: Initialize the CostDistance voxter
	double ***CostDistance = new double **[HowManyRows];
	for (int i = 0; i < HowManyRows; i++)
	{
		CostDistance[i] = new double*[HowManyColumns];
		for (int j = 0; j < HowManyColumns; j++)
		{
			CostDistance[i][j] = new double[HowManyLayers];
		}
	}
	for (int i = 0; i < HowManyRows; i++){

		for (int j = 0; j < HowManyColumns; j++){

			for (int k = 0; k < HowManyLayers; k++){

				CostDistance[i][j][k] = 0.0;
			}
		}
	}
	for (int ThisRow = 0; ThisRow < HowManyRows; ThisRow++){

		for (int ThisColumn = 0; ThisColumn < HowManyColumns; ThisColumn++) {

			for (int ThisLayer = 0; ThisLayer < HowManyLayers; ThisLayer++) {

				// Incremental cost should be a non-negative value
				if (Friction[ThisRow][ThisColumn][ThisLayer] <= 0)
					Friction[ThisRow][ThisColumn][ThisLayer] = 1.0;
				// step 2:Initialize the propagation front with all the initial voxels in the InitialSource voxters
				if (InitialSource[ThisRow][ThisColumn][ThisLayer] >= 0 && InitialSource[ThisRow][ThisColumn][ThisLayer] != HowFar){
					ActiveList.push(Node(ThisRow, ThisColumn, ThisLayer, InitialSource[ThisRow][ThisColumn][ThisLayer]), ThisLayer*HowManyColumns*HowManyRows + ThisRow*HowManyColumns + ThisColumn);
					CostDistance[ThisRow][ThisColumn][ThisLayer] = InitialSource[ThisRow][ThisColumn][ThisLayer];
				}
				else
					CostDistance[ThisRow][ThisColumn][ThisLayer] = HowFar;
			}
		}
	}
	// step 5: until there are no more voxels in the front
	while (!ActiveList.empty()){
		// step 3: Obtain the min-cost voxel(Active-) from the front
		pair<Node, long> n1 = ActiveList.pop();
		Node NextActiveVoxel = n1.first; // first:voxel node£¬second: the indexed key in the hash table
		if (NextActiveVoxel.cost() >= HowFar)
			break;
		double ActiveDistance = NextActiveVoxel.cost();
		int ActiveRow = NextActiveVoxel.row();
		int ActiveColumn = NextActiveVoxel.col();
		int ActiveLayer = NextActiveVoxel.layer();
		ActiveCost = Friction[ActiveRow][ActiveColumn][ActiveLayer];
		InwardShiftNorth = ShiftNorth[ActiveRow][ActiveColumn][ActiveLayer];
		InwardShiftEast = ShiftEast[ActiveRow][ActiveColumn][ActiveLayer];
		InwardShiftUp = ShiftUp[ActiveRow][ActiveColumn][ActiveLayer];
		// step 4:  for each adjacent voxel of the min-cost voxel
		for (int NextNeighbor = 0; NextNeighbor < 26; NextNeighbor++){
			int OutwardShiftNorth = -RowShift[NextNeighbor]; //row distance of min-cost voxel to it's neighbor
			int OutwardShiftEast = ColumnShift[NextNeighbor]; //column distance of min-cost voxel to it's neighbor
			int OutwardShiftUp = LayerShift[NextNeighbor]; //layer distance of min-cost voxel to it's neighbor

			// Determine if neighbor voxel is out of range
			int NeighborRow = ActiveRow + RowShift[NextNeighbor];
			if ((NeighborRow < 0) || (NeighborRow >= HowManyRows))
				continue;
			int NeighborColumn = ActiveColumn + OutwardShiftEast;
			if ((NeighborColumn < 0) || (NeighborColumn >= HowManyColumns))
				continue;
			int NeighborLayer = ActiveLayer + OutwardShiftUp;
			if ((NeighborLayer < 0) || (NeighborLayer >= HowManyLayers))
				continue;

			// Determine if neighbor voxel's cost exceeds Howfar
			if (Friction[NeighborRow][NeighborColumn][NeighborLayer] >= HowFar || Friction[ActiveRow][ActiveColumn][ActiveLayer] >= HowFar)
				continue;

			// step a: If the adjacent voxel and the min-cost voxel have different frictions 
			// or if the propagation from the source voxel of the min-cost voxel to the adjacent voxel is blocked by a voxel with different friction
			if (Friction[NeighborRow][NeighborColumn][NeighborLayer] != Friction[ActiveRow][ActiveColumn][ActiveLayer]
				|| (Friction[NeighborRow][NeighborColumn][NeighborLayer] != Friction[NeighborRow + OutwardShiftNorth + InwardShiftNorth][NeighborColumn - (OutwardShiftEast + InwardShiftEast)][NeighborLayer - (OutwardShiftUp + InwardShiftUp)])
				|| IsBlocked(ActiveRow, ActiveColumn, ActiveLayer, InwardShiftNorth, InwardShiftEast, InwardShiftUp, NeighborRow, NeighborColumn, NeighborLayer, Friction)){
				// Set the min-cost voxel as the source voxel of the adjacent voxel
				NeighborShiftNorth = OutwardShiftNorth;
				NeighborShiftEast = OutwardShiftEast;
				NeighborShiftUp = OutwardShiftUp;
				// Distance from min-cost voxel to its neighbour voxel
				NewDistance = ((ActiveCost + Friction[NeighborRow][NeighborColumn][NeighborLayer]) / 2.0)* DistanceToNeighbor[NextNeighbor];
				NeighborDistance = ActiveDistance + NewDistance;
			}
			// step b:
			else{
				// Set the direct source voxel of the min-cost voxel as the direct source voxel of the neighbor voxel
				NeighborShiftNorth = OutwardShiftNorth + InwardShiftNorth;
				NeighborShiftEast = OutwardShiftEast + InwardShiftEast;
				NeighborShiftUp = OutwardShiftUp + InwardShiftUp;
				// Straight-line distance from the source voxel of the min-cost voxel to the neighbor voxel
				double SourceToNeighbor = sqrt(pow((InwardShiftNorth + OutwardShiftNorth), 2) + pow((InwardShiftEast + OutwardShiftEast), 2) + pow((InwardShiftUp + OutwardShiftUp), 2));
				if (Friction[NeighborRow][NeighborColumn][NeighborLayer] == HowFar){

					NeighborDistance = HowFar;
				}
				else
					NeighborDistance = CostDistance[int(ActiveRow + InwardShiftNorth)][int(ActiveColumn - InwardShiftEast)][int(ActiveLayer - InwardShiftUp)] + SourceToNeighbor * Friction[NeighborRow][NeighborColumn][NeighborLayer];
			}

			// If the NewValue >= OldValue
			if (((NeighborDistance * 1.00001) >= CostDistance[NeighborRow][NeighborColumn][NeighborLayer]) || (NeighborDistance >= HowFar) || (NeighborDistance < 0) || (NeighborDistance < ActiveDistance))
				continue;
			// step c: Add the adjacent voxel to the front or update its cost if it is already in the front but has a higher cost
			ActiveList.push(Node(NeighborRow, NeighborColumn, NeighborLayer, NeighborDistance), NeighborLayer*HowManyColumns*HowManyRows + NeighborRow*HowManyColumns + NeighborColumn);
			// step d: Update the cost and direct source of neighbor voxel
			CostDistance[NeighborRow][NeighborColumn][NeighborLayer] = NeighborDistance;
			ShiftNorth[NeighborRow][NeighborColumn][NeighborLayer] = NeighborShiftNorth;
			ShiftEast[NeighborRow][NeighborColumn][NeighborLayer] = NeighborShiftEast;
			ShiftUp[NeighborRow][NeighborColumn][NeighborLayer] = NeighborShiftUp;
		}
	}
	return CostDistance;
}

#endif /*COSTDISTANCE3D_H_*/