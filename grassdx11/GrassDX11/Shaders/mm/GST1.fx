#include "GSFunc.fx"

inline void PtTo2Vertex(float3 vPt, float3 vOffs, float vTCy, float vNy, float2 vTexindTexcrdX, float2 a_vWorldTC, float3 a_vColor, float a_fNoise,  float a_fDissolve,
	inout TriangleStream< PSIn > TriStream)
{
	PSIn Vertex;
	float2 vTexCoord;
	//float tst = length(vPt - g_mInvCamView[3].xyz)
	vTexCoord.y = vTCy-0.015;
    vTexCoord.x = 0.0+0.2125;
    Vertex.vColor = a_vColor;
    Vertex.fNoise = a_fNoise;
    CreateVertex(vPt + vOffs, vTexCoord, vNy, vTexindTexcrdX.x, a_fDissolve, a_vWorldTC, Vertex);
    TriStream.Append(Vertex);    
    //vTexCoord.x = vTexindTexcrdX.y-0.2125;
    vTexCoord.x = 1.0-0.2125;
    CreateVertex(vPt - vOffs, vTexCoord, vNy, vTexindTexcrdX.x, a_fDissolve, a_vWorldTC, Vertex);
    TriStream.Append(Vertex);
}
    
inline void Make4Pts( GSIn In, inout TriangleStream< PSIn > TriStream )
{
	float3 vDir = normalize(In.vPos3 - In.vPos2);
	float fNormalY = max(vDir.y,0.0);
	float3 vCamDir = In.vPos3 - g_mInvCamView[3].xyz;
	float3 vOffs = cross(vCamDir, vDir);
	if (length(vOffs) < 0.0001) vOffs = float3(1.0, 0.0, 0.0);
	else vOffs = normalize(vOffs);
	vOffs = vOffs * SubTypes[In.uTypeIndex].vSizes.x * In.vPackedData.y;;
	float2 vWorldTC = (In.vPos0.xz) / g_fTerrRadius * 0.5 + 0.5;
    PtTo2Vertex(In.vPos3, 1.1*vOffs, 1.0, fNormalY, In.vPackedData.zw, vWorldTC, In.vColor, In.fNoise, In.fDissolve, TriStream);
    /* Update offset */
    vDir = (In.vPos2 - In.vPos1);
    vOffs = cross(vCamDir, vDir);
	if (length(vOffs) < 0.0001) vOffs = float3(1.0, 0.0, 0.0);
	else vOffs = normalize(vOffs);
	vOffs = vOffs * SubTypes[In.uTypeIndex].vSizes.x * In.vPackedData.y;
    PtTo2Vertex(In.vPos2, 1.1*vOffs, 0.66, fNormalY, In.vPackedData.zw, vWorldTC, In.vColor,  In.fNoise, In.fDissolve, TriStream);
    /* Update offset */
    vDir = (In.vPos1 - In.vPos0);
    vOffs = cross(vCamDir, vDir);
	if (length(vOffs) < 0.0001) vOffs = float3(1.0, 0.0, 0.0);
	else vOffs = normalize(vOffs);
	vOffs = vOffs * SubTypes[In.uTypeIndex].vSizes.x * In.vPackedData.y;
	PtTo2Vertex(In.vPos1, 1.1*vOffs, 0.33, fNormalY, In.vPackedData.zw, vWorldTC, In.vColor,  In.fNoise, In.fDissolve, TriStream);
    /* Update offset */
    PtTo2Vertex(In.vPos0, 1.1*vOffs, 0.0, fNormalY, In.vPackedData.zw, vWorldTC, In.vColor,  In.fNoise, In.fDissolve, TriStream);
    
	TriStream.RestartStrip();    
}

