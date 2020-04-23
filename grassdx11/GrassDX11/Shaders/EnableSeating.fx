inline float GetSeatingInfo( float2 vUV )
{              
    return g_txSeatingMap.SampleLevel(g_samLinear, float3(vUV, 0), 0).r;
}
