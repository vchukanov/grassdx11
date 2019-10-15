#include "main.h"

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // DXUT will create and use the best device (either D3D9 or D3D10) 
    // that is available on the system depending on which D3D callbacks are set below

    // Set DXUT callbacks
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( KeyboardProc );
    DXUTSetCallbackFrameMove( OnFrameMove );

    DXUTSetCallbackD3D10DeviceAcceptable( IsD3D10DeviceAcceptable );
    DXUTSetCallbackD3D10DeviceCreated( OnD3D10CreateDevice );
    DXUTSetCallbackD3D10SwapChainResized( OnD3D10SwapChainResized );
    DXUTSetCallbackD3D10SwapChainReleasing( OnD3D10SwapChainReleasing );
    DXUTSetCallbackD3D10DeviceDestroyed( OnD3D10DestroyDevice );
    DXUTSetCallbackD3D10FrameRender( OnD3D10FrameRender );

    InitApp();
    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"Grass Demo 06.10.10" );
    //DXUTCreateDevice( true, 1024, 768 );
    DXUTCreateDevice( true, 1280, 1024 );
    DXUTMainLoop(); // Enter into the DXUT render loop

    return DXUTGetExitCode();
}


void InitMeshes( ID3D10Device *a_pD3DDevice )
{    
    D3DXVECTOR4 vPosAndRad;

    vPosAndRad.x = 0.0f;
    vPosAndRad.z = 0.0f;
    vPosAndRad.w = 1.2f;
    vPosAndRad.y = 0.0f;

    g_fNumOfMeshes = 0;
}