inline void Make3Pts( GSIn In, inout TriangleStream< PSIn > TriStream )
{
	float3 vDir = normalize(In.vPos3 - In.vPos2);
	float fNormalY = max(vDir.y,0.0);
	vDir = In.vPos3 - In.vPos1;
	float3 vCamDir = normalize(In.vPos3 - g_mInvCamView[3].xyz);
	float3 vOffs = cross(vCamDir, vDir);
	if (length(vOffs) < 0.0001) vOffs = float3(1.0, 0.0, 0.0);
	else vOffs = normalize(vOffs);
	vOffs = vOffs * SubTypes[In.uTypeIndex].vSizes.x * In.vPackedData.y;;
	float2 vWorldTC = (In.vPos0.xz) / g_fTerrRadius * 0.5 + 0.5;

    PtTo2Vertex(In.vPos3, 1.2*vOffs, 1.0, fNormalY, In.vPackedData.zw, vWorldTC, In.vColor,  In.fNoise, In.fDissolve, TriStream);
    /* Update offset */
    vDir = In.vPos1 - In.vPos0;
	vOffs = cross(vCamDir, vDir);
	if (length(vOffs) < 0.0001) vOffs = float3(1.0, 0.0, 0.0);
	else vOffs = normalize(vOffs);
	vOffs = vOffs * SubTypes[In.uTypeIndex].vSizes.x * In.vPackedData.y;
	PtTo2Vertex(In.vPos1, 1.2*vOffs, 0.33, fNormalY, In.vPackedData.zw, vWorldTC, In.vColor, In.fNoise,  In.fDissolve, TriStream);
    /* Update offset */
    PtTo2Vertex(In.vPos0, 1.2*vOffs, 0.0, fNormalY, In.vPackedData.zw, vWorldTC, In.vColor,  In.fNoise, In.fDissolve, TriStream);
    
	TriStream.RestartStrip();    
}

inline void Make2Pts( GSIn In, inout TriangleStream< PSIn > TriStream )
{
/*	float3 vDir = In.vPos3 - In.vPos2;
	if (length(vDir) < 0.0001) vDir = float3(1.0, 0.0, 0.0);
	vDir = normalize(vDir);
*/
	float3 vDir = normalize(In.vPos3 - In.vPos2);
	float fNormalY = max(vDir.y,0.0);
	vDir = In.vPos3 - In.vPos0;
	float3 vCamDir = In.vPos3 - g_mInvCamView[3].xyz;
	vCamDir = normalize(vCamDir); 
	float3 vOffs = cross(vCamDir, vDir);
	if (length(vOffs) < 0.0001) vOffs = float3(1.0, 0.0, 0.0);
	else vOffs = normalize(vOffs);
	vOffs = vOffs * SubTypes[In.uTypeIndex].vSizes.x * In.vPackedData.y;;
	float2 vWorldTC = (In.vPos0.xz) / g_fTerrRadius * 0.5 + 0.5;

    PtTo2Vertex(In.vPos3, 1.1*vOffs, 1.0, fNormalY, In.vPackedData.zw, vWorldTC, In.vColor,  In.fNoise, In.fDissolve, TriStream);
    /* Update offset */
    PtTo2Vertex(In.vPos0, 1.1*vOffs, 0.0, fNormalY, In.vPackedData.zw, vWorldTC, In.vColor,  In.fNoise, In.fDissolve, TriStream);
    
	TriStream.RestartStrip();    
}

