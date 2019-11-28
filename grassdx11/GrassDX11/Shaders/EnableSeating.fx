inline float GetSeatingInfo( float2 vUV )
{              
    return g_txSeatingMap.SampleLevel(g_samLinear, vUV * 0.33, 0).r;
}