//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
static UINT CameraType = 0;
void InitApp()
{
    g_D3DSettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );
    g_SampleUI.SetCallback( OnGUIEvent );

    g_HUD.SetCallback( OnGUIEvent ); 
    int iY = 10;
    int iYInc = 21;
    WCHAR sStr[MAX_PATH];
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 25, iY += iYInc, 125, 22, VK_F2 );
    g_HUD.AddButton( IDC_SAVE_BTN    , L"Save colors (F3)", 25, iY += iYInc, 125, 22, VK_F3 );
    iYInc = 18;

    //swprintf_s( sStr, MAX_PATH, L"Camera at: (%.1f, %.1f, %.1f)", g_Camera->GetEyePt()->x, g_Camera->GetEyePt()->y, g_Camera->GetEyePt()->z );
    swprintf_s( sStr, MAX_PATH, L"Camera at: (0, 0, 0)");
    g_HUD.AddStatic( IDC_CAM_POS_LABEL, sStr, 5, iY += iYInc, 180, 22 );

    swprintf_s( sStr, MAX_PATH, L"Camera speed: %.0fm/s", g_fCameraSpeed );
    g_HUD.AddStatic( IDC_CAM_SPEED_LABEL, sStr, 20, iY += iYInc, 140, 22 );
    g_HUD.AddSlider( IDC_CAM_SPEED_SLYDER, 20, iY += iYInc, 135, 22, 1, 100, ( int )( g_fCameraSpeed ) );

    /*swprintf_s( sStr, MAX_PATH, L"Quality Value: %.2f", g_fQuality );
    g_HUD.AddStatic( IDC_GRASS_QUALITY_LABEL, sStr, 20, iY += iYInc, 140, 22 );
    g_HUD.AddSlider( IDC_GRASS_QUALITY_SLYDER, 20, iY += iYInc, 135, 22, 0, 100, ( int )( g_fQuality * 100 ) );

    swprintf_s( sStr, MAX_PATH, L"Lod Smooth Value: %.2f", g_fGrassLodBias );
    g_HUD.AddStatic( IDC_GRASS_LOD_BIAS_LABEL, sStr, 20, iY += iYInc, 140, 22 );
    g_HUD.AddSlider( IDC_GRASS_LOD_BIAS_SLYDER, 20, iY += iYInc, 135, 22, 0, 100, ( int )( g_fGrassLodBias * 100 ) );

    swprintf_s( sStr, MAX_PATH, L"SubScatter gamma: %.2f", g_fGrassSubScatterGamma );
    g_HUD.AddStatic( IDC_GRASS_SUBSCATTER_LABEL, sStr, 20, iY += iYInc, 140, 22 );
    g_HUD.AddSlider( IDC_GRASS_SUBSCATTER_SLYDER, 20, iY += iYInc, 135, 22, 0, 100, ( int )( g_fGrassSubScatterGamma * 100 ) );

    swprintf_s( sStr, MAX_PATH, L"Grass Ambient: %.2f", g_fGrassAmbient );
    g_HUD.AddStatic( IDC_GRASS_AMBIENT_LABEL, sStr, 20, iY += iYInc, 140, 22 );
    g_HUD.AddSlider( IDC_GRASS_AMBIENT_SLYDER, 20, iY += iYInc, 135, 22, 0, 100, ( int )( g_fGrassAmbient * 100 ) );
*/
    swprintf_s( sStr, MAX_PATH, L"Specular (%.2f,%.2f,%.2f)", g_vGrassSpecular.x, g_vGrassSpecular.y, g_vGrassSpecular.z );
    g_HUD.AddStatic( IDC_LG_RGB_LABEL, sStr, 20, iY += iYInc, 140, 22 );
    g_HUD.AddSlider( IDC_LG_R_SLYDER, 20, iY += iYInc, 135, 22, 0, 100, ( int )( g_vGrassSpecular.x * 100 ) );
    g_HUD.AddSlider( IDC_LG_G_SLYDER, 20, iY += iYInc, 135, 22, 0, 100, ( int )( g_vGrassSpecular.y * 100 ) );
    g_HUD.AddSlider( IDC_LG_B_SLYDER, 20, iY += iYInc, 135, 22, 0, 100, ( int )( g_vGrassSpecular.z * 100 ) );

    /*swprintf_s( sStr, MAX_PATH, L"HeightScale: %.0f", g_fHeightScale );
    g_HUD.AddStatic( IDC_HEIGHTSCALE_LABEL, sStr, 20, iY += iYInc, 140, 22 );
    g_HUD.AddSlider( IDC_HEIGHTSCALE_SLYDER, 20, iY += iYInc, 135, 22, 1, 20, ( int )( g_fHeightScale / 5.f ) );*/

    swprintf_s( sStr, MAX_PATH, L"Wind Strength: %.4f", g_fWindStrength);
    g_HUD.AddStatic( IDC_GRASS_WIND_LABEL, sStr, 20, iY += iYInc, 140, 22 );
    g_HUD.AddSlider( IDC_GRASS_WIND_FORCE_SLYDER, 20, iY += iYInc, 135, 22, 10, 6500, ( int )( g_fWindStrength * 10000 ) );

    /*swprintf_s( sStr, MAX_PATH, L"Wind Bias: %.4f", g_fWindBias);
    g_HUD.AddStatic( IDC_GRASS_WIND_BIAS_LABEL, sStr, 20, iY += iYInc, 140, 22 );
    g_HUD.AddSlider( IDC_GRASS_WIND_BIAS_SLYDER, 20, iY += iYInc, 135, 22, 0, 1000, ( int )( g_fWindBias * 1000 ) );

    swprintf_s( sStr, MAX_PATH, L"Wind Scale: %.4f", g_fWindScale);
    g_HUD.AddStatic( IDC_GRASS_WIND_SCALE_LABEL, sStr, 20, iY += iYInc, 140, 22 );
    g_HUD.AddSlider( IDC_GRASS_WIND_SCALE_SLYDER, 20, iY += iYInc, 135, 22, 0, 1000, ( int )( g_fWindScale * 100 ) );

    swprintf_s( sStr, MAX_PATH, L"WindTex Speed: %.2f", g_fWindTexSpeed);
    g_HUD.AddStatic( IDC_GRASS_WIND_SPEED_LABEL, sStr, 20, iY += iYInc, 140, 22 );
    g_HUD.AddSlider( IDC_GRASS_WIND_SPEED_SLYDER, 20, iY += iYInc, 135, 22, 0, 1000, ( int )( g_fWindTexSpeed * 100 ) );

    swprintf_s( sStr, MAX_PATH, L"WindTex Tile: %.2f", g_fWindTexTile);
    g_HUD.AddStatic( IDC_GRASS_WIND_PHASE_LABEL, sStr, 20, iY += iYInc, 140, 22 );
    g_HUD.AddSlider( IDC_GRASS_WIND_PHASE_SLYDER, 20, iY += iYInc, 135, 22, 0, 100, ( int )( g_fWindTexTile * 10 ) );

    swprintf_s( sStr, MAX_PATH, L"Hardness: %.2f", g_fHardness );
    g_HUD.AddStatic( IDC_GRASS_SEG_HARD_LABEL, sStr, 20, iY += iYInc, 160, 22 );
    g_HUD.AddSlider( IDC_GRASS_SEG_HARD_1_SLYDER, 20, iY += iYInc, 135, 22, 0, 500, ( int )( g_fHardness * 100 ) );

    swprintf_s( sStr, MAX_PATH, L"Segment Mass: %.3f", g_fMass );
    g_HUD.AddStatic( IDC_GRASS_SEG_MASS_LABEL, sStr, 20, iY += iYInc, 140, 22 );
    g_HUD.AddSlider( IDC_GRASS_SEG_MASS_SLYDER, 20, iY += iYInc, 135, 22, 1, 1000, ( int )( g_fMass * 1000 ) );
*/
    swprintf_s( sStr, MAX_PATH, L"Diffuse: (%.2f,%.2f,%.2f)", g_vTerrRGB.x, g_vTerrRGB.y, g_vTerrRGB.z );
    g_HUD.AddStatic( IDC_TERR_RGB_LABEL, sStr, 20, iY += iYInc, 140, 22 );
    g_HUD.AddSlider( IDC_TERR_R_SLYDER, 20, iY += iYInc, 135, 22, 0, 100, ( int )( g_vTerrRGB.x * 100 ) );
    g_HUD.AddSlider( IDC_TERR_G_SLYDER, 20, iY += iYInc, 135, 22, 0, 100, ( int )( g_vTerrRGB.y * 100 ) );
    g_HUD.AddSlider( IDC_TERR_B_SLYDER, 20, iY += iYInc, 135, 22, 0, 100, ( int )( g_vTerrRGB.z * 100 ) );

    swprintf_s( sStr, MAX_PATH, L"Fog Color: (%.2f,%.2f,%.2f)", g_vFogColor.x, g_vFogColor.y, g_vFogColor.z );
    g_HUD.AddStatic( IDC_FOG_RGB_LABEL, sStr, 20, iY += iYInc, 140, 22 );
    g_HUD.AddSlider( IDC_FOG_R_SLYDER, 20, iY += iYInc, 135, 22, 0, 100, ( int )( g_vFogColor.x * 100 ) );
    g_HUD.AddSlider( IDC_FOG_G_SLYDER, 20, iY += iYInc, 135, 22, 0, 100, ( int )( g_vFogColor.y * 100 ) );
    g_HUD.AddSlider( IDC_FOG_B_SLYDER, 20, iY += iYInc, 135, 22, 0, 100, ( int )( g_vFogColor.z * 100 ) );

    /*swprintf_s( sStr, MAX_PATH, L"Terr Tile %.2f", g_fTerrTile );
    g_HUD.AddStatic( IDC_TERR_TILE_LABEL, sStr, 20, iY += iYInc, 140, 22 );
    g_HUD.AddSlider( IDC_TERR_TILE_SLYDER, 20, iY += iYInc, 135, 22, 0, 1000, ( int )( g_fTerrTile * 10 ) );

    g_HUD.AddStatic( IDC_STATIC, L"MSAA Samples(F4)", 20, iY += iYInc, 105, 25 );
    CDXUTComboBox* pComboBox = NULL;
    g_HUD.AddComboBox( IDC_SAMPLE_COUNT, 20, iY += iYInc, 140, 24, VK_F4, false, &pComboBox );
    if( pComboBox )
        pComboBox->SetDropHeight( 30 );*/

    CDXUTComboBox* pComboBox = NULL;
    g_HUD.AddStatic( IDC_STATIC, L"Camera Type", 20, iY += iYInc, 105, 25 );
    pComboBox = NULL;
    g_HUD.AddComboBox( IDC_CAMERA_TYPE, 20, iY += iYInc, 140, 24, VK_F4, false, &pComboBox );
    if( pComboBox )
    {
        pComboBox->SetDropHeight(30);
        pComboBox->AddItem(L"Normal camera", IntToPtr(CAMERA_NORMAL));
        pComboBox->AddItem(L"Terrain camera", IntToPtr(CAMERA_TERRAIN));
        pComboBox->AddItem(L"Mesh camera", IntToPtr(CAMERA_MESH));
    }
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D10 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    pDeviceSettings->d3d10.SyncInterval = 0;
    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( ( DXUT_D3D9_DEVICE == pDeviceSettings->ver && pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF ) ||
            ( DXUT_D3D10_DEVICE == pDeviceSettings->ver &&
              pDeviceSettings->d3d10.DriverType == D3D10_DRIVER_TYPE_REFERENCE ) )
            DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
    }

    // Disable MSAA settings from the settings dialog
    g_D3DSettingsDlg.GetDialogControl()->GetComboBox( DXUTSETTINGSDLG_D3D10_MULTISAMPLE_COUNT )->SetEnabled( false );
    g_D3DSettingsDlg.GetDialogControl()->GetComboBox( DXUTSETTINGSDLG_D3D10_MULTISAMPLE_QUALITY )->SetEnabled( false );
    g_D3DSettingsDlg.GetDialogControl()->GetStatic( DXUTSETTINGSDLG_D3D10_MULTISAMPLE_COUNT_LABEL )->SetEnabled( false );
    g_D3DSettingsDlg.GetDialogControl()->GetStatic( DXUTSETTINGSDLG_D3D10_MULTISAMPLE_QUALITY_LABEL )->SetEnabled( false );
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
    if( g_D3DSettingsDlg.IsActive() )
    {
        g_D3DSettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
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
    if (g_Camera != NULL)
	    g_Camera->HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    WCHAR sStr[MAX_PATH] = {0};
    static std::ofstream OutFile;
    switch( nControlID )
    {
        case IDC_CHANGEDEVICE:
        {
            g_D3DSettingsDlg.SetActive( !g_D3DSettingsDlg.IsActive() ); 
            break;
        }

        case IDC_CAM_SPEED_SLYDER:
        {
            g_fCameraSpeed = (float)g_HUD.GetSlider( IDC_CAM_SPEED_SLYDER )->GetValue();
            swprintf_s( sStr, MAX_PATH, L"Camera speed: %.0fm/s", g_fCameraSpeed );
            g_HUD.GetStatic( IDC_CAM_SPEED_LABEL )->SetText(sStr);
            g_Camera->SetScalers(0.01f, g_fCameraSpeed /* g_fMeter*/);
            break;
        }
        
        case IDC_GRASS_LOD_BIAS_SLYDER:
        {
            g_fGrassLodBias = (float)g_HUD.GetSlider( IDC_GRASS_LOD_BIAS_SLYDER )->GetValue() / 100.0f;
            swprintf_s( sStr, MAX_PATH, L"Lod Smooth Value: %.2f", g_fGrassLodBias );
            g_HUD.GetStatic( IDC_GRASS_LOD_BIAS_LABEL )->SetText( sStr );
            g_pGrassField->SetGrassLodBias(g_fGrassLodBias);
            break;
        }
        
        case IDC_GRASS_QUALITY_SLYDER:
        {
            g_fQuality = (float)g_HUD.GetSlider( IDC_GRASS_QUALITY_SLYDER )->GetValue() / 100.0f;
            swprintf_s( sStr, MAX_PATH, L"Quality Value: %.2f", g_fQuality );
            g_HUD.GetStatic( IDC_GRASS_QUALITY_LABEL )->SetText( sStr );
            g_pGrassField->SetQuality(g_fQuality);
            break;
        }

        case IDC_GRASS_SUBSCATTER_SLYDER:
        {
            g_fGrassSubScatterGamma = (float)g_HUD.GetSlider( IDC_GRASS_SUBSCATTER_SLYDER )->GetValue() / 100.0f;
            swprintf_s( sStr, MAX_PATH, L"SubScatter gamma: %.2f", g_fGrassSubScatterGamma );
            g_HUD.GetStatic( IDC_GRASS_SUBSCATTER_LABEL )->SetText( sStr );
            g_pGrassField->SetSubScatterGamma(g_fGrassSubScatterGamma);
            break;
        }

        case IDC_GRASS_AMBIENT_SLYDER:
        {
            g_fGrassAmbient = (float)g_HUD.GetSlider( IDC_GRASS_AMBIENT_SLYDER )->GetValue() / 100.0f;
            swprintf_s( sStr, MAX_PATH, L"Grass Ambient: %.2f", g_fGrassAmbient );
            g_HUD.GetStatic( IDC_GRASS_AMBIENT_LABEL )->SetText( sStr );
            g_pGrassField->SetGrassAmbient(g_fGrassAmbient);
            break;
        }

        /*case IDC_HEIGHTSCALE_SLYDER:
        {
            g_fHeightScale = (float)g_HUD.GetSlider( IDC_HEIGHTSCALE_SLYDER )->GetValue() * 5.0f;
            swprintf_s( sStr, MAX_PATH, L"HeightScale: %.0f", g_fHeightScale );
            g_HUD.GetStatic( IDC_HEIGHTSCALE_LABEL )->SetText( sStr );
            g_pGrassField->SetHeightScale(g_fHeightScale);
            break;
        }*/

        case IDC_GRASS_SEG_MASS_SLYDER:
        {
            g_fMass = (float)g_HUD.GetSlider( IDC_GRASS_SEG_MASS_SLYDER )->GetValue() / 1000.0f;
            swprintf_s( sStr, MAX_PATH, L"Segment Mass: %.3f", g_fMass );
            g_HUD.GetStatic( IDC_GRASS_SEG_MASS_LABEL )->SetText( sStr );
            //g_pSegMass->SetFloat(g_fMass);
            g_pGrassField->SetSegMass(g_fMass);
            break;
        }

        case IDC_GRASS_WIND_FORCE_SLYDER:
        {
            g_fWindStrength = (float)g_HUD.GetSlider( IDC_GRASS_WIND_FORCE_SLYDER )->GetValue() / 10000.0f;
            swprintf_s( sStr, MAX_PATH, L"Wind Strength: %.4f", g_fWindStrength);
            g_HUD.GetStatic( IDC_GRASS_WIND_LABEL )->SetText( sStr );
            g_pGrassField->SetWindStrength(g_fWindStrength);
            break;
        }

        case IDC_GRASS_WIND_SPEED_SLYDER:
        {
            g_fWindTexSpeed = (float)g_HUD.GetSlider( IDC_GRASS_WIND_SPEED_SLYDER )->GetValue() / 100.0f;                
            swprintf_s( sStr, MAX_PATH, L"WindTex Speed: %.2f", g_fWindTexSpeed);
            g_HUD.GetStatic( IDC_GRASS_WIND_SPEED_LABEL )->SetText( sStr );
            g_pGrassField->SetWindSpeed(g_fWindTexSpeed);
            break;
        }

        case IDC_GRASS_WIND_BIAS_SLYDER:
        {
            g_fWindBias = (float)g_HUD.GetSlider( IDC_GRASS_WIND_BIAS_SLYDER )->GetValue() / 1000.0f;                
            swprintf_s( sStr, MAX_PATH, L"Wind Bias: %.4f", g_fWindBias);
            g_HUD.GetStatic( IDC_GRASS_WIND_BIAS_LABEL )->SetText( sStr );
            g_pGrassField->SetWindBias(g_fWindBias);
            break;
        }

        case IDC_GRASS_WIND_SCALE_SLYDER:
        {                
            g_fWindScale = (float)g_HUD.GetSlider( IDC_GRASS_WIND_SCALE_SLYDER )->GetValue() / 100.0f;                
            swprintf_s( sStr, MAX_PATH, L"Wind Scale: %.4f", g_fWindScale);
            g_HUD.GetStatic( IDC_GRASS_WIND_SCALE_LABEL )->SetText( sStr );
            g_pGrassField->SetWindScale(g_fWindScale);
            break;
        }

        case IDC_GRASS_WIND_PHASE_SLYDER:
        {
            g_fWindTexTile = (float)g_HUD.GetSlider( IDC_GRASS_WIND_PHASE_SLYDER )->GetValue() / 10.0f;
            swprintf_s( sStr, MAX_PATH, L"WindTex Tile: %.2f", g_fWindTexTile );
            g_HUD.GetStatic( IDC_GRASS_WIND_PHASE_LABEL )->SetText( sStr );
            //g_pWindTexTile->SetFloat(g_fWindTexTile);
            g_pGrassField->SetWindMapTile(g_fWindTexTile);
            break;
        }

        case IDC_GRASS_SEG_HARD_1_SLYDER:
        {
            g_fHardness = (float)g_HUD.GetSlider( IDC_GRASS_SEG_HARD_1_SLYDER )->GetValue() / 100.0f;
            swprintf_s( sStr, MAX_PATH, L"Hardness: %.2f", g_fHardness );
            g_HUD.GetStatic( IDC_GRASS_SEG_HARD_LABEL )->SetText( sStr );
            //g_pHardnessMultiplyer->SetFloatVector((FLOAT *)(g_vHardnessMultiplyer));
            g_pGrassField->SetHardness(g_fHardness);
            break;
        }

        case IDC_LG_R_SLYDER:
        case IDC_LG_G_SLYDER:
        case IDC_LG_B_SLYDER:
        {
            g_vGrassSpecular.x = (float)g_HUD.GetSlider( IDC_LG_R_SLYDER )->GetValue() / 100.0f;
            g_vGrassSpecular.y = (float)g_HUD.GetSlider( IDC_LG_G_SLYDER )->GetValue() / 100.0f;
            g_vGrassSpecular.z = (float)g_HUD.GetSlider( IDC_LG_B_SLYDER )->GetValue() / 100.0f;
            swprintf_s( sStr, MAX_PATH, L"Specular: (%.2f,%.2f,%.2f)", g_vGrassSpecular.x, g_vGrassSpecular.y, g_vGrassSpecular.z );
            g_HUD.GetStatic( IDC_LG_RGB_LABEL )->SetText( sStr );
            g_pGrassField->SetLowGrassDiffuse(g_vGrassSpecular);
            break;
        }

        case IDC_TERR_R_SLYDER:
        case IDC_TERR_G_SLYDER:
        case IDC_TERR_B_SLYDER:
        {
            g_vTerrRGB.x = (float)g_HUD.GetSlider( IDC_TERR_R_SLYDER )->GetValue() / 100.0f;
            g_vTerrRGB.y = (float)g_HUD.GetSlider( IDC_TERR_G_SLYDER )->GetValue() / 100.0f;
            g_vTerrRGB.z = (float)g_HUD.GetSlider( IDC_TERR_B_SLYDER )->GetValue() / 100.0f;
            swprintf_s( sStr, MAX_PATH, L"Diffuse: (%.2f,%.2f,%.2f)", g_vTerrRGB.x, g_vTerrRGB.y, g_vTerrRGB.z );
            g_HUD.GetStatic( IDC_TERR_RGB_LABEL )->SetText( sStr );
            g_pGrassField->SetTerrRGB(g_vTerrRGB);
            break;
        }

        case IDC_FOG_R_SLYDER:
        case IDC_FOG_G_SLYDER:
        case IDC_FOG_B_SLYDER:
            {
                g_vFogColor.x = (float)g_HUD.GetSlider( IDC_FOG_R_SLYDER )->GetValue() / 100.0f;
                g_vFogColor.y = (float)g_HUD.GetSlider( IDC_FOG_G_SLYDER )->GetValue() / 100.0f;
                g_vFogColor.z = (float)g_HUD.GetSlider( IDC_FOG_B_SLYDER )->GetValue() / 100.0f;
                swprintf_s( sStr, MAX_PATH, L"Fog Color: (%.2f,%.2f,%.2f)", g_vFogColor.x, g_vFogColor.y, g_vFogColor.z );
                g_HUD.GetStatic( IDC_FOG_RGB_LABEL )->SetText( sStr );
                g_pGrassField->SetFogColor(g_vFogColor);
                break;
            }

        case IDC_TERR_TILE_SLYDER:
        {
            g_fTerrTile = (float)g_HUD.GetSlider( IDC_TERR_TILE_SLYDER )->GetValue() / 10.0f;
            swprintf_s( sStr, MAX_PATH, L"Terr Tile %.2f", g_fTerrTile );
            g_HUD.GetStatic( IDC_TERR_TILE_LABEL )->SetText( sStr );
            g_pTerrTile->SetFloat(g_fTerrTile);
            break;
        }

        case IDC_SAVE_BTN:
        {
            OutFile.open("config/colors.ini");
            OutFile << g_vFogColor.x      << " " << g_vFogColor.y      << " " << g_vFogColor.z      << '\n';
            OutFile << g_vTerrRGB.x       << " " << g_vTerrRGB.y       << " " << g_vTerrRGB.z       << '\n';
            OutFile << g_vGrassSpecular.x << " " << g_vGrassSpecular.y << " " << g_vGrassSpecular.z << '\n';
            OutFile.close();
            break;
        }
            

        case IDC_SAMPLE_COUNT:
        {
            CDXUTComboBox* pComboBox = ( CDXUTComboBox* )pControl;

            g_MSAASampleCount = ( UINT )PtrToInt( pComboBox->GetSelectedData() );

            HRESULT hr = S_OK;
            ID3D10Device* pd3dDevice = DXUTGetD3D10Device();
            if( pd3dDevice )
                V( CreateRenderTarget( pd3dDevice, g_BackBufferWidth, g_BackBufferHeight, g_MSAASampleCount, 0 ) );

            break;
        }

        case IDC_CAMERA_TYPE:
        {
            CDXUTComboBox* pComboBox = ( CDXUTComboBox* )pControl;
            UINT value;

            value = ( UINT )PtrToInt( pComboBox->GetSelectedData() );
			CameraType = value;

            D3DXVECTOR3 vEyeStart(9.8f, 9.5f, 7.8f);
            D3DXVECTOR3 vAtStart(10.8f, 9.2f, 8.8f);
            float height_scale, grass_radius;
            Terrain * const terr = g_pGrassField->GetTerrain(&height_scale, &grass_radius);

            switch (value)
            {
		     case CAMERA_NORMAL:
                if (g_Camera != NULL)
                    delete g_Camera;

                g_Camera = new CFirstPersonCamera();

                for (UINT i = 0; i < g_fNumOfMeshes; i++)
                    delete g_pMeshes[i];
                g_fNumOfMeshes = 0;
                break;
            case CAMERA_TERRAIN:
                if (g_Camera != NULL)
                    delete g_Camera;

                g_Camera = new HeightCamera(g_fCameraHeight, terr, height_scale, grass_radius);

                for (UINT i = 0; i < g_fNumOfMeshes; i++)
                    delete g_pMeshes[i];
                g_fNumOfMeshes = 0;
                break;
			case CAMERA_MESH:
                if (g_Camera != NULL)
                    delete g_Camera;

                if (g_fNumOfMeshes == 0)
                {
                    ID3D10Device* pd3dDevice = DXUTGetD3D10Device();
                    float height_scale, grass_radius;
                    Terrain * const terr = g_pGrassField->GetTerrain(&height_scale, &grass_radius);

                    g_pMeshes[0] = new Car(pd3dDevice, g_pGrassField->SceneEffect(),
                        D3DXVECTOR4(0, 0, 0, 0),
                        terr, height_scale, grass_radius,
                        g_fCarFrontWidth, g_fCarHeight, g_fCarLength, 0.0f);
//                        g_fCarFrontWidth, g_fCarHeight, g_fCarLength, 0.05f);
                    g_fNumOfMeshes = 1;
                }

                g_Camera = new MeshCamera(g_fCameraMeshDist, g_fCameraHeight+1.5,
                    g_pMeshes[0], terr, height_scale, grass_radius);                
                break;
            }
            g_Camera->SetProjParams( 60.4f * ( 3.14159f / 180.0f ), 1.33f, 0.1f, 1000.0f /*g_fMeter*/ );
            g_Camera->SetViewParams( &vEyeStart, &vAtStart );
            g_Camera->SetScalers(0.01f, g_fCameraSpeed);

            break;
        }
	}
}


