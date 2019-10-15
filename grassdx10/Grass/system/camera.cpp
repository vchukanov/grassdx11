#include "camera.h"

void LandscapeCamera::FrameMove( FLOAT fElapsedTime )
{
    if( DXUTGetGlobalTimer()->IsStopped() ) {
        if (DXUTGetFPS() == 0.0f) fElapsedTime = 0;
        else fElapsedTime = 1.0f / DXUTGetFPS();
    }

    if( IsKeyDown( m_aKeys[CAM_RESET] ) )
        Reset();

    // Get keyboard/mouse/gamepad input
    GetInput( m_bEnablePositionMovement, ( m_nActiveButtonMask & m_nCurrentButtonMask ) || m_bRotateWithoutButtonDown,
        true, m_bResetCursorAfterMove );

    //// Get the mouse movement (if any) if the mouse button are down
    //if( (m_nActiveButtonMask & m_nCurrentButtonMask) || m_bRotateWithoutButtonDown )
    //    UpdateMouseDelta( fElapsedTime );

    // Get amount of velocity based on the keyboard input and drag (if any)
    UpdateVelocity( fElapsedTime );

    // Simple euler method to calculate position delta
    D3DXVECTOR3 vPosDelta = m_vVelocity * fElapsedTime;

    // If rotating the camera 
    if( ( m_nActiveButtonMask & m_nCurrentButtonMask ) ||
        m_bRotateWithoutButtonDown ||
        m_vGamePadRightThumb.x != 0 ||
        m_vGamePadRightThumb.z != 0 )
    {
        // Update the pitch & yaw angle based on mouse movement
        float fYawDelta = m_vRotVelocity.x;
        float fPitchDelta = m_vRotVelocity.y;

        // Invert pitch if requested
        if( m_bInvertPitch )
            fPitchDelta = -fPitchDelta;

        m_fCameraPitchAngle += fPitchDelta;
        m_fCameraYawAngle += fYawDelta;

        // Limit pitch to straight up or straight down
        m_fCameraPitchAngle = __max( -D3DX_PI / 2.0f, m_fCameraPitchAngle );
        m_fCameraPitchAngle = __min( +D3DX_PI / 2.0f, m_fCameraPitchAngle );
    }

    // Make a rotation matrix based on the camera's yaw & pitch
    D3DXMATRIX mCameraRot;
    D3DXMatrixRotationYawPitchRoll( &mCameraRot, m_fCameraYawAngle, m_fCameraPitchAngle, 0 );

    // Transform vectors based on camera's rotation matrix
    D3DXVECTOR3 vWorldUp, vWorldAhead;
    D3DXVECTOR3 vLocalUp = D3DXVECTOR3( 0, 1, 0 );
    D3DXVECTOR3 vLocalAhead = D3DXVECTOR3( 0, 0, 1 );
    D3DXVec3TransformCoord( &vWorldUp, &vLocalUp, &mCameraRot );
    D3DXVec3TransformCoord( &vWorldAhead, &vLocalAhead, &mCameraRot );

    // Transform the position delta by the camera's rotation 
    D3DXVECTOR3 vPosDeltaWorld;
    if( !m_bEnableYAxisMovement )
    {
        // If restricting Y movement, do not include pitch
        // when transforming position delta vector.
        D3DXMatrixRotationYawPitchRoll( &mCameraRot, m_fCameraYawAngle, 0.0f, 0.0f );
    }
    D3DXVec3TransformCoord( &vPosDeltaWorld, &vPosDelta, &mCameraRot );

    // Move the eye position 
    m_vEye += vPosDeltaWorld;
    if( m_bClipToBoundary )
        ConstrainToBoundary( &m_vEye );

    // Correct camera position by landscape height
    TerrainHeightData *pHD = m_pTerrain->HeightDataPtr();
    float2 vTexCoord = float2(m_vEye.x / m_fGrassRadius * 0.5f + 0.5f, m_vEye.z / m_fGrassRadius * 0.5f + 0.5f);

    float terrain_height = pHD->GetHeight(vTexCoord.x, vTexCoord.y) * m_fHeightScale;
    if (m_vEye.y < terrain_height + 1)
        m_vEye.y = terrain_height + 1;

    // Update the lookAt position based on the eye position 
    m_vLookAt = m_vEye + vWorldAhead;

    // Update the view matrix
    D3DXMatrixLookAtLH( &m_mView, &m_vEye, &m_vLookAt, &vWorldUp );

    D3DXMatrixInverse( &m_mCameraWorld, NULL, &m_mView );
}

