__kernel void generateRegion(const int width, const ulong worldseed, const int xRegionStart, const int zRegionStart, __global int* regionData) {
    /* because of the stupid way the original MC java code does this calculation, every type declaration and
     * bit manipulation has to be precise. it's dependent on signed 32-bit overflow behavior and using those
     * signed values for the math, meaning we can't really get away with anything less than typecasting from
     * 32-bit to 64-bit integers. using 64-bit ints the whole way through and manually truncating them, as an
     * example, fails because bit 32 no longer encodes necessary sign information. if we let it, the compiler 
     * decides to "optimize" away this behavior, which is why we use the "no compiler optimizations" tag. */
    
    // identify where we are
    int xIndex = get_global_id(0);
    int zIndex = get_global_id(1);

    int xChunk = xRegionStart + (xIndex << 5);
    int zChunk = zRegionStart + zIndex;

    // not actually a "number" with any real meaning, but 32 bits of slime chunk data
    uint slice = 0;

    // do the slime chunk calculation for 32 chunks, then encode the data into the slice variable
    int seed1 = zChunk * zChunk;
    int seed2 = zChunk * 0x5F24F;
    ulong zseed = worldseed + (convert_ulong(seed1) * 0x4307A7) + convert_ulong(seed2);

    int seed3, seed4;

    for (int i = 0; i < 32; i++) {
        seed3 = xChunk * xChunk * 0x4C1906;
        seed4 = xChunk * 0x5AC0DB;

        // this is what we call "performance critical" lmao
        slice += (!((((((zseed + convert_ulong(seed3) + convert_ulong(seed4)) 
                    ^ 0x5E434E432) * 0x5DEECE66D + 0xB) & 0xFFFFFFFFFFFF) >> 17) % 10) << i);

        xChunk++;
    }

    regionData[zIndex * width + xIndex] = slice;
}