//--------------------------------------------------------------------------------------
// Reject any D3D10 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D10DeviceAcceptable( UINT Adapter, UINT Output, D3D10_DRIVER_TYPE DeviceType,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D10 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D10CreateDevice( ID3D10Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                      void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D10CreateDevice( pd3dDevice ) );
    V_RETURN( g_D3DSettingsDlg.OnD3D10CreateDevice( pd3dDevice ) );
    V_RETURN( D3DX10CreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                                L"Arial", &g_pFont10 ) );
    V_RETURN( D3DX10CreateSprite( pd3dDevice, 512, &g_pSprite10 ) );
    g_pTxtHelper = new CDXUTTextHelper( NULL, NULL, g_pFont10, g_pSprite10, 15 );

    // Set us up for multisampling
    D3D10_RASTERIZER_DESC CurrentRasterizerState;
    CurrentRasterizerState.FillMode = D3D10_FILL_SOLID;
    CurrentRasterizerState.CullMode = D3D10_CULL_NONE;
    CurrentRasterizerState.FrontCounterClockwise = true;
    CurrentRasterizerState.DepthBias             = false;
    CurrentRasterizerState.DepthBiasClamp        = 0;
    CurrentRasterizerState.SlopeScaledDepthBias  = 0;
    CurrentRasterizerState.DepthClipEnable       = true;
    CurrentRasterizerState.ScissorEnable         = false;
    CurrentRasterizerState.MultisampleEnable     = true;
    CurrentRasterizerState.AntialiasedLineEnable = false;
    V_RETURN( pd3dDevice->CreateRasterizerState( &CurrentRasterizerState, &g_pRasterState ) );
    pd3dDevice->RSSetState( g_pRasterState );

    g_GrassInitState.InitState[0].fMaxQuality          = 0.0f;//0.7f;
    g_GrassInitState.InitState[0].dwBladesPerPatchSide = 20;
    g_GrassInitState.InitState[0].dwPatchesPerSide     = 37;//40;//43;//45;//32;//50;
    g_GrassInitState.InitState[0].fMostDetailedDist    = 2.0f;//* g_fMeter;
    g_GrassInitState.InitState[0].fLastDetailedDist    = 140.0;//85;//150.0f;// * g_fMeter;
    g_GrassInitState.InitState[0].fGrassRadius         = 140.0;//85;//150.0f;// * g_fMeter;
    g_GrassInitState.InitState[0].pD3DDevice           = pd3dDevice;
    
    
    g_GrassInitState.InitState[0].uNumCollidedPatchesPerMesh = 10;
    g_GrassInitState.InitState[0].uMaxColliders              = MAX_NUM_MESHES;
    
    g_GrassInitState.InitState[1]                            = g_GrassInitState.InitState[0];
    g_GrassInitState.InitState[2]                            = g_GrassInitState.InitState[0];
    //some differences...
    g_GrassInitState.InitState[0].sLowGrassTexPath           = L"";//L"resources/LowGrass.dds";
    g_GrassInitState.InitState[0].sIndexTexPath              = L"resources/IndexType1.dds";
    g_GrassInitState.InitState[1].sIndexTexPath              = L"resources/IndexType2.dds";
    g_GrassInitState.InitState[2].sIndexTexPath              = L"resources/IndexType3.dds";
    //g_GrassInitState.InitState[1].dwBladesPerPatchSide       = 2;
    g_GrassInitState.InitState[2].dwBladesPerPatchSide       = 2;
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
    g_GrassInitState.InitState[0].sEffectPath     = L"Shaders/GrassType1.fx";
    g_GrassInitState.InitState[1].sEffectPath     = L"Shaders/GrassType2.fx";
    g_GrassInitState.InitState[2].sEffectPath     = L"Shaders/GrassType3.fx";
    g_GrassInitState.InitState[0].sSeatingTexPath = L"resources/SeatingType1.dds";
    g_GrassInitState.InitState[1].sSeatingTexPath = L"resources/SeatingType2.dds";
    g_GrassInitState.InitState[2].sSeatingTexPath = L"resources/SeatingType3.dds";

    g_GrassInitState.InitState[0].sSubTypesPath   = L"config/T1SubTypes.cfg";
    g_GrassInitState.InitState[1].sSubTypesPath   = L"config/T2SubTypes.cfg";
    g_GrassInitState.InitState[2].sSubTypesPath   = L"config/T3SubTypes.cfg";
    g_GrassInitState.sSceneEffectPath             = L"Shaders/SceneEffect.fx";
    g_GrassInitState.sNoiseMapPath                = L"resources/Noise.dds";
    g_GrassInitState.sColorMapPath                = L"resources/GrassColor.dds";
    g_GrassInitState.fHeightScale                 = g_fHeightScale;
    g_GrassInitState.fTerrRadius                  = 400.0f;
    g_pGrassField                                 = new GrassFieldManager(g_GrassInitState);    
    g_pTerrTile                                   = g_pGrassField->SceneEffect()->GetVariableByName( "g_fTerrTile" )->AsScalar();
	g_pSkyBoxESRV								  = g_pGrassField->SceneEffect()->GetVariableByName( "g_txSkyBox" )->AsShaderResource();
	g_pRenderSkybox								  = g_pGrassField->SceneEffect()->GetTechniqueByName( "RenderSkyBox" );
    g_pSkyViewProjEMV                             = g_pGrassField->SceneEffect()->GetVariableByName( "g_mViewProj" )->AsMatrix();
	// Define our scene vertex layout
	const D3D10_INPUT_ELEMENT_DESC SkyBoxLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	};
	int iNumElements = sizeof( SkyBoxLayout ) / sizeof( D3D10_INPUT_ELEMENT_DESC );
	D3D10_PASS_DESC PassDesc;
	ID3D10EffectPass* pPass;
	pPass = g_pRenderSkybox->GetPassByIndex( 0 );
	pPass->GetDesc( &PassDesc );
	V_RETURN( pd3dDevice->CreateInputLayout( SkyBoxLayout, iNumElements, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &g_pSkyVertexLayout ) );
	g_MeshSkybox.Create( pd3dDevice, L"resources\\skysphere.sdkmesh", true );
    /*Loading colors*/
    std::ifstream InFile;
    InFile.open("config/colors.ini");
    InFile >> g_vFogColor.x      >>  g_vFogColor.y      >>  g_vFogColor.z     ;
    InFile >> g_vTerrRGB.x       >>  g_vTerrRGB.y       >>  g_vTerrRGB.z      ;
    InFile >> g_vGrassSpecular.x >>  g_vGrassSpecular.y >>  g_vGrassSpecular.z;
    InFile.close();
    /****************/
    g_pGrassField->SetTerrRGB(g_vTerrRGB);
    g_pTerrTile->SetFloat(g_fTerrTile);
    g_pGrassField->SetWindMapTile(g_fWindTexTile);
    g_pGrassField->SetSegMass(g_fMass);
    g_pGrassField->SetHardness(g_fHardness);
    g_pGrassField->SetGrassLodBias(g_fGrassLodBias);            
    g_pGrassField->SetSubScatterGamma(g_fGrassSubScatterGamma);
    g_pGrassField->SetGrassAmbient(g_fGrassAmbient);
    g_pGrassField->SetWindStrength(g_fWindStrength);
    g_pGrassField->SetWindSpeed(g_fWindTexSpeed);
    //g_pGrassField->SetHeightScale(g_fHeightScale);
    g_pGrassField->SetQuality(g_fQuality);
    g_pGrassField->SetLowGrassDiffuse(g_vGrassSpecular);
    g_pGrassField->SetFogColor(g_vFogColor);
    g_pGrassField->SetWindBias(g_fWindBias);
    g_pGrassField->SetWindScale(g_fWindScale);
    InitMeshes(pd3dDevice);
	

    // Setup the camera's view parameters
	g_Camera = new CFirstPersonCamera();
	D3DXVECTOR3 vEyeStart( 9.8f, 9.5f, 7.8f);
    D3DXVECTOR3 vAtStart( 10.8f, 9.0f, 8.8f);
