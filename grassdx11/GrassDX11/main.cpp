#include <string>

#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTmisc.h"
#include "DXUTCamera.h"
#include "DXUTSettingsDlg.h"
#include "SDKmisc.h"
#include "SDKmesh.h"

#include "main.h"

#include "camera.h"
#include "StateManager.h"

#pragma warning( disable : 4100 )

using namespace DirectX;

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
CFirstPersonCamera          *g_Camera;               // A model viewing camera

CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg             g_SettingsDlg;          // Device settings dialog
CDXUTTextHelper*            g_pTxtHelper = nullptr;
CDXUTDialog                 g_HUD;                  // dialog for standard controls
CDXUTDialog                 g_SampleUI;             // dialog for sample specific controls

// Scene Effect vars
ID3DX11EffectMatrixVariable* g_mWorld = nullptr;
ID3DX11EffectMatrixVariable* g_mViewProj = nullptr;
ID3DX11EffectMatrixVariable* g_mInvCamView = nullptr;
ID3DX11EffectMatrixVariable* g_mLightViewProj = nullptr;
ID3DX11EffectMatrixVariable* g_mNormalMatrix = nullptr;
//ID3DX11EffectScalarVariable* g_fTime = nullptr;


// Direct3D 11 resources
ID3D11VertexShader*         g_pVertexShader11 = nullptr;
ID3D11PixelShader*          g_pPixelShader11 = nullptr;
ID3D11InputLayout*          g_pLayout11 = nullptr;
ID3D11SamplerState*         g_pSamLinear = nullptr;

//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------
#pragma pack(push,1)
struct CB_VS_PER_OBJECT
{
    XMFLOAT4X4  m_mWorldViewProjection;
    XMFLOAT4X4  m_mWorld;
    XMFLOAT4    m_MaterialAmbientColor;
    XMFLOAT4    m_MaterialDiffuseColor;
};

struct CB_VS_PER_FRAME
{
    XMFLOAT3    m_vLightDir;
    float       m_fTime;
    XMFLOAT4    m_LightDiffuse;
};
#pragma pack(pop)

ID3D11Buffer*                       g_pcbVSPerObject11 = nullptr;
ID3D11Buffer*                       g_pcbVSPerFrame11 = nullptr;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3
#define IDC_TOGGLEWARP          4



//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#ifdef _DEBUG
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // DXUT will create and use the best device
    // that is available on the system depending on which D3D callbacks are set below

    // Set DXUT callbacks
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

    DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
    DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );

    InitApp();
    DXUTInit( true, true, nullptr ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true );
    DXUTCreateWindow( L"GrassDX11" );

    // Only require 10-level hardware, change to D3D_FEATURE_LEVEL_11_0 to require 11-class hardware
    // Switch to D3D_FEATURE_LEVEL_9_x for 10level9 hardware
    DXUTCreateDevice( D3D_FEATURE_LEVEL_10_0, true, 1000, 800 );

    DXUTMainLoop(); // Enter into the DXUT render loop

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
    g_SettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );

    g_HUD.SetCallback( OnGUIEvent );
    int iY = 30;
    int iYo = 26;
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 0, iY, 170, 22 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 0, iY += iYo, 170, 22, VK_F2 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 0, iY += iYo, 170, 22, VK_F3 );
    g_HUD.AddButton( IDC_TOGGLEWARP, L"Toggle WARP (F4)", 0, iY += iYo, 170, 22, VK_F4 );

    g_SampleUI.SetCallback( OnGUIEvent ); iY = 10;
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text.
//--------------------------------------------------------------------------------------
void RenderText()
{
    g_pTxtHelper->Begin();
    g_pTxtHelper->SetInsertionPos( 5, 5 );
    g_pTxtHelper->SetForegroundColor( Colors::Yellow );
    g_pTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
    g_pTxtHelper->DrawTextLine( DXUTGetDeviceStats() );

	XMVECTOR vEye = g_Camera->GetEyePt();
	XMFLOAT3 eye;
	XMStoreFloat3(&eye, vEye);

	XMVECTOR vLookAt = g_Camera->GetLookAtPt();
	XMFLOAT3 lookAt;
	XMStoreFloat3(&lookAt, vLookAt);
	
	WCHAR eyeStr[100];
	swprintf(eyeStr, sizeof(eyeStr), L"Eye: X = %f, Y = %f, Z = % f", eye.x, eye.y, eye.z);

	WCHAR lookAtStr[100];
	swprintf(lookAtStr, sizeof(lookAtStr), L"LookAt: X = %f, Y = %f, Z = % f", lookAt.x, lookAt.y, lookAt.z);

	g_pTxtHelper->DrawTextLine(eyeStr);
	g_pTxtHelper->DrawTextLine(lookAtStr);

    g_pTxtHelper->End();
}


