//+-----------------------------------------------------------------------------
//
//  CRendererL2D
//
//      �� Ŭ�������� Live2D �������� ����մϴ�.
//
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "atlstr.h"
#include "Live2D.h"
#include "Live2DModelD3D.h"
#include "util/UtSystem.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#pragma warning( default : 4533 )
#define BUFFER_SIZE 1024

using std::wstring;
using std::vector;

live2d::Live2DModelD3D*	live2DModel;

struct CUSTOMVERTEX
{
	FLOAT x, y, z;
	DWORD color;
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE)

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRendererL2D ������
//
//------------------------------------------------------------------------------
CRendererL2D::CRendererL2D() : CRenderer(), m_pd3dVB(NULL)
{
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRendererL2D �Ҹ���
//
//------------------------------------------------------------------------------
CRendererL2D::~CRendererL2D()
{
	SAFE_RELEASE(m_pd3dVB);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRendererL2D::Create
//
//  Synopsis:
//      �������� �����մϴ�.
//
//------------------------------------------------------------------------------
HRESULT
CRendererL2D::Create(IDirect3D9 *pD3D, HWND hwnd, UINT uAdapter, Argument argument, CRenderer **ppRenderer)
{
	HRESULT hr = S_OK;

	CRendererL2D *pRenderer = new CRendererL2D();
	IFCOOM(pRenderer);

	IFC(pRenderer->Init(pD3D, hwnd, uAdapter, argument));

	*ppRenderer = pRenderer;
	pRenderer = NULL;

Cleanup:
	delete pRenderer;

	return hr;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRendererL2D::Init
//
//  Synopsis:
//      CRenderer::Init�� �������̵� �� �߰����� ó���� �����մϴ�.
//
//------------------------------------------------------------------------------
HRESULT
CRendererL2D::Init(IDirect3D9 *pD3D, HWND hwnd, UINT uAdapter, Argument argument)
{
	HRESULT hr = S_OK;

	// �ʱ�ȭ
	IFC(CRenderer::Init(pD3D, hwnd, uAdapter, argument));

	// ���� �ʱ�ȭ
	live2d::Live2D::init();

	// �� �ҷ�����
	size_t tmp;
	char *strChar = new char[BUFFER_SIZE];
	wcstombs_s(&tmp, strChar, BUFFER_SIZE, argument.model, BUFFER_SIZE);
	live2DModel = live2d::Live2DModelD3D::loadModel(strChar);

	// �ؽ�ó �ҷ����� ����
	for (int i = 0; i < 1000; i++) {
		if (!argument.textures[i]) break;

		LPDIRECT3DTEXTURE9 tex;
		if (FAILED(D3DXCreateTextureFromFileExW(m_pd3dDevice
			, argument.textures[i]
			, 0
			, 0
			, 0
			, 0
			, D3DFMT_A8R8G8B8
			, D3DPOOL_MANAGED
			, D3DX_FILTER_LINEAR
			, D3DX_FILTER_BOX
			, 0
			, NULL
			, NULL
			, &tex)))
		{
			goto Cleanup;
		}
		else
		{
			live2DModel->setTexture(i, tex);
		}
	}

Cleanup:
	return hr;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRendererL2D::BeginRender
//
//  Synopsis:
//      Live2D �������� �����մϴ�.
//
//------------------------------------------------------------------------------
HRESULT CRendererL2D::BeginRender()
{
	HRESULT hr = S_OK;

	// ��� ����
	IFC(m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0));
	IFC(m_pd3dDevice->BeginScene());

	// ��Ʈ���� ����
	D3DXMATRIXA16 matWorld;
	D3DXMatrixIdentity(&matWorld);

	D3DXMATRIX Ortho2D;
	D3DXMATRIX Identity;

	int w, h;
	w = 800;
	h = 800;

	float modelWidth = live2DModel->getModelImpl()->getCanvasWidth();
	float modelHeight = live2DModel->getModelImpl()->getCanvasHeight();

	D3DXMatrixOrthoLH(&Ortho2D, modelHeight, -modelHeight *h / w, -1.0f, 1.0f);
	D3DXMatrixIdentity(&Identity);

	m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &Ortho2D);
	m_pd3dDevice->SetTransform(D3DTS_WORLD, &Identity);
	m_pd3dDevice->SetTransform(D3DTS_VIEW, &Identity);

	D3DXMATRIXA16 world, scale, trans;
	m_pd3dDevice->GetTransform(D3DTS_WORLD, &world);
	D3DXMatrixTranslation(&trans, -modelWidth / 2, -modelHeight / 2, 0);
	world = trans *world;

	m_pd3dDevice->SetTransform(D3DTS_WORLD, &world);

Cleanup:
	return hr;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRendererL2D::EndRender
//
//  Synopsis:
//      Live2D �������� �����ϴ�.
//
//------------------------------------------------------------------------------
HRESULT CRendererL2D::EndRender()
{
	HRESULT hr = S_OK;

	// �׸��� �۾� ����
	live2DModel->setDevice(m_pd3dDevice);
	live2DModel->update();
	live2DModel->draw();

	// �׸��� ����
	IFC(m_pd3dDevice->EndScene());
	IFC(m_pd3dDevice->Present(NULL, NULL, NULL, NULL));

Cleanup:
	return hr;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRendererL2D::SetParamFloat
//
//  Synopsis:
//     Ű�� �ش��ϴ� �Ű������� ���� �����մϴ�.
//
//------------------------------------------------------------------------------
void CRendererL2D::SetParamFloat(char* key, float value)
{
	live2DModel->setParamFloat(key, value);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRendererL2D::AddToParamFloat
//
//  Synopsis:
//      Ű�� �ش��ϴ� �Ű������� ���� ���մϴ�.
//
//------------------------------------------------------------------------------
void CRendererL2D::AddToParamFloat(char* key, float value)
{
	live2DModel->addToParamFloat(key, value);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRendererL2D::MultParamFloat
//
//  Synopsis:
//      Ű�� �ش��ϴ� �Ű������� ���� ���մϴ�.
//
//------------------------------------------------------------------------------
void CRendererL2D::MultParamFloat(char* key, float value)
{
	live2DModel->multParamFloat(key, value);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRendererL2D::GetParamFloat
//
//  Synopsis:
//      Ű�� �ش��ϴ� �Ű������� ���� �����ɴϴ�.
//
//------------------------------------------------------------------------------
float CRendererL2D::GetParamFloat(char* key)
{
	return live2DModel->getParamFloat(key);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRendererL2D::SetPartsOpacity
//
//  Synopsis:
//      Ű�� �ش��ϴ� �κ��� ������ �����մϴ�.
//
//------------------------------------------------------------------------------
void CRendererL2D::SetPartsOpacity(char* key, float value)
{
	live2DModel->setPartsOpacity(key, value);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRendererL2D::GetPartsOpacity
//
//  Synopsis:
//      Ű�� �ش��ϴ� �κ��� ������ �����ɴϴ�.
//
//------------------------------------------------------------------------------
float CRendererL2D::GetPartsOpacity(char* key)
{
	return live2DModel->getPartsOpacity(key);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRendererL2D::SaveParam
//
//  Synopsis:
//      �Ű������� �����մϴ�.
//
//------------------------------------------------------------------------------
void CRendererL2D::SaveParam()
{
	live2DModel->saveParam();
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRendererL2D::LoadParam
//
//  Synopsis:
//      �Ű������� �ҷ��ɴϴ�.
//
//------------------------------------------------------------------------------
void CRendererL2D::LoadParam()
{
	live2DModel->loadParam();
}