//Author AKM Mubashwir Alam
#ifndef __KMEANS_
#define __KMEANS

void kmeans(int pBlockSize, int pEnablePadding, int pSortType, string pCentroidFile, string pCoordinateFile, string pOutputPath);
void kmeans_encoder(int pTotalBlock, int pBlockSize, string pCentroidFile, string pCoordinateFile);
#endif
