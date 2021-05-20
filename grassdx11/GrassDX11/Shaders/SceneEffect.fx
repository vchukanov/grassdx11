//--------------------------------------------------------------------------------------
// Constant buffers 
//--------------------------------------------------------------------------------------
cbuffer cEveryFrame
{
    float4x4 g_mWorld;
    float4x4 g_mViewProj;
    float4x4 g_mInvCamView;
    
    float4x4 g_mPrevWorld;
    float4x4 g_mPrevViewProj;
    float4x4 g_mPrevInvCamView;

    float4x4 g_mLightViewProj;
    float4x4 g_mNormalMatrix;
    float    g_fTime;
};
    
// Mesh material settings
float3 g_vKd;
float3 g_vKs;
float3 g_vKa;
float  g_nNs;

cbuffer cUserControlled
{
    float3 g_vTerrRGB;
    float4 g_vFogColor;// = float4(0.0, 0.3, 0.8, 1.0);
    float  g_fTerrTile;
    float  g_fHeightScale;
    float3 g_vTerrSpec = float3(202.0/255.0, 218.0/255.0, 50.0/255.0);
    
    float3 ambientLightColor    = float3(1, 1, 0.8);
    float  ambientLightStrength = 1;

};

cbuffer cRarely
{
    float  g_fTerrRadius;
    float  g_fGrassRadius;
    float2 g_vPixSize = float2(0.0025, 0.0025);
};

cbuffer cImmutable
{
    float3 vLightDir;// = float3( 0.0, 0.9174, -0.2752 ); 
};

//--------------------------------------------------------------------------------------
// Texture and samplers
//--------------------------------------------------------------------------------------
Texture2D g_txMeshDiffuse;
Texture2D g_txScene;
Texture2D g_txVelocityMap;
Texture2D g_txShadowMap;
Texture2D g_txSeatingT1;
Texture2D g_txSeatingT2;
Texture2D g_txSeatingT3;
Texture2D g_txSnowCover;
Texture2D g_txHeightMap;
Texture2D g_txLightMap;
Texture2D g_txSkyBox;
Texture2D g_txTerrGrass;
Texture2D g_txTerrGrassSnowed;
Texture2D g_txSandDiffuse;
Texture2D g_txSandSnowedDiffuse;

Texture2D g_txTerrHeight;
Texture2D g_txTerrNormal;
Texture2D g_txGrassNormal;
Texture2D g_txGrassHeight;

// Mesh textures
Texture2D g_txMeshMapKd;
Texture2D g_txMeshMapKs;

#include "Samplers.fx"
#include "Fog.fx"
#include "States.fx"


//--------------------------------------------------------------------------------------
// Input and output structures 
//--------------------------------------------------------------------------------------

/* Terrain input structures */

struct TerrVSIn
{
    float3 vPos      : POSITION;
    float2 vTexCoord : TEXCOORD0;

    float3 tangent  : TANGENT;
    float3 normal   : NORMAL;
    float3 bitangent : BITANGENT;
};

struct TerrPSIn
{
    float4 vPos      : SV_Position;
    
    float4 vShadowPos : TEXCOORD0;
    float4 vTexCoord  : TEXCOORD1;

    float3 tanLightDir  : POSITION0;
    float3 tanViewPos   : POSITION1;
    float3 tanFragPos   : POSITION2;
    float3 tanFragPos1  : POSITION3;

    float3 worldPos  : POSITION4;

    //float3 vNormal   : NORMAL;
};

/* Sky box input */

struct VSSceneIn
{
    float3 pos  : POSITION;
    float3 norm : NORMAL;
    float2 tex  : TEXTURE0;
}; 

struct PSSceneIn
{
    float4 pos : SV_Position;
    float3 tex : TEXTURE0;
};

struct PSCarIn
{
    float4 pos : SV_Position;
    float3 world_pos : TEXCOORD1;
    float3 norm : NORMAL;
    float2 tex : TEXTURE0;
};


struct MeshVSIn
{
   float3 vPos      : POSITION;
   float2 vTexCoord : TEXCOORD;
   float3 vNormal   : NORMAL;
};

