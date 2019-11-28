#pragma once
//#pragma warning(disable:1563)

#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTsettingsdlg.h"
#include "DXUTcamera.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "DXUTRes.h"

#include <d3dx11effect.h>

#include "mesh.h"
#include "plane.h"
#include "Terrain.h"
#include "GrassFieldManager.h"


#pragma comment(lib, "Effects11d.lib")
#pragma comment(lib, "DirectXTex.lib")

#define MAX_NUM_MESHES 1

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
ID3DX11Effect			    *g_pSceneEffect;
//Mesh						*g_pMeshes[MAX_NUM_MESHES];
Terrain                     *g_pTerrain;

ID3DX11EffectScalarVariable *g_pEffectHeightScale;

//XMFLOAT3                     g_vCameraEyeStart(9.8f, 9.5f, 7.8f);
//XMFLOAT3                     g_vCameraAtStart(10.8f, 9.0f, 8.8f);

// Effect handles
ID3DX11EffectScalarVariable		   *g_pTerrTile = NULL;

ID3DX11EffectScalarVariable		    *g_pGrassDiffuse = NULL;
GrassFieldState                      g_GrassInitState;
GrassFieldManager                   *g_pGrassField;
Mesh							    *g_pMeshes[MAX_NUM_MESHES];

XMFLOAT3	                         g_MeshVels[MAX_NUM_MESHES];

/* Grass global variables */
float                               g_fGrassLodBias = 0.0f;//0.02f;//0.35f;//0.1f;
float                               g_fGrassSubScatterGamma = 0.95f;
float                               g_fGrassAmbient = 0.15f;//0.23f;//0.05f
float                               g_fGrassDiffuse = 10.0f;

//phys
float                               g_fMass = 0;//1.0;//0.2450f; //0.230f;
float                               g_fHardness = 0;//1.0f;
float                               g_fWindStrength = 0;//1.0f;//0.0616f;
float                               g_fWindStrengthDefault = 0;//1.0f;
float                               g_fWindBias = 0;//0.4370f;
float                               g_fWindScale = 0;//4.96f;
//phys

float                               g_fWindTexSpeed = 0;//2.5f;//3.78f;
float                               g_fWindTexTile = 0;//4.f;//4.f;//5.2f;
float                               g_fCameraSpeed = 30.0f;
float                               g_fTime = 0.0f;
float                               g_fHeightScale = 40.0f;
float                               g_fQuality = 1.0f;
XMFLOAT4							g_vFogColor = XMFLOAT4(0.2f, 0.3f, 0.25f, 1.0f);
XMFLOAT3							g_vTerrRGB = XMFLOAT3(0.16f, 0.28f, 0.09f);
XMFLOAT4							g_vGrassSpecular = XMFLOAT4(0.64f, 0.8f, 0.24f, 1.0f);

float                               g_fTerrTile = 45.0f;


float                               g_fCameraMeshDist = 13.0f;
float                               g_fCameraMeshDistMin;
float                               g_fCameraMeshDistMax;
float                               g_fCameraHeight = 5.0f;
float                               g_fCameraHeightMax;
float                               g_fCameraHeightMin;
XMFLOAT3                            g_vCameraEyeStart(9.8f, 9.5f, 7.8f);
XMFLOAT3                            g_vCameraAtStart(10.8f, 9.0f, 8.8f);

							 
//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc     (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext);
void CALLBACK    OnKeyboard  (UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext);
void CALLBACK    OnGUIEvent  (UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);
void CALLBACK    OnFrameMove (double fTime, float fElapsedTime, void* pUserContext);

bool CALLBACK ModifyDeviceSettings	  (DXUTDeviceSettings* pDeviceSettings, void* pUserContext);

bool CALLBACK IsD3D11DeviceAcceptable (const CD3D11EnumAdapterInfo* AdapterInfo, UINT Output,
                                       const CD3D11EnumDeviceInfo* DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext);

HRESULT CALLBACK OnD3D11CreateDevice (ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
HRESULT CALLBACK OnD3D11ResizedSwapChain (ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
										  const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);

void CALLBACK OnD3D11ReleasingSwapChain (void* pUserContext);
void CALLBACK OnD3D11DestroyDevice      (void* pUserContext);
void CALLBACK OnD3D11FrameRender        (ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                         float fElapsedTime, void* pUserContext );

void InitApp    (void);
void RenderText (void);