void HeightCamera::FrameMove( FLOAT fElapsedTime )
{
    if (DXUTGetGlobalTimer()->IsStopped())
    {
        if (DXUTGetFPS() == 0.0f)
            fElapsedTime = 0;
        else
            fElapsedTime = 1.0f / DXUTGetFPS();
    }

    if (IsKeyDown(m_aKeys[CAM_RESET]))
        Reset();

    // Change camera height
    if (IsKeyDown(m_aKeys[CAM_MOVE_UP]))
    {
        m_fDefaultHeight += 0.1f;
        if (m_fDefaultHeight > m_pMinMaxHeight.second)
            m_fDefaultHeight = m_pMinMaxHeight.second;
    }
    if (IsKeyDown(m_aKeys[CAM_MOVE_DOWN]))
    {
        m_fDefaultHeight -= 0.1f;
        if (m_fDefaultHeight < m_pMinMaxHeight.first)
            m_fDefaultHeight = m_pMinMaxHeight.first;
    }

    // Get keyboard/mouse/gamepad input
    GetInput(m_bEnablePositionMovement, (m_nActiveButtonMask & m_nCurrentButtonMask) || m_bRotateWithoutButtonDown,
        true, m_bResetCursorAfterMove);

    // Get amount of velocity based on the keyboard input and drag (if any)
    UpdateVelocity(fElapsedTime);

    // Simple euler method to calculate position delta
    D3DXVECTOR3 vPosDelta = m_vVelocity * fElapsedTime;

    // If rotating the camera 
    if((m_nActiveButtonMask & m_nCurrentButtonMask) ||
        m_bRotateWithoutButtonDown ||
        m_vGamePadRightThumb.x != 0 ||
        m_vGamePadRightThumb.z != 0 )
    {
        // Update the pitch & yaw angle based on mouse movement
        float fYawDelta = m_vRotVelocity.x;
        float fPitchDelta = m_vRotVelocity.y;

        // Invert pitch if requested
        if( m_bInvertPitch )
            fPitchDelta = -fPitchDelta;

        //m_fCameraPitchAngle += fPitchDelta;
        m_fCameraYawAngle += fYawDelta;

        // Limit pitch to straight up or straight down
        m_fCameraPitchAngle = __max( -D3DX_PI / 2.0f, m_fCameraPitchAngle );
        m_fCameraPitchAngle = __min( +D3DX_PI / 2.0f, m_fCameraPitchAngle );
    }

    // Make a rotation matrix based on the camera's yaw & pitch
    D3DXMATRIX mCameraRot;
    D3DXMatrixRotationYawPitchRoll( &mCameraRot, m_fCameraYawAngle, m_fCameraPitchAngle, 0 );

    // Transform vectors based on camera's rotation matrix
    D3DXVECTOR3 vWorldUp, vWorldAhead;
    D3DXVECTOR3 vLocalUp = D3DXVECTOR3( 0, 1, 0 );
    D3DXVECTOR3 vLocalAhead = D3DXVECTOR3( 0, 0, 1 );
    D3DXVec3TransformCoord( &vWorldUp, &vLocalUp, &mCameraRot );
    D3DXVec3TransformCoord( &vWorldAhead, &vLocalAhead, &mCameraRot );

    // Transform the position delta by the camera's rotation 
    D3DXVECTOR3 vPosDeltaWorld;
    if( !m_bEnableYAxisMovement )
    {
        // If restricting Y movement, do not include pitch
        // when transforming position delta vector.
        D3DXMatrixRotationYawPitchRoll( &mCameraRot, m_fCameraYawAngle, 0.0f, 0.0f );
    }
    D3DXVec3TransformCoord( &vPosDeltaWorld, &vPosDelta, &mCameraRot );

    // Move the eye position 
    m_vEye += vPosDeltaWorld;
    if( m_bClipToBoundary )
        ConstrainToBoundary( &m_vEye );

    // Correct camera position by landscape height
    TerrainHeightData *pHD = m_pTerrain->HeightDataPtr();
    float2 vTexCoord = float2(m_vEye.x / m_fGrassRadius * 0.5f + 0.5f, m_vEye.z / m_fGrassRadius * 0.5f + 0.5f);

    float terrain_height = pHD->GetHeight(vTexCoord.x, vTexCoord.y) * m_fHeightScale;
    m_vEye.y = m_fDefaultHeight + terrain_height;
    if (m_vEye.y < terrain_height + m_pMinMaxHeight.first)
        m_vEye.y = terrain_height + m_pMinMaxHeight.first;

    // Update the lookAt position based on the eye position 
    m_vLookAt = m_vEye + vWorldAhead;

    // Update the view matrix
    D3DXMatrixLookAtLH( &m_mView, &m_vEye, &m_vLookAt, &vWorldUp );

    D3DXMatrixInverse( &m_mCameraWorld, NULL, &m_mView );  
}