/*            
	D3DXVECTOR3 vEyeStart(9.8f, 9.5f, 7.8f);
    D3DXVECTOR3 vAtStart(10.3f, 9.3f, 8.8f);
	float height_scale, grass_radius; 
	Terrain * const terr = g_pGrassField->GetTerrain(&height_scale, &grass_radius);
    g_Camera = new HeightCamera(g_fCameraHeight+0.5, terr, height_scale, grass_radius);
 */
	//vAtStart = *(D3DXVECTOR3*)(float*)(&g_pMeshes[0]->GetPosAndRadius());
    g_Camera->SetViewParams( &vEyeStart, &vAtStart );
    g_Camera->SetScalers(0.01f, g_fCameraSpeed /* g_fMeter*/);

    return S_OK;
}

//
//static float4 pos(16.0f,  0.0f, 8.0f, 0.0f);
static float4 pos(18.0f,  0.0f, 8.0f, 0.0f);
static float3 vCarDir = float3(0.0, 0.0, 1.0);
static float fFi = 0.0, fTimeEmul = 0.0;
static float fTimeRot[5] = {3.0, 4.0, 10.0, 11.0, 1000.0};
//static float fF[5] = {0.0, 0.0001, 0.0, -0.0001, 0.0}, fw = 0.0;
static float fF[5] = {0.0, 0.0, 0.0, 0.0, 0.0}, fw = 0.0;
static int Indx = 0;

