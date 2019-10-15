//--------------------------------------------------------------------------------------
// File: MeshLoader10.cpp
//
// Wrapper class for ID3DX10Mesh interface. Handles loading mesh data from an .obj file
// and resource management for material textures.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "SDKmisc.h"
#pragma warning(disable: 4995)
#include "meshloader10.h"
#include <fstream>
#include <vector>
using namespace std;
#pragma warning(default: 4995)

template<typename TYPE> BOOL IsErrorResource( TYPE data )
{
    if( ( TYPE )ERROR_RESOURCE_VALUE == data )
        return TRUE;
    return FALSE;
}

// Define the input layout
const D3D10_INPUT_ELEMENT_DESC layout_CMeshLoader10[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D10_INPUT_PER_VERTEX_DATA, 0 },
} ;
UINT numElements_layout_CMeshLoader10 = sizeof( layout_CMeshLoader10 ) / sizeof( layout_CMeshLoader10[0] );


//--------------------------------------------------------------------------------------
CMeshLoader10::CMeshLoader10()
{
    m_pd3dDevice = NULL;
    m_pMesh = NULL;

    m_NumAttribTableEntries = 0;
    m_pAttribTable = NULL;

    ZeroMemory( m_strMediaDir, sizeof( m_strMediaDir ) );
}


//--------------------------------------------------------------------------------------
CMeshLoader10::~CMeshLoader10()
{
    Destroy();
}


//--------------------------------------------------------------------------------------
void CMeshLoader10::Destroy()
{
    for ( int iMaterial = 0; iMaterial < m_Materials.GetSize(); ++iMaterial )
    {
        Material *pMaterial = m_Materials.GetAt( iMaterial );

        if ( pMaterial->pMapKdRV10 && !IsErrorResource(pMaterial->pMapKdRV10) )
        {
            ID3D10Resource* pRes = NULL;

            pMaterial->pMapKdRV10->GetResource( &pRes );
            SAFE_RELEASE( pRes );
            SAFE_RELEASE( pRes );   // do this twice, because GetResource adds a ref

            SAFE_RELEASE( pMaterial->pMapKdRV10 );
        }
        if ( pMaterial->pMapKsRV10 && !IsErrorResource(pMaterial->pMapKsRV10) )
        {
            ID3D10Resource* pRes = NULL;

            pMaterial->pMapKsRV10->GetResource( &pRes );
            SAFE_RELEASE( pRes );
            SAFE_RELEASE( pRes );   // do this twice, because GetResource adds a ref

            SAFE_RELEASE( pMaterial->pMapKsRV10 );
        }

        SAFE_DELETE( pMaterial );
    }

    m_Materials.RemoveAll();
    m_Vertices.RemoveAll();
    m_Indices.RemoveAll();
    m_Attributes.RemoveAll();

    SAFE_DELETE_ARRAY( m_pAttribTable );
    m_NumAttribTableEntries = 0;

    SAFE_RELEASE( m_pMesh );
    m_pd3dDevice = NULL;
    SAFE_RELEASE(m_pVbuf);
    SAFE_RELEASE(m_pIbuf);
}