void MeshCamera::FrameMove( FLOAT fElapsedTime )
{
    if (DXUTGetGlobalTimer()->IsStopped())
    {
        if (DXUTGetFPS() == 0.0f)
            fElapsedTime = 0;
        else
            fElapsedTime = 1.0f / DXUTGetFPS();
    }

    if (IsKeyDown(m_aKeys[CAM_RESET]))
        Reset();

    // Change camera height
    if (IsKeyDown(m_aKeys[CAM_MOVE_UP]))
    {
        m_fDefaultHeight += 0.1f;
        if (m_fDefaultHeight > m_pMinMaxHeight.second)
            m_fDefaultHeight = m_pMinMaxHeight.second;
    }
    if (IsKeyDown(m_aKeys[CAM_MOVE_DOWN]))
    {
        m_fDefaultHeight -= 0.1f;
        if (m_fDefaultHeight < m_pMinMaxHeight.first)
            m_fDefaultHeight = m_pMinMaxHeight.first;
    }


    // Change mesh dist
    if (m_nMouseWheelDelta != m_nOldWheelMouseDelta)
    {
        m_fMeshDist -= 0.2f * ((m_nOldWheelMouseDelta - m_nMouseWheelDelta) / WHEEL_DELTA);
        if (m_fMeshDist < m_pMinMaxMeshDist.first)
            m_fMeshDist = m_pMinMaxMeshDist.first;
        if (m_fMeshDist > m_pMinMaxMeshDist.second)
            m_fMeshDist = m_pMinMaxMeshDist.second;
        m_nOldWheelMouseDelta = m_nMouseWheelDelta;
    }

    // Get position delta
    D3DXVECTOR4 mesh_pos_and_radius;
    D3DXVECTOR3 mesh_pos, mesh_dir;
    float mesh_radius;//, len;

    mesh_pos_and_radius = m_pMesh->GetPosAndRadius();
    mesh_pos = D3DXVECTOR3(mesh_pos_and_radius.x, mesh_pos_and_radius.y, mesh_pos_and_radius.z);
    mesh_radius = mesh_pos_and_radius.w;

    /*mesh_dir = mesh_pos - m_vEye;

    len = D3DXVec3Length(&mesh_dir);
    D3DXVec3Normalize(&mesh_dir, &mesh_dir);

    // Move the eye position 
    m_vEye += mesh_dir * (len - m_fMeshDist);
    if (m_bClipToBoundary)
    ConstrainToBoundary( &m_vEye );*/

    if (D3DXVec3Length(&m_vLookDir) < 0.00001f)
        m_vLookDir = m_pMesh->GetMoveDir();
    else
        m_vLookDir += m_pMesh->GetMoveDir() * 0.5f;
    D3DXVec3Normalize(&m_vLookDir, &m_vLookDir);
    m_vEye = mesh_pos - m_vLookDir * m_fMeshDist;//11.0f;
    // Correct camera position by landscape height
    TerrainHeightData *pHD = m_pTerrain->HeightDataPtr();
    float2 vTexCoord = float2(m_vEye.x / m_fGrassRadius * 0.5f + 0.5f, m_vEye.z / m_fGrassRadius * 0.5f + 0.5f);

    float terrain_height = pHD->GetHeight(vTexCoord.x, vTexCoord.y) * m_fHeightScale;
    //m_vEye.y = m_fMeshDist * 0.39f + terrain_height;
    m_vEye.y = max(m_fDefaultHeight + terrain_height, mesh_pos.y + 0.5f);
    if (m_vEye.y < terrain_height + m_pMinMaxHeight.first)
        m_vEye.y = terrain_height + m_pMinMaxHeight.first;

    m_vLookAt = mesh_pos;
    m_vLookAt.y += 1.2f;

    D3DXMatrixLookAtLH(&m_mView, &m_vEye, &m_vLookAt, &D3DXVECTOR3(0, 1, 0));

    D3DXMatrixInverse(&m_mCameraWorld, NULL, &m_mView);
}

float MeshCamera::GetMeshDist( void )
{
    return m_fMeshDist;
}