void UpdateMeshes(float a_fElapsedTime)
{
//	float4 pos(16.0f,  0.0f, 8.0f, 0.0f);
//    float4 pos(-65.0f,  0.0f, 109.0f, 0.0f);
	if (CameraType == CAMERA_MESH)
	{
		if (fTimeEmul > fTimeRot[Indx]) Indx++; 
//		fFi = (float)M_PI * sinf(fTimeEmul);
		fw += fF[Indx];
		fFi += fw;
		fTimeEmul += 0.02;
		vCarDir = float3(0.0, 0.0, 1.0);
		for (UINT i = 0; i < g_fNumOfMeshes; i++)
		{
	//        pos.x += sinf(g_fTime*0.1f) * 15.0f;
	//        pos.z += g_fTime*2.0f;
	//        pos.z += a_fElapsedTime*2.0f;
	//        pos.z += 0.05*1.0f;
	//        pos.x += sinf(g_fTime*0.1f) * 20.0f;
	//        pos.z -= cosf(g_fTime*0.1f) * 20.0f;
			D3DXMATRIX mRot;
			D3DXMatrixRotationY(&mRot, fFi);
			D3DXVec3TransformCoord(&vCarDir, &vCarDir, &mRot);
			D3DXVec3Normalize(&vCarDir, &vCarDir);
			pos.x += vCarDir.x * 0.101f;
			pos.z += vCarDir.z * 0.101f;
			g_pMeshes[i]->SetPosAndRadius(pos);
		}
    }
}