inline void Make7Pts( GSIn In, inout TriangleStream< PSIn > TriStream )
{
	float3 vDir = float3(0.0, 1.0, 0.0);
	float3 vCamDir = g_mInvCamView[2].xyz;
	vCamDir.y = 0.0;
	
	float3 vZ = float3(0.0, 0.0, 1.0);
	float2 vTexCoord = float2(0.0, 0.0);
	
        
    float3 vPos[4];
    int i;
    InitFramePts(In, vPos);    
    float3 vOffsBase[4];
    
    float3 vPts[7];
    vPts[0] = vPos[0];
    vPts[1] = LerpPoint(vPos, 0.165);
    vPts[2] = LerpPoint(vPos, 0.33);
    vPts[3] = LerpPoint(vPos, 0.495);
    vPts[4] = LerpPoint(vPos, 0.66);
    vPts[5] = LerpPoint(vPos, 0.825);
    vPts[6] = vPos[3];
    
    /* Building normals by 4 pts */
    float3 vOffs[7];
    float3 vOff;
    [unroll]for (i = 0; i < 7; i++)
    {
		if (i > 0)
	    {
	        vDir = vPts[i] - vPts[i-1];
	    }
	    vCamDir = vPts[i] - g_mInvCamView[3].xyz;
		vOff = cross(vCamDir, vDir);
		if (length(vOff) < 0.0001) vOff = float3(1.0, 0.0, 0.0);
		else vOff = normalize(vOff);
		vOffs[i] = vOff * SubTypes[In.uTypeIndex].vSizes.x * In.vPackedData.y;;
//        vOffs[i] = normalize(cross(vCamDir, vDir)) * SubTypes[In.uTypeIndex].vSizes.x * In.vPackedData.y;
    }
    //float fNy = normalize(cross(vPos[3] - vPos[2], vOffsBase[3])).y;
	float fNy = max(normalize(vPos[3] - vPos[2]).y, 0.0);//normalize(vPos[3] - vPos[2]).y;

	

    float fInvNum = 0.166;  
    float2 vWorldTC = (In.vPos0.xz) / g_fTerrRadius * 0.5 + 0.5;
    PSIn Vertex;  
    /* Building vertices */
    [unroll]for (i = 0; i < 7; i++)
	{
	    PtTo2Vertex(vPts[i], vOffs[i], i * fInvNum, fNy, In.vPackedData.zw, vWorldTC, In.vColor, In.fNoise,  In.fDissolve, TriStream);	    
	}   
	
	TriStream.RestartStrip();    
}
/*
inline void Make7Pts( GSIn In, inout TriangleStream< PSIn > TriStream )
{
	float3 vDir = float3(0.0, 1.0, 0.0);
	float3 vCamDir = g_mInvCamView[2].xyz;
	vCamDir.y = 0.0;
	
	float3 vZ = float3(0.0, 0.0, 1.0);
	float2 vTexCoord = float2(0.0, 0.0);
	
        
    float3 vPos[4];
    int i;
    InitFramePts(In, vPos);    
    float3 vOffsBase[4];
    
 
    [unroll]for (i = 0; i < 4; i++)
    {
		if (i > 0)
	    {
	        vDir = vPos[i] - vPos[0];
	    }
	    
        vOffsBase[i] = normalize(cross(vCamDir, vDir)) * SubTypes[In.uTypeIndex].vSizes.x * In.vPackedData.y;;
    }
    //float fNy = normalize(cross(vPos[3] - vPos[2], vOffsBase[3])).y;
	float fNy = max(normalize(vPos[3] - vPos[2]).y, 0.0);
    float3 vOffs[7] = {vOffsBase[0],
                       vOffsBase[1],
                       vOffsBase[1],
                       vOffsBase[2],
                       vOffsBase[2],
                       vOffsBase[3],
                       vOffsBase[3],
                       };

	float3 vPts[7];
    vPts[0] = vPos[0];
    vPts[1] = LerpPoint(vPos, 0.165);
    vPts[2] = LerpPoint(vPos, 0.33);
    vPts[3] = LerpPoint(vPos, 0.495);
    vPts[4] = LerpPoint(vPos, 0.66);
    vPts[5] = LerpPoint(vPos, 0.825);
    vPts[6] = vPos[3];

    float fInvNum = 0.166;  
    float2 vWorldTC = (In.vPos0.xz) / g_fTerrRadius * 0.5 + 0.5;
    PSIn Vertex;  
  
    [unroll]for (i = 0; i < 7; i++)
	{
	    PtTo2Vertex(vPts[i], vOffs[i], i * fInvNum, fNy, In.vPackedData.zw, vWorldTC, In.vColor, In.fNoise,  In.fDissolve, TriStream);	    
	}   
	
	TriStream.RestartStrip();    
}
*/