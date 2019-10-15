#pragma once

#include <string>
#include <map>

#include "includes.h"

class StateManager
{
public:
    ~StateManager( void );

    void SetDevice( ID3D10Device *a_pD3DDevice )
    {
        m_pD3DDevice = a_pD3DDevice;
    }
    
    bool AddRasterizerState( std::string a_sName, D3D10_RASTERIZER_DESC &Desc );

    bool SetRasterizerState( std::string a_sName );

    void ToggleWireframe( void )
    {
        m_bUseWireframe = !m_bUseWireframe;
    }

    bool UseWireframe( void ) const
    {
        return m_bUseWireframe;
    }

protected:
    StateManager( void );

    friend StateManager& GetGlobalStateManager();

private:
    typedef std::map< std::string, ID3D10RasterizerState * > RasterStateMap;
    
    RasterStateMap m_mRasterizerStates;

    ID3D10Device *m_pD3DDevice;

    bool m_bUseWireframe;
};

StateManager& GetGlobalStateManager( void );