//--------------------------------------------------------------------------------------
// Update the MSAA sample count combo box for this format
//--------------------------------------------------------------------------------------
void UpdateMSAASampleCounts( ID3D10Device* pd3dDevice, DXGI_FORMAT fmt )
{
    CDXUTComboBox* pComboBox = NULL;
    bool bResetSampleCount = false;
    UINT iHighestSampleCount = 0;

    pComboBox = g_HUD.GetComboBox( IDC_SAMPLE_COUNT );
    if( !pComboBox )
        return;

    pComboBox->RemoveAllItems();

    WCHAR val[10];
    for( UINT i = 1; i <= D3D10_MAX_MULTISAMPLE_SAMPLE_COUNT; i++)
    {
        UINT Quality;
        if( SUCCEEDED( pd3dDevice->CheckMultisampleQualityLevels( fmt, i, &Quality ) ) &&
            Quality > 0 )
        {
            swprintf_s( val, 10, L"%d", i );
            pComboBox->AddItem( val, IntToPtr( i ) );
            iHighestSampleCount = i;
        }
        else if( g_MSAASampleCount == i )
        {
            bResetSampleCount = true;
        }
    }

    if( bResetSampleCount )
        g_MSAASampleCount = iHighestSampleCount;

    pComboBox->SetSelectedByData( IntToPtr( g_MSAASampleCount ) );

}