struct MeshPSIn
{
   float4 vPos      : SV_Position;
   float3 vWorldPos : POSITION;
   float4 vTexCoord : TEXCOORD;
   float3 vNormal   : NORMAL;
};


struct BlurPSIn
{
   float4 vPos : SV_Position;

   float4 vPosition : POSITION0;
   float4 vPrevPosition : POSITION1;
};

//--------------------------------------------------------------------------------------
// Shaders
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Sky vertex shader
//--------------------------------------------------------------------------------------
PSSceneIn VSSkymain(VSSceneIn Input)
{
    PSSceneIn Output;
    float3 vWorldPos = Input.pos * 0.25;
    Output.pos = mul(float4(vWorldPos, 1.0), g_mViewProj);
    Output.tex.xy = 1.0 - Input.tex;
    float fY = vWorldPos.y;
    Output.tex.z = FogValue(350 - fY);
    return Output;
}

float4 PSSkymain(PSSceneIn Input): SV_Target0
{
	return lerp(g_txSkyBox.Sample(g_samLinear, Input.tex.xy), g_vFogColor, Input.tex.z);
}

PSCarIn VSCarmain(VSSceneIn Input)
{
    PSCarIn Output;
    
    float4 vWorldPos = mul(float4(Input.pos, 1.0), g_mWorld);
    //vWorldPos.y += 0.2;
    Output.world_pos = vWorldPos.xyz;
    Output.pos = mul(vWorldPos, g_mViewProj);
    Output.tex = Input.tex;
    Output.tex.y = 1.0 - Output.tex.y;
    Output.norm = normalize(mul((float3x3)g_mNormalMatrix, Input.norm));
    
    return Output;
}

float4 PSCarmain0(PSCarIn Input): SV_Target0
{
    float4 ka, kd, ks;
    float3 eye_dir = normalize(g_mInvCamView[3].xyz - Input.world_pos);

    ka = float4(g_vKa, 1.0f);
	
    if (g_vKd.x != 0 || g_vKd.y != 0 || g_vKd.z != 0)
        kd = float4(g_vKd, 1.0f);
    else
        kd = g_txMeshMapKd.Sample(g_samLinear, Input.tex);
        
    if (g_vKs.x != 0 || g_vKs.y != 0 || g_vKs.z != 0)
        ks = float4(g_vKs, 1.0f);
    else
        ks = g_txMeshMapKs.Sample(g_samLinear, Input.tex);
    
    float3 half = normalize(eye_dir - vLightDir);
    
    float3 refl = normalize(reflect(eye_dir, Input.norm));
    float2 sphere_tex_coords;
    
    sphere_tex_coords.x = refl.x / 2 + 0.5;
    sphere_tex_coords.y = refl.y / 2 + 0.5;

    float3 Ir = g_txSkyBox.Sample(g_samLinear, sphere_tex_coords).xyz;
    
    float Ia = 0.3, Id = 0.2, Is = 1.1;

    return float4(Ia * ka.xyz * kd.xyz, kd.w) +
        float4(Id * kd.xyz * max(dot(-vLightDir, Input.norm), 0), kd.w) +
        float4(ks.xyz * (Is * pow(max(dot(half, Input.norm), 0.0f), g_nNs) + Ir), 0.0f);
}


/* Mesh shaders */
MeshPSIn MeshVSMain( MeshVSIn Input )
{
    MeshPSIn Output;
    Output.vPos         = mul(mul(float4(Input.vPos, 1.0), g_mWorld), g_mViewProj);
    Output.vTexCoord.xy = Input.vTexCoord;
    Output.vNormal      = normalize(mul(float4(Input.vNormal, 0.0), g_mWorld));
    Output.vWorldPos    = mul(float4(Input.vPos, 1.0), g_mWorld);
    return Output;    
}

