#pragma once

#include <string>
#include <map>

#include "includes.h"

class StateManager
{
public:
	~StateManager (void);

	void SetDevice (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx)
	{
		m_pD3DDevice = a_pD3DDevice;
		m_pD3DDeviceCtx = a_pD3DDeviceCtx;
	}

	bool AddRasterizerState(std::string a_sName, D3D11_RASTERIZER_DESC& Desc);

	bool SetRasterizerState(std::string a_sName);

	void ToggleWireframe(void)
	{
		m_bUseWireframe = !m_bUseWireframe;
	}

	bool UseWireframe(void) const
	{
		return m_bUseWireframe;
	}

protected:
	StateManager(void);

	friend StateManager& GetGlobalStateManager();

private:
	typedef std::map< std::string, ID3D11RasterizerState* > RasterStateMap;

	RasterStateMap m_mRasterizerStates;

	ID3D11Device* m_pD3DDevice;
	ID3D11DeviceContext* m_pD3DDeviceCtx;

	bool m_bUseWireframe;
};

StateManager& GetGlobalStateManager(void);