//--------------------------------------------------------------------------------------
// Create any D3D10 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D10SwapChainResized( ID3D10Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr = S_OK;
    V_RETURN( g_DialogResourceManager.OnD3D10ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );
    V_RETURN( g_D3DSettingsDlg.OnD3D10ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
    g_Camera->SetProjParams( 70.4f * ( 3.14159f / 180.0f ), fAspectRatio, 0.1f, 1000.0f /*g_fMeter*/ );

    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
    g_HUD.SetSize( 170, 170 );
    g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 300 );
    g_SampleUI.SetSize( 170, 300 );

    // Update the sample count
    UpdateMSAASampleCounts( pd3dDevice, pBackBufferSurfaceDesc->Format );

    // Create a multi-sample render target
    g_BackBufferWidth = pBackBufferSurfaceDesc->Width;
    g_BackBufferHeight = pBackBufferSurfaceDesc->Height;
    V_RETURN( CreateRenderTarget( pd3dDevice, g_BackBufferWidth, g_BackBufferHeight, g_MSAASampleCount, 0 ) );

    return S_OK;
}

void RenderGrass( ID3D10Device* pd3dDevice, D3DXMATRIXA16& mView, D3DXMATRIXA16& mProj, float a_fElapsedTime )
{    
    D3DXMATRIXA16 mViewProj;
    D3DXMatrixMultiply( &mViewProj, &mView, &mProj );
    g_pGrassField->SetTime(g_fTime);
    g_pGrassField->SetViewProjMtx(mViewProj);
    g_pGrassField->SetViewMtx(mView);
    g_pGrassField->SetProjMtx(mProj);
    
    // Draw Mesh
    UpdateMeshes(a_fElapsedTime);
    
    // Draw Grass
    D3DXVECTOR3 vCamDir = *g_Camera->GetLookAtPt() - *g_Camera->GetEyePt();
    g_pGrassField->Update(vCamDir, *g_Camera->GetEyePt(), g_pMeshes, g_fNumOfMeshes, a_fElapsedTime);
    g_pGrassField->Render();
    WCHAR sStr[MAX_PATH] = {0};
    swprintf_s( sStr, MAX_PATH, L"Camera at: (%.1f, %.1f, %.1f)", g_Camera->GetEyePt()->x, g_Camera->GetEyePt()->y, g_Camera->GetEyePt()->z );
    g_HUD.GetStatic( IDC_CAM_POS_LABEL )->SetText(sStr);

    UINT i;
    for (i = 0; i < g_fNumOfMeshes; i++)
    {
        g_pMeshes[i]->Render();
    }
	pd3dDevice->IASetInputLayout(g_pSkyVertexLayout);
    g_pSkyViewProjEMV->SetMatrix((float*)&mViewProj);
	g_MeshSkybox.Render( pd3dDevice, g_pRenderSkybox, g_pSkyBoxESRV );
}