//--------------------------------------------------------------------------------------
HRESULT CMeshLoader10::Create( ID3D10Device* pd3dDevice, const WCHAR* strFilename )
{
    HRESULT hr;
    WCHAR str[ MAX_PATH ] = {0};

    m_pIbuf = NULL;
    m_pVbuf = NULL;
    // Start clean
    Destroy();

    // Store the device pointer
    m_pd3dDevice = pd3dDevice;
    

    // Load the vertex buffer, index buffer, and subset information from a file. In this case, 
    // an .obj file was chosen for simplicity, but it's meant to illustrate that ID3DXMesh objects
    // can be filled from any mesh file format once the necessary data is extracted from file.
    V_RETURN( LoadGeometryFromOBJ( strFilename ) );

    // Set the current directory based on where the mesh was found
    WCHAR wstrOldDir[MAX_PATH] = {0};
    GetCurrentDirectory( MAX_PATH, wstrOldDir );
    SetCurrentDirectory( m_strMediaDir );    

    // Load material textures
    for ( int iMaterial = 0; iMaterial < m_Materials.GetSize(); ++iMaterial )
    {
        Material *pMaterial = m_Materials.GetAt( iMaterial );

        if ( pMaterial->strMapKd[0] )
        {            
            pMaterial->pMapKdRV10 = (ID3D10ShaderResourceView*)ERROR_RESOURCE_VALUE;

            if ( SUCCEEDED(DXUTFindDXSDKMediaFileCch( str, MAX_PATH, pMaterial->strMapKd) ) )
            {
                DXUTGetGlobalResourceCache().CreateTextureFromFile( pd3dDevice, str, 
                    &pMaterial->pMapKdRV10, false ) ;
            }
        }
        if ( pMaterial->strMapKs[0] )
        {
            pMaterial->pMapKsRV10 = (ID3D10ShaderResourceView*)ERROR_RESOURCE_VALUE;

            if ( SUCCEEDED(DXUTFindDXSDKMediaFileCch( str, MAX_PATH, pMaterial->strMapKs) ) )
            {
                DXUTGetGlobalResourceCache().CreateTextureFromFile( pd3dDevice, str, 
                    &pMaterial->pMapKsRV10, false ) ;
            }
        }
    }

    // Restore the original current directory
    SetCurrentDirectory( wstrOldDir );

    // Create the encapsulated mesh
    // Create the encapsulated mesh
    ID3DX10Mesh *pMesh = NULL;

    V_RETURN( D3DX10CreateMesh( pd3dDevice,
        layout_CMeshLoader10,
        numElements_layout_CMeshLoader10,
        layout_CMeshLoader10[0].SemanticName,
        m_Vertices.GetSize(),
        m_Indices.GetSize() / 3,
        D3DX10_MESH_32_BIT,
        &pMesh ) );

    // Set the vertex data
    pMesh->SetVertexData( 0, (void*)m_Vertices.GetData() );
    m_Vertices.RemoveAll();

    // Set the index data
    pMesh->SetIndexData( (void*)m_Indices.GetData(), m_Indices.GetSize() );
    m_Indices.RemoveAll();

    // Set the attribute data
    //UINT u = 1;
    //pMesh->SetAttributeData( &u );
    pMesh->SetAttributeData( (UINT*)m_Attributes.GetData() );
    m_Attributes.RemoveAll();

    // Reorder the vertices according to subset and optimize the mesh for this graphics 
    // card's vertex cache. When rendering the mesh's triangle list the vertices will 
    // cache hit more often so it won't have to re-execute the vertex shader.
    V( pMesh->GenerateAdjacencyAndPointReps( 1e-6f ) );
    V( pMesh->Optimize( D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, NULL, NULL ) );

    pMesh->GetAttributeTable( NULL, &m_NumAttribTableEntries );
    m_pAttribTable = new D3DX10_ATTRIBUTE_RANGE[m_NumAttribTableEntries];
    for (UINT i = 0; i < m_NumAttribTableEntries; i++)
        m_pAttribTable[i].AttribId = 1;
    pMesh->GetAttributeTable( m_pAttribTable, &m_NumAttribTableEntries );

    V( pMesh->CommitToDevice() );

    m_pMesh = pMesh;

    /*
    m_pMesh = NULL;
    D3D10_BUFFER_DESC BufferDesc = 
    {
        m_Vertices.GetSize() * sizeof(VERTEX),
        D3D10_USAGE_DEFAULT,
        D3D10_BIND_VERTEX_BUFFER,
        0, 0
    };
    D3D10_SUBRESOURCE_DATA BufferInitData;
    BufferInitData.pSysMem = m_Vertices.GetData();
    BufferInitData.SysMemPitch = 0;
    BufferInitData.SysMemSlicePitch = 0;
    pd3dDevice->CreateBuffer(&BufferDesc, &BufferInitData, &m_pVbuf);
    BufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
    BufferDesc.ByteWidth = m_Indices.GetSize() * sizeof(UINT);
    BufferInitData.pSysMem = m_Indices.GetData();
    pd3dDevice->CreateBuffer(&BufferDesc, &BufferInitData, &m_pIbuf);
    */

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CMeshLoader10::LoadGeometryFromOBJ( const WCHAR* strFileName )
{
    WCHAR strMaterialFilename[MAX_PATH] = {0};
    WCHAR wstr[MAX_PATH];
    char str[MAX_PATH];
    HRESULT hr;

    // Find the file
    V_RETURN( DXUTFindDXSDKMediaFileCch( wstr, MAX_PATH, strFileName ) );
    WideCharToMultiByte( CP_ACP, 0, wstr, -1, str, MAX_PATH, NULL, NULL );

    // Store the directory where the mesh was found
    wcscpy_s( m_strMediaDir, MAX_PATH - 1, wstr );
    WCHAR* pch = wcsrchr( m_strMediaDir, L'\\' );
    if( pch )
        *pch = NULL;

    // Create temporary storage for the input data. Once the data has been loaded into
    // a reasonable format we can create a D3DXMesh object and load it with the mesh data.
    CGrowableArray <D3DXVECTOR3> Positions;
    CGrowableArray <D3DXVECTOR2> TexCoords;
    CGrowableArray <D3DXVECTOR3> Normals;

    // The first subset uses the default material
    Material* pMaterial = new Material();
    if( pMaterial == NULL )
        return E_OUTOFMEMORY;

    InitMaterial( pMaterial );
    wcscpy_s( pMaterial->strName, MAX_PATH - 1, L"default" );
    m_Materials.Add( pMaterial );

    DWORD dwCurSubset = 0;

    // File input
    WCHAR strCommand[256] = {0};
    std::vector<VERTEX> FaceVertices;
    std::vector<UINT>   FaceVerticesInd;
    std::vector<UINT>   FaceIndices;
    wifstream InFile( str );
    if( !InFile )
        return DXTRACE_ERR( L"wifstream::open", E_FAIL );

    for(; ; )
    {
        InFile >> strCommand;
        if( !InFile )
            break;

        if( 0 == wcscmp( strCommand, L"#" ) )
        {
            // Comment
        }
        else if( 0 == wcscmp( strCommand, L"v" ) )
        {
            // Vertex Position
            float x, y, z;
            InFile >> x >> y >> z;
            Positions.Add( D3DXVECTOR3( x, y, z ) );
        }
        else if( 0 == wcscmp( strCommand, L"vt" ) )
        {
            // Vertex TexCoord
            float u, v;
            InFile >> u >> v;
            TexCoords.Add( D3DXVECTOR2( u, v ) );
        }
        else if( 0 == wcscmp( strCommand, L"vn" ) )
        {
            // Vertex Normal
            float x, y, z;
            InFile >> x >> y >> z;
            Normals.Add( D3DXVECTOR3( x, y, z ) );
        }
        else if( 0 == wcscmp( strCommand, L"f" ) )
        {
            // Face
            UINT iPosition, iTexCoord, iNormal;
            VERTEX vertex;
            
            wchar_t symb;
            FaceVertices.clear();
            FaceVerticesInd.clear();

            //for( UINT iFace = 0; iFace < 3; iFace++ )
            for (;;)
            {
                ZeroMemory( &vertex, sizeof( VERTEX ) );

                //skipping spaces
                while (InFile.peek() == ' ')
                    InFile.ignore();

                symb = InFile.peek();
                if ( symb == '\n' )
                {
                    //InFile.ignore();
                    break;
                }
                // OBJ format uses 1-based arrays
                InFile >> iPosition;
                vertex.position = Positions[ iPosition - 1 ];

                if( '/' == InFile.peek() )
                {
                    InFile.ignore();

                    if( '/' != InFile.peek() )
                    {
                        // Optional texture coordinate
                        InFile >> iTexCoord;
                        vertex.texcoord = TexCoords[ iTexCoord - 1 ];
                    }

                    if( '/' == InFile.peek() )
                    {
                        InFile.ignore();

                        // Optional vertex normal
                        InFile >> iNormal;
                        vertex.normal = Normals[ iNormal - 1 ];
                    }
                }
                FaceVertices.push_back(vertex);
                FaceVerticesInd.push_back(iPosition);

                // If a duplicate vertex doesn't exist, add this vertex to the Vertices
                // list. Store the index in the Indices array. The Vertices and Indices
                // lists will eventually become the Vertex Buffer and Index Buffer for
                // the mesh.
                /*DWORD index = AddVertex( iPosition, &vertex );
                if ( index == (DWORD)-1 )
                   return E_OUTOFMEMORY;

                m_Indices.Add( index );*/
            }
            FaceIndices.resize(FaceVertices.size());
            for (UINT i = 0; i < FaceVertices.size(); i++ )
            {
                //triangle strip
                if (i > 2)
                {                    
                    m_Indices.Add( FaceIndices[0] );
                    m_Indices.Add( FaceIndices[i-1] );
                }
                FaceIndices[i] = AddVertex( FaceVerticesInd[i], &FaceVertices[i] );
                if ( FaceIndices[i] == (DWORD)-1 )
                    return E_OUTOFMEMORY;

                m_Indices.Add( FaceIndices[i] );

                if (i >= 2)
                    m_Attributes.Add( dwCurSubset );
            }
        }
        else if( 0 == wcscmp( strCommand, L"mtllib" ) )
        {
            // Material library
            InFile >> strMaterialFilename;
        }
        else if( 0 == wcscmp( strCommand, L"usemtl" ) )
        {
            // Material
            WCHAR strName[MAX_PATH] = {0};
            InFile >> strName;

            bool bFound = false;
            for( int iMaterial = 0; iMaterial < m_Materials.GetSize(); iMaterial++ )
            {
                Material* pCurMaterial = m_Materials.GetAt( iMaterial );
                if( 0 == wcscmp( pCurMaterial->strName, strName ) )
                {
                    bFound = true;
                    dwCurSubset = iMaterial;
                    break;
                }
            }

            if( !bFound )
            {
                pMaterial = new Material();
                if( pMaterial == NULL )
                    return E_OUTOFMEMORY;

                dwCurSubset = m_Materials.GetSize();

                InitMaterial( pMaterial );
                wcscpy_s( pMaterial->strName, MAX_PATH - 1, strName );

                m_Materials.Add( pMaterial );
            }
        }
        else
        {
            // Unimplemented or unrecognized command
        }

        InFile.ignore( 1000, '\n' );
    }

    /* Position analysis: getting AABB */
    int i;
    D3DXVECTOR3 vBBoxMin;
    D3DXVECTOR3 vBBoxMax;
    D3DXVECTOR3 *pCurrent;
    vBBoxMax = vBBoxMin = Positions.GetAt(0);
    for (i = 1; i < Positions.GetSize(); i++)
    {
        pCurrent = &Positions.GetAt(i);
        //min
        if (vBBoxMin.x > pCurrent->x)
            vBBoxMin.x = pCurrent->x;
        if (vBBoxMin.y > pCurrent->y)
            vBBoxMin.y = pCurrent->y;
        if (vBBoxMin.z > pCurrent->z)
            vBBoxMin.z = pCurrent->z;
        //max
        if (vBBoxMax.x < pCurrent->x)
            vBBoxMax.x = pCurrent->x;
        if (vBBoxMax.y < pCurrent->y)
            vBBoxMax.y = pCurrent->y;
        if (vBBoxMax.z < pCurrent->z)
            vBBoxMax.z = pCurrent->z;
    }

    float *output = (float*)(&m_mNormalize);
    output[ 0] = 2.0f/(vBBoxMax[0] - vBBoxMin[0]);
    output[ 4] = 0.0;
    output[ 8] = 0.0;
    output[12] = -(vBBoxMax[0] + vBBoxMin[0]) / (vBBoxMax[0] - vBBoxMin[0]);

    output[ 1] = 0.0;
    output[ 5] = 2.0f / (vBBoxMax[1] - vBBoxMin[1]);
    output[ 9] = 0.0;
    output[13] = -(vBBoxMax[1] + vBBoxMin[1]) / (vBBoxMax[1] - vBBoxMin[1]);


    output[ 2] = 0.0;
    output[ 6] = 0.0;
    output[10] = 2.0f/(vBBoxMax[2]-vBBoxMin[2]);
    output[14] = -(vBBoxMax[2] + vBBoxMin[2]) / (vBBoxMax[2] - vBBoxMin[2]);
    

    output[ 3] = 0.0;
    output[ 7] = 0.0;
    output[11] = 0.0;
    output[15] = 1.0;
    /*********************/
    // Cleanup
    InFile.close();
    DeleteCache();

    // If an associated material file was found, read that in as well.
    if( strMaterialFilename[0] )
    {
        V_RETURN( LoadMaterialsFromMTL( strMaterialFilename ) );
    }

    return S_OK;
}


//--------------------------------------------------------------------------------------
DWORD CMeshLoader10::AddVertex( UINT hash, VERTEX* pVertex )
{
    // If this vertex doesn't already exist in the Vertices list, create a new entry.
    // Add the index of the vertex to the Indices list.
    bool bFoundInList = false;
    DWORD index = 0;

    // Since it's very slow to check every element in the vertex list, a hashtable stores
    // vertex indices according to the vertex position's index as reported by the OBJ file
    if( ( UINT )m_VertexCache.GetSize() > hash )
    {
        CacheEntry* pEntry = m_VertexCache.GetAt( hash );
        while( pEntry != NULL )
        {
            VERTEX* pCacheVertex = m_Vertices.GetData() + pEntry->index;

            // If this vertex is identical to the vertex already in the list, simply
            // point the index buffer to the existing vertex
            if( 0 == memcmp( pVertex, pCacheVertex, sizeof( VERTEX ) ) )
            {
                bFoundInList = true;
                index = pEntry->index;
                break;
            }

            pEntry = pEntry->pNext;
        }
    }

    // Vertex was not found in the list. Create a new entry, both within the Vertices list
    // and also within the hashtable cache
    if( !bFoundInList )
    {
        // Add to the Vertices list
        index = m_Vertices.GetSize();
        m_Vertices.Add( *pVertex );

        // Add this to the hashtable
        CacheEntry* pNewEntry = new CacheEntry;
        if( pNewEntry == NULL )
            return (DWORD)-1;

        pNewEntry->index = index;
        pNewEntry->pNext = NULL;

        // Grow the cache if needed
        while( ( UINT )m_VertexCache.GetSize() <= hash )
        {
            m_VertexCache.Add( NULL );
        }

        // Add to the end of the linked list
        CacheEntry* pCurEntry = m_VertexCache.GetAt( hash );
        if( pCurEntry == NULL )
        {
            // This is the head element
            m_VertexCache.SetAt( hash, pNewEntry );
        }
        else
        {
            // Find the tail
            while( pCurEntry->pNext != NULL )
            {
                pCurEntry = pCurEntry->pNext;
            }

            pCurEntry->pNext = pNewEntry;
        }
    }

    return index;
}


//--------------------------------------------------------------------------------------
void CMeshLoader10::DeleteCache()
{
    // Iterate through all the elements in the cache and subsequent linked lists
    for( int i = 0; i < m_VertexCache.GetSize(); i++ )
    {
        CacheEntry* pEntry = m_VertexCache.GetAt( i );
        while( pEntry != NULL )
        {
            CacheEntry* pNext = pEntry->pNext;
            SAFE_DELETE( pEntry );
            pEntry = pNext;
        }
    }

    m_VertexCache.RemoveAll();
}


//--------------------------------------------------------------------------------------
HRESULT CMeshLoader10::LoadMaterialsFromMTL( const WCHAR* strFileName )
{
    HRESULT hr;

    // Set the current directory based on where the mesh was found
    WCHAR wstrOldDir[MAX_PATH] = {0};
    GetCurrentDirectory( MAX_PATH, wstrOldDir );
    SetCurrentDirectory( m_strMediaDir );

    // Find the file
    WCHAR strPath[MAX_PATH];
    char cstrPath[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( strPath, MAX_PATH, strFileName ) );
    WideCharToMultiByte( CP_ACP, 0, strPath, -1, cstrPath, MAX_PATH, NULL, NULL );

    // File input
    WCHAR strCommand[256] = {0};
    wifstream InFile( cstrPath );
    if( !InFile )
        return DXTRACE_ERR( L"wifstream::open", E_FAIL );

    // Restore the original current directory
    SetCurrentDirectory( wstrOldDir );

    Material* pMaterial = NULL;

    for(; ; )
    {
        InFile >> strCommand;
        if( !InFile )
            break;

        if( 0 == wcscmp( strCommand, L"newmtl" ) )
        {
            // Switching active materials
            WCHAR strName[MAX_PATH] = {0};
            InFile >> strName;

            pMaterial = NULL;
            for( int i = 0; i < m_Materials.GetSize(); i++ )
            {
                Material* pCurMaterial = m_Materials.GetAt( i );
                if( 0 == wcscmp( pCurMaterial->strName, strName ) )
                {
                    pMaterial = pCurMaterial;
                    break;
                }
            }
        }

        // The rest of the commands rely on an active material
        if( pMaterial == NULL )
            continue;

        if( 0 == wcscmp( strCommand, L"#" ) )
        {
            // Comment
        }
        else if( 0 == wcscmp( strCommand, L"Ka" ) )
        {
            // Ambient color
            float r, g, b;
            InFile >> r >> g >> b;
            pMaterial->vAmbient = D3DXVECTOR3( r, g, b );
        }
        else if( 0 == wcscmp( strCommand, L"Kd" ) )
        {
            // Diffuse color
            float r, g, b;
            InFile >> r >> g >> b;
            pMaterial->vDiffuse = D3DXVECTOR3( r, g, b );
        }
        else if( 0 == wcscmp( strCommand, L"Ks" ) )
        {
            // Specular color
            float r, g, b;
            InFile >> r >> g >> b;
            pMaterial->vSpecular = D3DXVECTOR3( r, g, b );
        }
        else if( 0 == wcscmp( strCommand, L"d" ) ||
                 0 == wcscmp( strCommand, L"Tr" ) )
        {
            // Alpha
            InFile >> pMaterial->fAlpha;
        }
        else if( 0 == wcscmp( strCommand, L"Ns" ) )
        {
            // Shininess
            int nShininess;
            InFile >> nShininess;
            pMaterial->nShininess = nShininess;
        }
        else if( 0 == wcscmp( strCommand, L"illum" ) )
        {
            // Specular on/off
            int illumination;
            InFile >> illumination;
            pMaterial->bSpecular = ( illumination == 2 );
        }
        else if( 0 == wcscmp( strCommand, L"map_Kd" ) )
        {
            // Texture
            InFile >> pMaterial->strMapKd;
        }
        else if( 0 == wcscmp( strCommand, L"map_Ks" ) )
        {
            // Texture
            InFile >> pMaterial->strMapKs;
        }

        else
        {
            // Unimplemented or unrecognized command
        }

        InFile.ignore( 1000, L'\n' );
    }

    InFile.close();

    return S_OK;
}


//--------------------------------------------------------------------------------------
void CMeshLoader10::InitMaterial( Material* pMaterial )
{
    ZeroMemory( pMaterial, sizeof( Material ) );

    pMaterial->vAmbient = D3DXVECTOR3( 0.2f, 0.2f, 0.2f );
    pMaterial->vDiffuse = D3DXVECTOR3( 0.8f, 0.8f, 0.8f );
    pMaterial->vSpecular = D3DXVECTOR3( 1.0f, 1.0f, 1.0f );
    pMaterial->nShininess = 50;
    pMaterial->fAlpha = 1.0f;
    pMaterial->bSpecular = false;
    pMaterial->pMapKdRV10 = NULL;
    pMaterial->pMapKsRV10 = NULL;
}

//--------------------------------------------------------------------------------------
void CMeshLoader10::Render( ID3D10EffectPass *a_pPass,
                           ID3D10EffectShaderResourceVariable *a_pMapKdESRV,
                           ID3D10EffectShaderResourceVariable *a_pMapKsESRV,
                           ID3D10EffectVectorVariable *a_pMaterialCoefsESV[3],
                           ID3D10EffectScalarVariable *a_pMaterialShininessESV ) const
{
    //ID3D10EffectPass *pPass = a_pTechnique->GetPassByIndex(0);
    for ( UINT iSubset = 0; iSubset < m_NumAttribTableEntries; ++iSubset )
        //UINT iSubset = 0;
        if (iSubset != 3)
        {
            // Setup material parameters
            //Material *tmp = m_Materials.GetAt(m_pAttribTable[iSubset].AttribId);
            a_pMapKdESRV->SetResource(m_Materials.GetAt(m_pAttribTable[iSubset].AttribId)->pMapKdRV10);
            a_pMapKsESRV->SetResource(m_Materials.GetAt(m_pAttribTable[iSubset].AttribId)->pMapKsRV10);
            a_pMaterialCoefsESV[0]->SetFloatVector(m_Materials.GetAt(m_pAttribTable[iSubset].AttribId)->vDiffuse);
            a_pMaterialCoefsESV[1]->SetFloatVector(m_Materials.GetAt(m_pAttribTable[iSubset].AttribId)->vSpecular);
            a_pMaterialCoefsESV[2]->SetFloatVector(m_Materials.GetAt(m_pAttribTable[iSubset].AttribId)->vAmbient);
            a_pMaterialShininessESV->SetInt(m_Materials.GetAt(m_pAttribTable[iSubset].AttribId)->nShininess);

            a_pPass->Apply(0);
            m_pMesh->DrawSubset(iSubset);
            /*
            UINT uStride = sizeof(VERTEX);
            UINT uOffs   = 0;
            m_pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            m_pd3dDevice->IASetVertexBuffers(0, 1, &m_pVbuf, &uStride, &uOffs );
            m_pd3dDevice->IASetIndexBuffer(m_pIbuf, DXGI_FORMAT_R32_UINT, uOffs );
            m_pd3dDevice->DrawIndexed(m_Indices.GetSize(), 0, 0);
            */
        }
}