float4 MeshPSMain (MeshPSIn Input): SV_Target
{
    float3 sampleColor = g_txMeshDiffuse.Sample(g_samLinear, Input.vTexCoord.xy);
    float3 ambientLight = float3(0.1, 0.1, 0.1);//ambientLightColor * ambientLightStrength;
    float3 appliedLight = ambientLight;

    float3 diffuseLightIntensity = max(dot(-vLightDir, Input.vNormal), 0);
    float3 diffuseLight = diffuseLightIntensity /** fLightStrength * fLightColor*/;

    appliedLight += diffuseLight;
    float3 finalColor = sampleColor * appliedLight;

    //return float4(finalColor, 1);

    float3 eye_dir = normalize(g_mInvCamView[3].xyz - Input.vWorldPos);

   /* float4 ka, kd, ks;
    
    ka = float4(g_vKa, 1.0f);
	
    if (g_vKd.x != 0 || g_vKd.y != 0 || g_vKd.z != 0)
        kd = float4(g_vKd, 1.0f);
    else
        kd = g_txMeshMapKd.Sample(g_samLinear, Input.vTexCoord);
        
    if (g_vKs.x != 0 || g_vKs.y != 0 || g_vKs.z != 0)
        ks = float4(g_vKs, 1.0f);
    else
        ks = g_txMeshMapKs.Sample(g_samLinear, Input.vTexCoord);
    */

    float3 half = normalize(eye_dir - vLightDir);
    float3 refl = normalize(reflect(eye_dir, Input.vNormal));
    float2 sphere_tex_coords;
    
    sphere_tex_coords.x = refl.x / 2 + 0.5;
    sphere_tex_coords.y = refl.y / 2 + 0.5;

    float3 Ir = g_txSkyBox.Sample(g_samLinear, sphere_tex_coords).xyz;
    Ir *= clamp(dot(Input.vNormal, float3(0, 1, 0)), 0, 1);

    return float4(finalColor + Ir, 1);
    //float Ia = 0.3, Id = 0.2, Is = 1.1;

    /*return float4(Ia * ka.xyz * kd.xyz, kd.w) +
        float4(Id * kd.xyz * max(dot(-vLightDir, Input.vNormal), 0), kd.w) +
        float4(ks.xyz * (Is * pow(max(dot(half, Input.vNormal), 0.0f), g_nNs) + Ir), 0.0f);*/
}



float4 MeshPSMainDbg (MeshPSIn Input): SV_Target
{
    return float4(1, 0, 0, 0.5);
}

float4 MeshPSMainBlured( TerrPSIn Input ): SV_Target
{
    float2 texelSize = (1.0 / 1600.0, 1.0 / 900.0);
    float2 screenTexCoords = Input.vPos * texelSize;
    screenTexCoords.x /= (1600.0 / 900.0);  // bug ??

    float2 velocity = g_txVelocityMap.Sample(g_samLinear, screenTexCoords).xy;
    velocity = pow(velocity * 2 - 1, 3.0);
    velocity *= 0.1; //uVelocityScale;

    float speed = length(velocity / texelSize);
    int nSamples = clamp(int(speed), 1, 4);
    
    float4 oResult = g_txScene.Sample(g_samLinear, screenTexCoords);
    for (int i = 1; i < nSamples; ++i) {
       float2 offset = velocity * (float(i) / float(nSamples - 1) - 0.5);
       oResult += g_txScene.Sample(g_samLinear, screenTexCoords + offset);
    }
    oResult /= float(nSamples);
    
    return oResult;
}

/* Blur shaders */
BlurPSIn MeshVSBlur( MeshVSIn Input )
{
    BlurPSIn Output;
    Output.vPosition = mul(mul(float4(Input.vPos, 1.0), g_mWorld), g_mViewProj);
    Output.vPrevPosition = mul(mul(float4(Input.vPos, 1.0), g_mPrevWorld), g_mPrevViewProj);

    Output.vPos = Output.vPosition;
    return Output;    
}


float4 MeshPSBlur( BlurPSIn Input ): SV_Target
{
    float2 a = (Input.vPosition.xy / Input.vPosition.w) * 0.5 + 0.5;
    float2 b = (Input.vPrevPosition.xy / Input.vPrevPosition.w) * 0.5 + 0.5;
    float2 oVelocity = pow(abs(a - b), 1 / 3.0) * sign(a - b) * 0.5 + 0.5;
    return float4(oVelocity.x, oVelocity.y, 0, 1);
}

/* Terrain shaders */