//--------------------------------------------------------------------------------------
void CALLBACK OnD3D10FrameRender( ID3D10Device* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    // If the settings dialog is being shown, then render it instead of rendering the app's scene
    if( g_D3DSettingsDlg.IsActive() )
    {
        g_D3DSettingsDlg.OnRender( fElapsedTime );
        return;
    }

    float ClearColor[4] = { 0.0, 0.3f, 0.8f, 0.0 };
    ID3D10DepthStencilView* pDSV = DXUTGetD3D10DepthStencilView();
    pd3dDevice->ClearDepthStencilView( pDSV, D3D10_CLEAR_DEPTH, 1.0, 0 );

    // Set our render target since we can't present multisampled ref
    ID3D10RenderTargetView* pOrigRT;
    ID3D10DepthStencilView* pOrigDS;
    pd3dDevice->OMGetRenderTargets( 1, &pOrigRT, &pOrigDS );

    ID3D10RenderTargetView* aRTViews[ 1 ] = { g_pRTRV };
    pd3dDevice->OMSetRenderTargets( 1, aRTViews, g_pDSRV );

    // Clear the render target and DSV
    pd3dDevice->ClearRenderTargetView( g_pRTRV, ClearColor );
    pd3dDevice->ClearDepthStencilView( g_pDSRV, D3D10_CLEAR_DEPTH, 1.0, 0 );

    g_fTime += fElapsedTime;
    D3DXMATRIXA16 mView;
    D3DXMATRIXA16 mProj;
    mProj = *g_Camera->GetProjMatrix();
    mView = *g_Camera->GetViewMatrix();

    RenderGrass( pd3dDevice, mView, mProj, fElapsedTime ); // Render grass

    // Copy it over because we can't resolve on present at the moment
    ID3D10Resource* pRT;
    pOrigRT->GetResource( &pRT );
    D3D10_RENDER_TARGET_VIEW_DESC rtDesc;
    pOrigRT->GetDesc( &rtDesc );
    pd3dDevice->ResolveSubresource( pRT, D3D10CalcSubresource( 0, 0, 1 ), g_pRenderTarget, D3D10CalcSubresource( 0, 0,
                                                                                                                 1 ),
                                    rtDesc.Format );
    SAFE_RELEASE( pRT );

    // Use our Old RT again
    aRTViews[0] = pOrigRT;
    pd3dDevice->OMSetRenderTargets( 1, aRTViews, pOrigDS );
    SAFE_RELEASE( pOrigRT );
    SAFE_RELEASE( pOrigDS );

    DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"HUD / Stats" );
    g_HUD.OnRender( fElapsedTime );
    g_SampleUI.OnRender( fElapsedTime );
    RenderText();
    DXUT_EndPerfEvent();

}

//--------------------------------------------------------------------------------------
// Render the help and statistics text
//--------------------------------------------------------------------------------------
void RenderText()
{
    g_pTxtHelper->Begin();
    g_pTxtHelper->SetInsertionPos( 2, 0 );
    g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    g_pTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
    g_pTxtHelper->DrawTextLine( DXUTGetDeviceStats() );
//  g_pTxtHelper->DrawTextLine((WCHAR)"d-inter");
    g_pTxtHelper->End();
}

//--------------------------------------------------------------------------------------
// Release D3D10 resources created in OnD3D10ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D10SwapChainReleasing( void* pUserContext )
{
    g_DialogResourceManager.OnD3D10ReleasingSwapChain();

    SAFE_RELEASE( g_pRTRV );
    SAFE_RELEASE( g_pDSRV );
    SAFE_RELEASE( g_pDSTarget );
}


//--------------------------------------------------------------------------------------
// Release D3D10 resources created in OnD3D10CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D10DestroyDevice( void* pUserContext )
{
    delete g_pGrassField;
    for (UINT i = 0; i < g_fNumOfMeshes; i++)
        delete g_pMeshes[i];
    g_DialogResourceManager.OnD3D10DestroyDevice();
    g_D3DSettingsDlg.OnD3D10DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();    
    SAFE_RELEASE( g_pFont10 );
    SAFE_RELEASE( g_pSprite10 );
    SAFE_DELETE( g_pTxtHelper );

    SAFE_RELEASE( g_pRenderTarget );
    SAFE_RELEASE( g_pRTRV );
    SAFE_RELEASE( g_pDSTarget );
    SAFE_RELEASE( g_pDSRV );
    SAFE_RELEASE( g_pRasterState );
	SAFE_RELEASE( g_pSkyVertexLayout );
	g_MeshSkybox.Destroy();
}

HRESULT CreateRenderTarget( ID3D10Device* pd3dDevice, UINT uiWidth, UINT uiHeight, UINT uiSampleCount,
                            UINT uiSampleQuality )
{
    HRESULT hr = S_OK;

    SAFE_RELEASE( g_pRenderTarget );
    SAFE_RELEASE( g_pRTRV );
    SAFE_RELEASE( g_pDSTarget );
    SAFE_RELEASE( g_pDSRV );

    ID3D10RenderTargetView* pOrigRT = NULL;
    ID3D10DepthStencilView* pOrigDS = NULL;
    pd3dDevice->OMGetRenderTargets( 1, &pOrigRT, &pOrigDS );

    D3D10_RENDER_TARGET_VIEW_DESC DescRTV;
    pOrigRT->GetDesc( &DescRTV );
    SAFE_RELEASE( pOrigRT );
    SAFE_RELEASE( pOrigDS );

    D3D10_TEXTURE2D_DESC dstex;
    dstex.Width = uiWidth;
    dstex.Height = uiHeight;
    dstex.MipLevels = 1;
    dstex.Format = DescRTV.Format;
    dstex.SampleDesc.Count = uiSampleCount;
    dstex.SampleDesc.Quality = uiSampleQuality;
    dstex.Usage = D3D10_USAGE_DEFAULT;
    dstex.BindFlags = D3D10_BIND_RENDER_TARGET;
    dstex.CPUAccessFlags = 0;
    dstex.MiscFlags = 0;
    dstex.ArraySize = 1;
    V_RETURN( pd3dDevice->CreateTexture2D( &dstex, NULL, &g_pRenderTarget ) );

    // Create the render target view
    D3D10_RENDER_TARGET_VIEW_DESC DescRT;
    DescRT.Format = dstex.Format;
    DescRT.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2DMS;
    V_RETURN( pd3dDevice->CreateRenderTargetView( g_pRenderTarget, &DescRT, &g_pRTRV ) );

    // Create depth stencil texture.
    dstex.Width = uiWidth;
    dstex.Height = uiHeight;
    dstex.MipLevels = 1;
    dstex.Format = DXGI_FORMAT_D32_FLOAT;
    dstex.SampleDesc.Count = uiSampleCount;
    dstex.SampleDesc.Quality = uiSampleQuality;
    dstex.Usage = D3D10_USAGE_DEFAULT;
    dstex.BindFlags = D3D10_BIND_DEPTH_STENCIL;
    dstex.CPUAccessFlags = 0;
    dstex.MiscFlags = 0;
    V_RETURN( pd3dDevice->CreateTexture2D( &dstex, NULL, &g_pDSTarget ) );

    // Create the depth stencil view
    D3D10_DEPTH_STENCIL_VIEW_DESC DescDS;
    DescDS.Format = DXGI_FORMAT_D32_FLOAT;
    DescDS.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2DMS;
    V_RETURN( pd3dDevice->CreateDepthStencilView( g_pDSTarget, &DescDS, &g_pDSRV ) );

    return hr;
}

