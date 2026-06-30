#define BLOCKSIZE 16

__kernel void generateXDensityGrid(__global const uint* regionData, const int regionXSize, const int regionZSize, __global char* xdensity) {
    int xIndex = get_global_id(0);
    int zIndex = get_global_id(1);

    int localX = get_local_id(0);
    int localZ = get_local_id(1);

    int xGroup = get_group_id(0);
    int zGroup = get_group_id(1);

    __local char tile[BLOCKSIZE][BLOCKSIZE + 1];

    ulong localRegionData;
    if ((xIndex % 32) > 15) {
        localRegionData = (convert_ulong(regionData[zIndex * regionXSize + (xIndex >> 5) + 1]) << 32) + convert_ulong(regionData[zIndex * regionXSize + (xIndex >> 5)]);
    }
    else {
        localRegionData = convert_ulong(regionData[zIndex * regionXSize + (xIndex >> 5)]);
    }

    uint chunkCount = convert_uint(localRegionData >> (xIndex % 32)) & 0x1FFFF;

    //xdensity[xIndex * regionZSize + zIndex] = regionData[zIndex * regionXSize + (xIndex >> 5)];

    tile[localX][localZ] = 0;
    while (chunkCount) {
        tile[localX][localZ]++;
        chunkCount &= chunkCount - 1;
    }

    barrier(1);

    int globalXStart = xGroup * BLOCKSIZE;
    int globalZStart = zGroup * BLOCKSIZE;

    xdensity[(globalXStart + localZ) * regionZSize + globalZStart + localX] = tile[localZ][localX];
}