float3 CalcNormal (float2 a_vTexCoord)
{
    const float2 dU = float2(g_vPixSize.x, 0);
    const float2 dV = float2(0, g_vPixSize.y);
  
    float heightC = g_txHeightMap.SampleLevel(g_samLinear, a_vTexCoord, 0).r * g_fHeightScale;
    float heightL = g_txHeightMap.SampleLevel(g_samLinear, a_vTexCoord - dU, 0).r * g_fHeightScale;
    float heightR = g_txHeightMap.SampleLevel(g_samLinear, a_vTexCoord + dU, 0).r * g_fHeightScale;
    float heightT = g_txHeightMap.SampleLevel(g_samLinear, a_vTexCoord - dV, 0).r * g_fHeightScale;
    float heightB = g_txHeightMap.SampleLevel(g_samLinear, a_vTexCoord + dV, 0).r * g_fHeightScale;
	
    return normalize(float3(heightR - heightL, 1.0, heightB - heightT));
}


TerrPSIn TerrainVSMain( TerrVSIn Input )
{
    TerrPSIn Output;
    float fY			 = g_txHeightMap.SampleLevel(g_samLinear, Input.vTexCoord, 0).a * g_fHeightScale;
    float4 vWorldPos	 = float4(Input.vPos + float3(0.0, fY, 0.0), 1.0);
    Output.vPos          = mul(vWorldPos, g_mViewProj);

    
    Output.worldPos = vWorldPos;

    Output.vTexCoord.xy  = Input.vTexCoord;
    Output.vTexCoord.z   = FogValue(length(vWorldPos - g_mInvCamView[3].xyz));
    Output.vTexCoord.w   = length(vWorldPos - g_mInvCamView[3].xyz);
    Output.vShadowPos   = mul( vWorldPos, g_mLightViewProj);    

    //float3 T = normalize(float3(model * vec4(aTangent,   0.0)));
    //float3 B = normalize(float3(model * vec4(aBitangent, 0.0)));
    //float3 N = normalize(float3(model * vec4(aNormal,    0.0)));
    float3x3 TBN = float3x3(Input.tangent, Input.bitangent, Input.normal);
//dbg
    Output.tanLightDir = mul(vLightDir, TBN);
    Output.tanViewPos  = mul(g_mInvCamView[3].xyz, TBN);
    Output.tanFragPos  = mul(Input.vPos + float3(0.0, fY, 0.0), TBN);  // minus becouse bug in tbn
    Output.tanFragPos1  = mul(Input.vPos - float3(0.0, fY, 0.0), TBN);  // minus becouse bug in tbn
 
 /*
    Output.tanFragPos;
    Output.tanLightPos;
    Output.tanViewPos;
*/
    return Output;
}

float GetAlphaCoef(float2 vTexCoord)
{
    float vAlpha =  g_txSeatingT1.Sample(g_samLinear, vTexCoord).r;
    return clamp(length(vAlpha), 0.0, 1.0);                            
}



float2 reliefPM(float2 inTexCoords, float3 inViewDir, float3 inViewDir1, Texture2D heigtTex) {
    float lastDepthValue;

	float _minLayers = 2.;
	float _maxLayers = 32.;
	float _numLayers = lerp(_maxLayers, _minLayers, abs(dot(float3(0., 0., 1.), inViewDir)));

	float deltaDepth = 1./_numLayers;
	float2 deltaTexcoord = g_vTerrRGB.x * inViewDir.xy / (inViewDir1.z * _numLayers);

	float2 currentTexCoords = inTexCoords;
	float currentLayerDepth = 0.;
	float currentDepthValue = 1 - heigtTex.Sample(g_samPoint, currentTexCoords);

    [unroll(64)]
    while (currentDepthValue > currentLayerDepth) {
		currentLayerDepth += deltaDepth;
		currentTexCoords -= deltaTexcoord;
		currentDepthValue = 1 - heigtTex.Sample(g_samPoint, currentTexCoords);
	}

	deltaTexcoord *= 0.5;
	deltaDepth *= 0.5;
	currentTexCoords += deltaTexcoord;
	currentLayerDepth -= deltaDepth;

	const int _reliefSteps = 5;
	int currentStep = _reliefSteps;

    [unroll(64)]
    while (currentStep > 0) {
		currentDepthValue = 1 - heigtTex.Sample(g_samPoint, currentTexCoords);
		deltaTexcoord *= 0.5;
		deltaDepth *= 0.5;

		if (currentDepthValue > currentLayerDepth) {
			currentTexCoords -= deltaTexcoord;
			currentLayerDepth += deltaDepth;
		}
		else {
			currentTexCoords += deltaTexcoord;
			currentLayerDepth -= deltaDepth;
		}
		currentStep--;
	}

	lastDepthValue = currentDepthValue;
	return currentTexCoords;
}


