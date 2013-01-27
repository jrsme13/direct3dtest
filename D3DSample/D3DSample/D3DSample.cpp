// D3DSample.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <d3d10.h>
#include <D3DX10.h>
#include "D3DSample.h"

#pragma comment (lib, "d3d10.lib")
#pragma comment (lib, "d3dx10.lib")

#define MAX_LOADSTRING 100

typedef struct
{
	D3DXVECTOR3 position;
	D3DXVECTOR4 colour;
} Vertex;

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

//--------------------------------------------------------------------
// globals
BOOL terminate = FALSE;
HWND g_hwnd = NULL;
IDXGISwapChain* g_pSwapChain = NULL;
ID3D10RenderTargetView* g_pRenderTargetView = NULL;
ID3D10Device* g_pDevice = NULL;
ID3D10EffectTechnique* g_pEffectTechnique = NULL;
ID3D10Effect* g_pEffect = NULL;
ID3D10InputLayout* g_pVertexLayout = NULL;
ID3D10Buffer*	g_pVertexBuffer = NULL;
ID3D10Buffer*               g_pIndexBuffer = NULL;

ID3D10EffectMatrixVariable* g_pWorld = NULL;
ID3D10EffectMatrixVariable* g_pView = NULL;
ID3D10EffectMatrixVariable* g_pProjection = NULL;

D3DXMATRIX g_worldMatrix;
D3DXMATRIX g_viewMatrix;
D3DXMATRIX g_projectionMatrix;

float rot = 0.0f;
//---------------------------------------------------------------------


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