//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output,
                                       const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext )
{
    HRESULT hr;

    auto pd3dImmediateContext = DXUTGetD3D11DeviceContext();
    V_RETURN( g_DialogResourceManager.OnD3D11CreateDevice( pd3dDevice, pd3dImmediateContext ) );
    V_RETURN( g_SettingsDlg.OnD3D11CreateDevice( pd3dDevice ) );
    g_pTxtHelper = new CDXUTTextHelper( pd3dDevice, pd3dImmediateContext, &g_DialogResourceManager, 15 );

	// Load camera parameters
	std::ifstream InFile;
	
	InFile.open("config/camera.ini");
	InFile >> g_fCameraHeight;
	InFile >> g_fCameraHeightMin;
	InFile >> g_fCameraHeightMax;
	InFile >> g_fCameraMeshDist;
	InFile >> g_fCameraMeshDistMin;
	InFile >> g_fCameraMeshDistMax;
	InFile.close();
	InFile.clear();

	// Create rasterizer states
	GetGlobalStateManager().SetDevice(pd3dDevice, pd3dImmediateContext); 
	
	D3D11_RASTERIZER_DESC CurrentRasterizerState;
	CurrentRasterizerState.FillMode = D3D11_FILL_SOLID;
	CurrentRasterizerState.CullMode = D3D11_CULL_NONE;
	CurrentRasterizerState.FrontCounterClockwise = true;
	CurrentRasterizerState.DepthBias = false;
	CurrentRasterizerState.DepthBiasClamp = 0;
	CurrentRasterizerState.SlopeScaledDepthBias = 0;
	CurrentRasterizerState.DepthClipEnable = true;
	CurrentRasterizerState.ScissorEnable = false;
	CurrentRasterizerState.MultisampleEnable = true;
	CurrentRasterizerState.AntialiasedLineEnable = false;
	GetGlobalStateManager().AddRasterizerState("EnableMSAA", CurrentRasterizerState);
	
	CurrentRasterizerState.CullMode = D3D11_CULL_FRONT;
	GetGlobalStateManager().AddRasterizerState("EnableMSAACulling", CurrentRasterizerState);
	
	CurrentRasterizerState.CullMode = D3D11_CULL_NONE;
	CurrentRasterizerState.FillMode = D3D11_FILL_WIREFRAME;
	GetGlobalStateManager().AddRasterizerState("EnableMSAA_Wire", CurrentRasterizerState);
	
	CurrentRasterizerState.CullMode = D3D11_CULL_FRONT;
	CurrentRasterizerState.FillMode = D3D11_FILL_WIREFRAME;
	GetGlobalStateManager().AddRasterizerState("EnableMSAACulling_Wire", CurrentRasterizerState);
	
	// Create grass field
	g_GrassInitState.InitState[0].fMaxQuality = 0.0f;//0.7f;
	g_GrassInitState.InitState[0].dwBladesPerPatchSide = 20;
	g_GrassInitState.InitState[0].dwPatchesPerSide = 37;//40;//43;//45;//32;//50;
	g_GrassInitState.InitState[0].fMostDetailedDist = 2.0f;//* g_fMeter;
	g_GrassInitState.InitState[0].fLastDetailedDist = 140.0;//85;//150.0f;// * g_fMeter;
	g_GrassInitState.InitState[0].fGrassRadius = 140.0;//85;//150.0f;// * g_fMeter;
	g_GrassInitState.InitState[0].pD3DDevice = pd3dDevice;
	g_GrassInitState.InitState[0].pD3DDeviceCtx = pd3dImmediateContext;
	
	g_GrassInitState.InitState[0].uNumCollidedPatchesPerMesh = 10;
	g_GrassInitState.InitState[0].uMaxColliders = MAX_NUM_MESHES;
	
	g_GrassInitState.InitState[1] = g_GrassInitState.InitState[0];
	g_GrassInitState.InitState[2] = g_GrassInitState.InitState[0];
	//some differences...
	g_GrassInitState.InitState[0].sLowGrassTexPath = L"resources/LowGrass.dds";
	g_GrassInitState.InitState[0].sIndexTexPath = L"resources/IndexType1.dds";
	g_GrassInitState.InitState[1].sIndexTexPath = L"resources/IndexType2.dds";
	g_GrassInitState.InitState[2].sIndexTexPath = L"resources/IndexType3.dds";
	//g_GrassInitState.InitState[1].dwBladesPerPatchSide       = 2;
	g_GrassInitState.InitState[2].dwBladesPerPatchSide = 2;
	g_GrassInitState.InitState[0].sTexPaths.push_back(L"resources/GrassType1.dds");
	g_GrassInitState.InitState[0].sTexPaths.push_back(L"resources/GrassType1_1.dds");
	//g_GrassInitState.InitState[1].sTexPaths.push_back(L"resources/GrassType2.dds");
	g_GrassInitState.InitState[1].sTexPaths.push_back(L"resources/GrassType2_1.dds");
	g_GrassInitState.InitState[2].sTexPaths.push_back(L"resources/GrassType3.dds");
	g_GrassInitState.InitState[2].sTopTexPaths.push_back(L"resources/Top1.dds");
	g_GrassInitState.InitState[2].sTopTexPaths.push_back(L"resources/Top2.dds");
	g_GrassInitState.InitState[2].sTopTexPaths.push_back(L"resources/Top3.dds");
	g_GrassInitState.InitState[2].sTopTexPaths.push_back(L"resources/Top4.dds");
	g_GrassInitState.InitState[2].sTopTexPaths.push_back(L"resources/Top5.dds");
	g_GrassInitState.InitState[2].sTopTexPaths.push_back(L"resources/Top6.dds");
	g_GrassInitState.InitState[2].sTopTexPaths.push_back(L"resources/Top7.dds");
	g_GrassInitState.InitState[0].sEffectPath = L"Shaders/GrassType1.fx";
	g_GrassInitState.InitState[1].sEffectPath = L"Shaders/GrassType2.fx";
	g_GrassInitState.InitState[2].sEffectPath = L"Shaders/GrassType3.fx";
	g_GrassInitState.InitState[0].sSeatingTexPath = L"resources/SeatingType1.dds";
	g_GrassInitState.InitState[1].sSeatingTexPath = L"resources/SeatingType2.dds";
	g_GrassInitState.InitState[2].sSeatingTexPath = L"resources/SeatingType3.dds";
	
	g_GrassInitState.InitState[0].sSubTypesPath = L"config/T1SubTypes.cfg";
	g_GrassInitState.InitState[1].sSubTypesPath = L"config/T2SubTypes.cfg";
	g_GrassInitState.InitState[2].sSubTypesPath = L"config/T3SubTypes.cfg";
	g_GrassInitState.InitState[0].fCameraMeshDist = g_fCameraMeshDistMax;
	g_GrassInitState.InitState[1].fCameraMeshDist = g_fCameraMeshDistMax;
	g_GrassInitState.InitState[2].fCameraMeshDist = g_fCameraMeshDistMax;
	g_GrassInitState.sSceneEffectPath = L"Shaders/SceneEffect.fx";
	g_GrassInitState.sNoiseMapPath = L"resources/Noise.dds";
	g_GrassInitState.sColorMapPath = L"resources/GrassColor.dds";
	g_GrassInitState.fHeightScale = g_fHeightScale;
	g_GrassInitState.fTerrRadius = 400.0f;
	g_pGrassField = new GrassFieldManager(g_GrassInitState);
	g_pTerrTile = g_pGrassField->SceneEffect()->GetVariableByName("g_fTerrTile")->AsScalar();
	
	/*Loading colors*/
	InFile.open("config/colors.ini");
	InFile >> g_vFogColor.x >> g_vFogColor.y >> g_vFogColor.z;
	InFile >> g_vTerrRGB.x >> g_vTerrRGB.y >> g_vTerrRGB.z;
	InFile >> g_vGrassSpecular.x >> g_vGrassSpecular.y >> g_vGrassSpecular.z;
	InFile.close();

	/****************/
	XM_TO_V(g_vTerrRGB, vTerrRGB, 3);
	XM_TO_V(g_vGrassSpecular, vGrassSpecular, 4);
	XM_TO_V(g_vFogColor, vFogColor, 4);

	g_pGrassField->SetTerrRGB(vTerrRGB);
	g_pTerrTile->SetFloat(g_fTerrTile);
	g_pGrassField->SetWindMapTile(6);
	//    g_pGrassField->SetWindMapTile(4);
	g_pGrassField->SetSegMass(g_fMass);
	g_pGrassField->SetHardness(g_fHardness);
	g_pGrassField->SetGrassLodBias(g_fGrassLodBias);
	g_pGrassField->SetSubScatterGamma(g_fGrassSubScatterGamma);
	g_pGrassField->SetGrassAmbient(g_fGrassAmbient);
	g_pGrassField->SetWindStrength(g_fWindStrength);
	g_pGrassField->SetWindSpeed(g_fWindTexSpeed);
	//g_pGrassField->SetHeightScale(g_fHeightScale);
	g_pGrassField->SetQuality(g_fQuality);
	g_pGrassField->SetLowGrassDiffuse(vGrassSpecular);
	g_pGrassField->SetFogColor(vFogColor);
	g_pGrassField->SetWindBias(g_fWindBias);
	g_pGrassField->SetWindScale(g_fWindScale);
	
	//InitMeshes(pd3dDevice);
	
	
	// Setup the camera's view parameters
	float height_scale, grass_radius;
	Terrain* const terr = g_pGrassField->GetTerrain(&height_scale, &grass_radius);
	
	g_Camera = new LandscapeCamera(g_fCameraHeight, terr, height_scale, grass_radius);
	
	g_Camera->SetViewParams(XMLoadFloat3(&g_vCameraEyeStart), XMLoadFloat3(&g_vCameraAtStart));
	g_Camera->SetScalers(0.01f, g_fCameraSpeed /* g_fMeter*/);

	
    g_HUD.GetButton( IDC_TOGGLEWARP )->SetEnabled( true );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                         const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );
    V_RETURN( g_SettingsDlg.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
    g_Camera->SetProjParams( XM_PI / 4, fAspectRatio, 0.1f, 1000.0f );
    

	//g_Camera->SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
    //g_Camera->SetButtonMasks( MOUSE_LEFT_BUTTON, MOUSE_WHEEL, MOUSE_MIDDLE_BUTTON );

    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
    g_HUD.SetSize( 170, 170 );
    g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 300 );
    g_SampleUI.SetSize( 170, 300 );

    return S_OK;
}



