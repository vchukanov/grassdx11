#pragma once
//#pragma warning(disable:1563)//taking address of a temporary

#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTsettingsdlg.h"
#include "DXUTcamera.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "DXUTRes.h"
#include "resource.h"
#include "GrassFieldManager.h"
#include "Mesh/Mesh.h"
#include "Mesh/Car.h"
#include "Camera.h"
#include "StateManager.h"

#define MAX_NUM_MESHES 1

/* Camera types */
#define CAMERA_NORMAL 0
#define CAMERA_TERRAIN 1
#define CAMERA_MESH 2

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
CFirstPersonCamera                 *g_Camera;
CDXUTDialogResourceManager          g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg                     g_D3DSettingsDlg;       // Device settings dialog
CDXUTDialog                         g_HUD;                  // manages the 3D UI
CDXUTDialog                         g_SampleUI;             // dialog for sample specific controls

// Direct3D 10 resources
ID3DX10Font                        *g_pFont10 = NULL;
ID3DX10Sprite                      *g_pSprite10 = NULL;
CDXUTTextHelper                    *g_pTxtHelper = NULL;
//ID3D10RasterizerState              *g_pRasterState = NULL;

ID3D10Texture2D*                    g_pRenderTarget = NULL;
ID3D10RenderTargetView*             g_pRTRV = NULL;
ID3D10Texture2D*                    g_pDSTarget = NULL;
ID3D10DepthStencilView*             g_pDSRV = NULL;

/* sky */
ID3D10InputLayout*                  g_pSkyVertexLayout = NULL;
CDXUTSDKMesh                        g_MeshSkybox;
ID3D10EffectShaderResourceVariable *g_pSkyBoxESRV = NULL;
ID3D10EffectMatrixVariable		   *g_pSkyViewProjEMV;	
ID3D10EffectTechnique*              g_pRenderSkybox = NULL;

UINT                                g_MSAASampleCount = 4;
UINT                                g_BackBufferWidth;
UINT                                g_BackBufferHeight;

// Effect handles
ID3D10EffectScalarVariable         *g_pTerrTile  = NULL;

//ID3D10EffectScalarVariable         *g_pGrassDiffuse = NULL;
GrassFieldState                     g_GrassInitState;
GrassFieldManager                  *g_pGrassField;
Mesh                               *g_pMeshes[MAX_NUM_MESHES];

D3DXVECTOR3                         g_MeshVels[MAX_NUM_MESHES];
/* Grass global variables */
float                               g_fGrassLodBias = 0.0f;//0.02f;//0.35f;//0.1f;
float                               g_fGrassSubScatterGamma = 0.95f;
float                               g_fGrassAmbient = 0.15f;//0.23f;//0.05f
float                               g_fGrassDiffuse = 10.0f;

//phys
float                               g_fMass = 1.0;//0.2450f; //0.230f;
float                               g_fHardness = 1.0f;
float                               g_fWindStrength = 1.0f;//0.0616f;
float                               g_fWindStrengthDefault = 1.0f;
float                               g_fWindBias = 0.4370f;
float                               g_fWindScale = 4.96f;
//phys

float                               g_fWindTexSpeed = 2.5f;//3.78f;
float                               g_fWindTexTile = 4.f;//4.f;//5.2f;
float                               g_fCameraSpeed = 4.0f;
float                               g_fTime = 0.0f;
float                               g_fHeightScale = 40.0f;
float                               g_fQuality     = 1.0f;
D3DXVECTOR4                         g_vFogColor    = D3DXVECTOR4(0.2f, 0.3f, 0.25f, 1.0f);
D3DXVECTOR3                         g_vTerrRGB     = D3DXVECTOR3(0.16f, 0.28f, 0.09f);
D3DXVECTOR4                         g_vGrassSpecular   = D3DXVECTOR4(0.64f, 0.8f, 0.24f, 1.0f);

float                               g_fTerrTile    = 45.0f;

float                               g_fCameraMeshDist = 13.0f;
float                               g_fCameraMeshDistMin;
float                               g_fCameraMeshDistMax;
float                               g_fCameraHeight = 5.0f;
float                               g_fCameraHeightMax;
float                               g_fCameraHeightMin;
D3DXVECTOR3                         g_vCameraEyeStart(9.8f, 9.5f, 7.8f);
D3DXVECTOR3                         g_vCameraAtStart(10.8f, 9.0f, 8.8f);

