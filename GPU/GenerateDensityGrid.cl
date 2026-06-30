#define BLOCKSIZE 16

__kernel void generateDensityGrid(__global const char* xdensity, const int regionSize, __global char* density) {
    int globalZ = get_global_id(0);
    int globalX = get_global_id(1);

    int localZ = get_local_id(0);
    int localX = get_local_id(1);

    int zGroup = get_group_id(0);
    int xGroup = get_group_id(1);

    __local char inputTile[BLOCKSIZE][2 * BLOCKSIZE];
    __local char outputTile[BLOCKSIZE][BLOCKSIZE + 1];

    inputTile[localX][localZ] = xdensity[globalX * (regionSize + 16) + globalZ];
    inputTile[localX][localZ + BLOCKSIZE] = xdensity[globalX * (regionSize + 16) + globalZ + BLOCKSIZE];

    barrier(1);

    outputTile[localX][localZ] = 0;
    for (int i = 0; i < 17; i++) {
        outputTile[localX][localZ] += inputTile[localX][localZ + i];
    }

    barrier(1);

    int globalXStart = xGroup * BLOCKSIZE;
    int globalZStart = zGroup * BLOCKSIZE;

    density[(globalZStart + localX) * regionSize + globalXStart + localZ] = outputTile[localZ][localX];
}