float2 ParallaxMapping(float2 texCoords, float3 viewDir,  float3 viewDir1, Texture2D heigtTex, out float lastDepthValue)
{ 
    //float height =  g_txTerrHeight.Sample(g_samLinear, texCoords).r;    
    //float2 p = -viewDir.xy / viewDir.z * (height * g_vTerrRGB.x/*height_scale*/);
    //return texCoords - p;    

    float minLayers = 0;
    float maxLayers = 32;
    float numLayers = lerp(maxLayers, minLayers, abs(dot(float3(0.0, 0.0, 1.0), viewDir)));
    float layerDepth = 1.0 / numLayers;

    float currentLayerDepth = 0;

    float2 P = viewDir.xy / viewDir1.z * 0.075;

    float2 deltaTexCoords = P / numLayers;

    float2 currentTexCoords = texCoords;

    float currentDepthMapValue =  1 - heigtTex.Sample(g_samLinear, currentTexCoords).r;

    [unroll(32)]
    while (currentLayerDepth < currentDepthMapValue)
    {
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = 1 - heigtTex.Sample(g_samLinear, currentTexCoords).r;
        currentLayerDepth += layerDepth;
    }

    float2 prevTexCoords = currentTexCoords + deltaTexCoords;
    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth =  1 - heigtTex.Sample(g_samLinear, prevTexCoords).r - currentLayerDepth + layerDepth;

    float weight = afterDepth / (afterDepth - beforeDepth);

	lastDepthValue = currentLayerDepth;

    float2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}


float ShadowCalc(float2 inTexCoords, float3 inLightDir, float inLastDepth)
{
    if (inLastDepth > 0.90) {
        inLastDepth = 1 - g_txTerrHeight.Sample(g_samLinear, inTexCoords).r;
    }
    
    float shadowMultiplier = 0.;
// расчет будем делать только для поверхностей, 
// освещенных используемым источником
	float alignFactor = dot(float3(0., 0., 1.), inLightDir);
	if (alignFactor > 0.) {
// знакомая инициализация параметров: слои глубины, шаг 
// слоя глубины, шаг смещения текстурных координат
		float _minLayers = 16.;
		float _maxLayers = 32.;
		float _numLayers = lerp(_maxLayers, _minLayers, abs(alignFactor));
		float _dDepth = inLastDepth/_numLayers;
		float2 _dtex = 0.3 * inLightDir.xy / (inLightDir.z * _numLayers);

// счетчик точек, оказавшихся под поверхностью
		int numSamplesUnderSurface = 0;

// поднимаемся на глубину слоя и смещаем 
// текстурные координаты вдоль вектора L
		float currentLayerDepth = inLastDepth - _dDepth;
		float2 currentTexCoords = inTexCoords + _dtex;

		float currentDepthValue = 1 - g_txTerrHeight.Sample(g_samLinear, currentTexCoords).r;
// номер текущего шага
		float stepIndex = 1.;
// повторяем, пока не выйдем за слой нулевой глубины…
        [unroll(16)]
		while (currentLayerDepth > 0.) {
// если нашли точку под поверхностью, то увеличим счетчик и 
// рассчитаем очередной частичный и полный коэффициенты
			if (currentDepthValue < currentLayerDepth) {
				numSamplesUnderSurface++;
				float currentShadowMultiplier = (currentLayerDepth - currentDepthValue) * (1. - stepIndex/_numLayers);
                shadowMultiplier = max(shadowMultiplier, currentShadowMultiplier);
			}
			stepIndex++;
			currentLayerDepth -= _dDepth;
			currentTexCoords += _dtex;
			currentDepthValue = 1 - g_txTerrHeight.Sample(g_samLinear, currentTexCoords).r;
		}
// если точек под поверхностью не было, то точка 
// считается освещенной и коэффициент оставим 1
		if (numSamplesUnderSurface < 1)
			shadowMultiplier = 1.;
		else
			shadowMultiplier = 1. - shadowMultiplier;
	} else {
        return float4(1, 0, 0, 1);
    }

	return shadowMultiplier;
 /*   if ( lightDir.z >= 0.0 )
        return 0.0;

    float shadow = 0.0;
    float minLayers = 0;
    float maxLayers = 32;
    float numLayers = lerp(maxLayers, minLayers, abs(dot(float3(0.0, 0.0, 1.0), lightDir)));

    float2 currentTexCoords = texCoord;
    float currentDepthMapValue = 1 - g_txTerrHeight.Sample(g_samLinear, currentTexCoords).r;
    float currentLayerDepth = currentDepthMapValue;

    float layerDepth = 1.0 / numLayers;
    float2 P = lightDir.xy / lightDir.z * g_vTerrRGB.x;
    float2 deltaTexCoords = P / numLayers;

    [unroll(32)]
    while (currentLayerDepth <= currentDepthMapValue && currentLayerDepth > 0.0)
    {
        currentTexCoords += deltaTexCoords;
        currentDepthMapValue = 1 - g_txTerrHeight.Sample(g_samLinear, currentTexCoords).r;
        currentLayerDepth -= layerDepth;
    }

    float r = currentLayerDepth > currentDepthMapValue ? 0.0 : 1.0;
    return r;*/
}