BOOL InitialiseD3D( HWND hwnd, int width, int height );
void CleanupD3D();
BOOL Resize(int width, int height);
BOOL DrawScene();

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_D3DSAMPLE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_D3DSAMPLE));

	// Main message loop:
	while (!terminate )
	{
		if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE) )
		{
			if ( msg.message == WM_QUIT )
			{
				terminate = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			DrawScene();
			//SwapBuffers(g_hdc);
		}
	}

	CleanupD3D();

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_D3DSAMPLE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL; //(HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL; //MAKEINTRESOURCE(IDC_D3DSAMPLE);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;
   RECT clientRect;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   GetClientRect(hWnd, &clientRect);

   if (!InitialiseD3D(hWnd, clientRect.right, clientRect.bottom))
   {
	   CleanupD3D();
	   return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void CleanupD3D()
{
	if (g_pDevice) g_pDevice->ClearState();

	if (g_pRenderTargetView) g_pRenderTargetView->Release();

	if (g_pSwapChain) g_pSwapChain->Release();

	if( g_pIndexBuffer ) g_pIndexBuffer->Release();

	if (g_pVertexBuffer) g_pVertexBuffer->Release();

	if (g_pVertexLayout) g_pVertexLayout->Release();

	if (g_pDevice) g_pDevice->Release();
}

BOOL InitialiseD3D(HWND hwnd, int width, int height)
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory( &swapChainDesc, sizeof(swapChainDesc) );
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;

	DWORD deviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	deviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
#endif

    if( FAILED( D3D10CreateDeviceAndSwapChain( NULL, 
        D3D10_DRIVER_TYPE_HARDWARE, NULL, 
        deviceFlags, D3D10_SDK_VERSION, &swapChainDesc, 
        &g_pSwapChain, &g_pDevice ) ) )
    {
        if( FAILED( D3D10CreateDeviceAndSwapChain( NULL, 
            D3D10_DRIVER_TYPE_REFERENCE, NULL, 
            0, D3D10_SDK_VERSION, &swapChainDesc, 
            &g_pSwapChain, &g_pDevice ) ) )
        {
            return FALSE;
        }
    }

    ID3D10Texture2D *pBackBuffer;
    if( FAILED( g_pSwapChain->GetBuffer( 0, __uuidof( ID3D10Texture2D ), 
              (LPVOID*)&pBackBuffer ) ) )
    {
        return FALSE;
    }

    if(FAILED( g_pDevice->CreateRenderTargetView( pBackBuffer, 
               NULL, &g_pRenderTargetView )))
    {
        return FALSE;
    }

    pBackBuffer->Release();

    g_pDevice->OMSetRenderTargets( 1, &g_pRenderTargetView, NULL );

    D3D10_VIEWPORT vp = {0, 0, width, height, 0, 1};
    g_pDevice->RSSetViewports( 1, &vp );
    
	DWORD shaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;

#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3D10_SHADER_DEBUG;
#endif
	// what shader is this?
	if(FAILED(D3DX10CreateEffectFromFile(
		L"basic.fx", 
		NULL, 
		NULL,
		"fx_4_0",
		shaderFlags, 
		0,
		g_pDevice, 
		NULL, 
		NULL, 
		&g_pEffect, 
		NULL,
		NULL)
		)
	)
	{
		return FALSE;
	}

	g_pEffectTechnique = g_pEffect->GetTechniqueByName("Render");

	g_pWorld = g_pEffect->GetVariableByName("World")->AsMatrix();
	g_pView = g_pEffect->GetVariableByName("View")->AsMatrix();
	g_pProjection = g_pEffect->GetVariableByName("Projection")->AsMatrix();

	D3D10_INPUT_ELEMENT_DESC layout[] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0},
	};

	 UINT numElements = sizeof( layout ) / sizeof( layout[0] );

    // Create the input layout
    D3D10_PASS_DESC PassDesc;
    g_pEffectTechnique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
    if( FAILED( g_pDevice->CreateInputLayout( layout, numElements, PassDesc.pIAInputSignature,
                                          PassDesc.IAInputSignatureSize, &g_pVertexLayout )))
	{
		return FALSE;
	}

	g_pDevice->IASetInputLayout(g_pVertexLayout);


	// vertex buffer
	Vertex vertices[] = 
	{
		{ D3DXVECTOR3( -1.0f, 1.0f, -1.0f ), D3DXVECTOR4( 0.0f, 0.0f, 1.0f, 1.0f ) },
        { D3DXVECTOR3( 1.0f, 1.0f, -1.0f ), D3DXVECTOR4( 0.0f, 1.0f, 0.0f, 1.0f ) },
        { D3DXVECTOR3( 1.0f, 1.0f, 1.0f ), D3DXVECTOR4( 0.0f, 1.0f, 1.0f, 1.0f ) },
        { D3DXVECTOR3( -1.0f, 1.0f, 1.0f ), D3DXVECTOR4( 1.0f, 0.0f, 0.0f, 1.0f ) },
        { D3DXVECTOR3( -1.0f, -1.0f, -1.0f ), D3DXVECTOR4( 1.0f, 0.0f, 1.0f, 1.0f ) },
        { D3DXVECTOR3( 1.0f, -1.0f, -1.0f ), D3DXVECTOR4( 1.0f, 1.0f, 0.0f, 1.0f ) },
        { D3DXVECTOR3( 1.0f, -1.0f, 1.0f ), D3DXVECTOR4( 1.0f, 1.0f, 1.0f, 1.0f ) },
        { D3DXVECTOR3( -1.0f, -1.0f, 1.0f ), D3DXVECTOR4( 0.0f, 0.0f, 0.0f, 1.0f ) },
	};

	D3D10_BUFFER_DESC bd;
    bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( Vertex ) * 8;
    bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = vertices;
    
	if ( FAILED(g_pDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer )) )
	{
		return FALSE;
	}

	// Set vertex buffer
    UINT stride = sizeof( Vertex );
    UINT offset = 0;
    g_pDevice->IASetVertexBuffers( 0, 1, &g_pVertexBuffer, &stride, &offset );

	// Create index buffer
    DWORD indices[] =
    {
        3,1,0,
        2,1,3,

        0,5,4,
        1,5,0,

        3,4,7,
        0,4,3,

        1,6,5,
        2,6,1,

        2,7,6,
        3,7,2,

        6,4,5,
        7,4,6,
    };
    bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( DWORD ) * 36;        // 36 vertices needed for 12 triangles in a triangle list
    bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    InitData.pSysMem = indices;
   	if ( FAILED(g_pDevice->CreateBuffer( &bd, &InitData, &g_pIndexBuffer )) )
	{
		return FALSE;
	}


	// sets index buffer
	g_pDevice->IASetIndexBuffer( g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0 );
    

	//no primitve topology? in drawscene

	D3DXMatrixIdentity(&g_worldMatrix);
	D3DXMatrixIdentity(&g_viewMatrix);
	D3DXMatrixIdentity(&g_projectionMatrix);

	// Initialize the view matrix
    D3DXVECTOR3 Eye( 0.0f, 0.0f, 5.0f );
    D3DXVECTOR3 At( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 Up( 0.0f, 1.0f, 0.0f );
    D3DXMatrixLookAtLH( &g_viewMatrix, &Eye, &At, &Up );

	return TRUE;
}

BOOL Resize(int width, int height)
{
	//g_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN

	D3DXMatrixPerspectiveFovLH( &g_projectionMatrix, 
		(float)D3DX_PI*0.33f,
		(float)width/(float)height,
		0.1f, 100.0f);

	return TRUE;
}

BOOL DrawScene()
{
	float bgColour[] = {0.0f, 0.0f, 0.0f, 1.0f};

	D3D10_TECHNIQUE_DESC technique;

	g_pDevice->ClearRenderTargetView( g_pRenderTargetView, bgColour );

	
	g_pDevice->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	rot = rot + 0.01f*(float)D3DX_PI/180.0f;

	if ( rot > 2.0f*(float)D3DX_PI )
	{
		rot = 0.0f;
	}

	D3DXMatrixRotationY(&g_worldMatrix, rot);
	
	g_pWorld->SetMatrix((float*)&g_worldMatrix);
	g_pView->SetMatrix((float*)&g_viewMatrix);
	g_pProjection->SetMatrix((float*)&g_projectionMatrix);

	D3D10_TECHNIQUE_DESC techDesc;
    g_pEffectTechnique->GetDesc( &techDesc );
    for( UINT p = 0; p < techDesc.Passes; ++p )
    {
		g_pEffectTechnique->GetPassByIndex( p )->Apply( 0 );
		g_pDevice->DrawIndexed(36, 0,0);
    }

	g_pSwapChain->Present(0,0);
	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_SIZE:
		Resize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