float                               g_fCarLength = 5.0f;//3.0;
float                               g_fCarHeight = 1.8f;
float                               g_fCarFrontWidth = 2.8f;
/*float                               g_fCarLength = 3;
float                               g_fCarHeight = 1;
float                               g_fCarFrontWidth = 2;
*/
float                               g_fCarBackWidth = 2.0f;

UINT                                g_fNumOfMeshes;

float                               g_fCarRotVel;
float                               g_fCarRotAccel;
const float                         g_fCarRotMaxVel = 0.01f;
const float                         g_fCarRotMinVel = -0.01f;
const float                         g_fCarRotForce = 0.04f;

const float                         g_fCarMinVelocity = 0.01f;
const float                         g_fCarMaxVelocity = 0.50f;
const float                         g_fCarForce = 0.005f;
float                               g_fCarVelocity = 0.1f;;
float                               g_fCarAccel;
float3                              g_vCarDir(0.0f, 0.0f, 1.0f);

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
enum IDC_HUD
{
    IDC_STATIC = -1,
    IDC_CHANGEDEVICE,
    IDC_GRASS_LOD_BIAS_LABEL,
    IDC_GRASS_LOD_BIAS_SLYDER,
    IDC_GRASS_SUBSCATTER_LABEL,
    IDC_GRASS_SUBSCATTER_SLYDER,
    IDC_GRASS_AMBIENT_LABEL,
    IDC_LG_RGB_LABEL,
    IDC_LG_R_SLYDER,
    IDC_LG_G_SLYDER,
    IDC_LG_B_SLYDER,
    IDC_FOG_RGB_LABEL,
    IDC_FOG_R_SLYDER,
    IDC_FOG_G_SLYDER,
    IDC_FOG_B_SLYDER,

    IDC_GRASS_AMBIENT_SLYDER,
    IDC_GRASS_DIFFUSE_LABEL,
    IDC_GRASS_DIFFUSE_SLYDER,
    IDC_GRASS_QUALITY_LABEL,
    IDC_GRASS_QUALITY_SLYDER,
    IDC_HEIGHTSCALE_SLYDER,
    IDC_HEIGHTSCALE_LABEL,
    IDC_GRASS_WIND_LABEL,
    IDC_GRASS_WIND_FORCE_SLYDER,
    IDC_GRASS_WIND_BIAS_SLYDER,
    IDC_GRASS_WIND_BIAS_LABEL,
    IDC_GRASS_WIND_SCALE_SLYDER,
    IDC_GRASS_WIND_SCALE_LABEL,
    IDC_GRASS_WIND_PHASE_SLYDER,
    IDC_GRASS_WIND_PHASE_LABEL,
    IDC_GRASS_WIND_SPEED_SLYDER,
    IDC_GRASS_WIND_SPEED_LABEL,
    IDC_GRASS_SEG_MASS_LABEL,
    IDC_GRASS_SEG_MASS_SLYDER,
    IDC_GRASS_SEG_HARD_LABEL,
    IDC_GRASS_SEG_HARD_1_SLYDER,
    IDC_GRASS_SEG_HARD_2_SLYDER,
    IDC_GRASS_SEG_HARD_3_SLYDER,
    IDC_SAVE_BTN,
    IDC_TERR_RGB_LABEL,
    IDC_TERR_TILE_LABEL,
    IDC_TERR_TILE_SLYDER,
    IDC_TERR_R_SLYDER,
    IDC_TERR_G_SLYDER,
    IDC_TERR_B_SLYDER,
    IDC_CAM_SPEED_LABEL,
    IDC_CAM_SPEED_SLYDER,
    IDC_CAM_POS_LABEL,
    IDC_SAMPLE_COUNT,
	IDC_CAMERA_TYPE,
    IDC_TOGGLE_WIREFRAME
};

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                         void* pUserContext );
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );

bool CALLBACK IsD3D10DeviceAcceptable( UINT Adapter, UINT Output, D3D10_DRIVER_TYPE DeviceType,
                                      DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnD3D10CreateDevice( ID3D10Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext );
HRESULT CALLBACK OnD3D10SwapChainResized( ID3D10Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                         const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void CALLBACK OnD3D10SwapChainReleasing( void* pUserContext );
void CALLBACK OnD3D10DestroyDevice( void* pUserContext );
void CALLBACK OnD3D10FrameRender( ID3D10Device* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );

void InitMeshes( ID3D10Device *a_pD3DDevice );
void UpdateMeshes( );
void InitApp();
void RenderText();
HRESULT CreateRenderTarget( ID3D10Device* pd3dDevice, UINT uiWidth, UINT uiHeight, UINT uiSampleCount,
                           UINT uiSampleQuality );