float3 Blend(float4 texture1, float a1, float4 texture2, float a2)
{
    /*if (a1 > g_vTerrRGB.y) {
        if ((texture1.r + texture1.g + texture1.b) / 3 > g_vTerrRGB.z) {
            return texture1;
        }
    }*/ 
    return (a1 * texture1) + (a2 * texture2);
}


float4 TerrainPSMain( TerrPSIn Input ): SV_Target
{
    float3 viewDir = normalize(Input.tanViewPos - Input.tanFragPos);
    float3 viewDir1 = normalize(Input.tanViewPos - Input.tanFragPos1);
    
    float alphaValue = GetAlphaCoef(Input.vTexCoord.xy);

    float lastDepthValue = 0;
    float2 texCoords = Input.vTexCoord * 64;
  
    float3 normal  = g_txTerrNormal.Sample(g_samLinear, Input.vTexCoord * 64);
  
    float selfShadow = 1;
    if (alphaValue < 0.5 && length(g_mInvCamView[3].xyz - Input.worldPos) < 75) {
        texCoords = ParallaxMapping(Input.vTexCoord * 64, viewDir, viewDir1, g_txTerrHeight, lastDepthValue);
        selfShadow = ShadowCalc(texCoords, -Input.tanLightDir, lastDepthValue);
    } else {
        if (alphaValue < 0.5)
            selfShadow = dot(-vLightDir * 2 * 0.88, normal);
    }
  
    float shadowCoef = ShadowCoef(Input.vShadowPos);
    
    float2 fDot = g_txLightMap.Sample(g_samLinear, Input.vTexCoord.xy).rg;
  
    float3 vGrassColor = g_txTerrGrass.Sample(g_samLinear, Input.vTexCoord.xy * 64).xyz;
    vGrassColor.xyz *= float3(0.22, 0.25, 0);
    float3 vGrassSnowedColor = g_txTerrGrassSnowed.Sample(g_samLinear, Input.vTexCoord.xy * 64).xyz;
    alphaValue = clamp(length(g_txSnowCover.Sample(g_samLinear, Input.vTexCoord.xy).r), 0.0, 1.0);
    float3 vGrassBlendColor = (alphaValue * vGrassSnowedColor) + ((1.0 - alphaValue) * vGrassColor);

    float3 vSandColor = g_txSandDiffuse.Sample(g_samLinear, Input.vTexCoord.xy * 64).xyz;
    vSandColor *= float3(0.37, 0.37, 0.28);
    float3 vSandSnowedColor = g_txSandSnowedDiffuse.Sample(g_samLinear, Input.vTexCoord.xy * 64).xyz;
    float3 vSandBlendColor = (alphaValue * vSandSnowedColor) + ((1.0 - alphaValue) * vSandColor);

    alphaValue = GetAlphaCoef(Input.vTexCoord.xy);    
    float3 blendColor = (alphaValue * vGrassBlendColor) + ((1.0 - alphaValue) * vSandBlendColor);

    float3 vL = blendColor * max(0.8, (2.0 + 5.0 * fDot.y) * 0.5);
	float fLimDist = clamp((Input.vTexCoord.w - 140.0) / 20.0, 0.0, 1.0);

    float4 color = lerp(float4(fDot.x * fLimDist * g_vTerrSpec + (1.0 - fDot.x * fLimDist) * vL, 1.0), g_vFogColor, Input.vTexCoord.z);
    
    color.xyz = color.xyz * shadowCoef * selfShadow;
    return color;
}