void RenderGrass(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dDeviceCtx, XMMATRIX& mView, XMMATRIX& mProj, float a_fElapsedTime)
{
	XMMATRIX mViewProj;
	mViewProj = mul(mView, mProj);
	
	g_pGrassField->SetTime(g_fTime);

	g_pGrassField->SetViewProjMtx(mViewProj);
	g_pGrassField->SetViewMtx(mView);
	g_pGrassField->SetProjMtx(mProj);

	// Draw Grass
	XMVECTOR vCamDir = g_Camera->GetLookAtPt() - g_Camera->GetEyePt();
	
	g_pGrassField->Update(vCamDir, g_Camera->GetEyePt(), g_pMeshes, 0/*g_fNumOfMeshes*/, a_fElapsedTime);
	g_pGrassField->Render();
	
	
	if (GetGlobalStateManager().UseWireframe())
		GetGlobalStateManager().SetRasterizerState("EnableMSAACulling_Wire");
	else
		GetGlobalStateManager().SetRasterizerState("EnableMSAACulling");
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                 float fElapsedTime, void* pUserContext )
{	
	HRESULT hr;
    // If the settings dialog is being shown, then render it instead of rendering the app's scene
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.OnRender( fElapsedTime );
        return;
    }       

    auto pRTV = DXUTGetD3D11RenderTargetView();
    pd3dImmediateContext->ClearRenderTargetView( pRTV, Colors::MidnightBlue );

    // Clear the depth stencil
    auto pDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

	// Get the projection & view matrix from the camera class
    XMMATRIX mView = g_Camera->GetViewMatrix();
    XMMATRIX mProj = g_Camera->GetProjMatrix();

	g_fTime += fElapsedTime;

	// Render grass
	RenderGrass(pd3dDevice, pd3dImmediateContext, mView, mProj, fElapsedTime);
	
    DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"HUD / Stats" );
    g_HUD.OnRender( fElapsedTime );
    g_SampleUI.OnRender( fElapsedTime );
    RenderText();
    DXUT_EndPerfEvent();

    static ULONGLONG timefirst = GetTickCount64();
    if ( GetTickCount64() - timefirst > 5000 )
    {    
        OutputDebugString( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
        OutputDebugString( L"\n" );
        timefirst = GetTickCount64();
    }
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
    g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D11DestroyDevice();
    g_SettingsDlg.OnD3D11DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();
    SAFE_DELETE( g_pTxtHelper );

    SAFE_RELEASE( g_pVertexShader11 );
    SAFE_RELEASE( g_pPixelShader11 );
    SAFE_RELEASE( g_pLayout11 );
    SAFE_RELEASE( g_pSamLinear );

    // Delete additional render resources here...

    SAFE_RELEASE( g_pcbVSPerObject11 );
    SAFE_RELEASE( g_pcbVSPerFrame11 );
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    // Update the camera's position based on user input 
    g_Camera->FrameMove( fElapsedTime );
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
    // Pass messages to dialog resource manager calls so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass messages to settings dialog if its active
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass all remaining windows messages to camera so it can respond to user input
	if (g_Camera != NULL) {
		g_Camera->HandleMessages(hWnd, uMsg, wParam, lParam);
	}

    return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN:
            DXUTToggleFullScreen();
            break;
        case IDC_TOGGLEREF:
            DXUTToggleREF();
            break;
        case IDC_TOGGLEWARP:
            DXUTToggleWARP();
            break;
        case IDC_CHANGEDEVICE:
            g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() );
            break;
    }
}
