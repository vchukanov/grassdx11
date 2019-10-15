inline uint GetTypeIndex( float2 vUV )
{
    return uint(g_txIndexMap.SampleLevel(g_samPoint, vUV, 0).r * 9.0);
}