/* Light Map Shaders */

TerrPSIn LightMapVSMain( TerrVSIn Input )
{
    TerrPSIn Output;
    Output.vPos          = float4(Input.vPos, 1.0);
    Output.vTexCoord.xy  = Input.vTexCoord;
    //Output.vTexCoord.x = 1.0 - Output.vTexCoord.x;
    return Output;
}

float4 LightMapPSMain( TerrPSIn Input ): SV_Target
{
    float2 vTransformedTC = (Input.vTexCoord * 2.0 - 1.0) * g_fTerrRadius;
    float4 vHeightData    = g_txHeightMap.Sample(g_samLinear, Input.vTexCoord.xy);
    vHeightData.xyz = vHeightData.xyz * 2.0 - 1.0;
    float3 vTerrPos = float3(vTransformedTC.x, vHeightData.a * g_fHeightScale, vTransformedTC.y);
    float fDot = 1.0 - dot(normalize(g_mInvCamView[3].xyz - vTerrPos), vHeightData.xyz);
    float fLightDot = -dot(vLightDir, vHeightData.xyz);
    fDot *= fDot;  
    fDot *= fDot;  
    fDot *= fDot;   
    fDot *= fDot;  
    return float4(fDot, fLightDot, 0.0, 0.0);
}

technique10 Render
{    
    pass RenderTerrainPass
    {
        SetVertexShader( CompileShader( vs_4_0, TerrainVSMain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, TerrainPSMain() ) ); 
        SetBlendState( NonAlphaState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepthTestWrite, 0 );
        SetRasterizerState( EnableMSAACulling );
    }  

    pass RenderMeshPass
    {
        SetVertexShader( CompileShader( vs_4_0, MeshVSMain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, MeshPSMain() ) ); 
        SetBlendState( NonAlphaState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );   
        SetRasterizerState( EnableMSAA );
    }

    pass RenderMeshPassDbg
    {
        SetVertexShader( CompileShader( vs_4_0, MeshVSMain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, MeshPSMainDbg() ) ); 
        SetBlendState( NonAlphaState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );   
        SetRasterizerState( EnableMSAA );
    }

    /*pass RenderMeshPassBlured
    {
        SetVertexShader( CompileShader( vs_5_0, MeshVSMain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, MeshPSMainBlured() ) ); 
        SetBlendState( NonAlphaState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );   
        SetRasterizerState( EnableMSAA );
    }*/

    pass RenderLightMapPass
    {
        SetVertexShader( CompileShader( vs_4_0, LightMapVSMain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, LightMapPSMain() ) ); 
        SetBlendState( NonAlphaState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );   
        SetRasterizerState( EnableMSAA );
    }

    
    pass RenderVelocityPass
    {
        SetVertexShader( CompileShader( vs_4_0, MeshVSBlur() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, MeshPSBlur() ) ); 
        SetBlendState( NonAlphaState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );   
        SetRasterizerState( EnableMSAA );
    }
}

technique10 RenderSkyBox
{
	pass RenderPass
    {
        SetVertexShader( CompileShader( vs_4_0, VSSkymain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSSkymain() ) ); 
        SetBlendState( NonAlphaState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );   
    }
}

technique10 RenderCar
{
	pass RenderPass
    {
        SetVertexShader( CompileShader( vs_4_0, VSCarmain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSCarmain0() ) ); 
        SetBlendState( NonAlphaState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );   
    }	
}
