#include <windows.h>
#include "d3dx9.h"
#include <Eigen/dense>

#include "XFileUtil.h"
#include "PhysicalObj.h"
#include "Camera.h"
#include "PhysicsTest.h"
#include <windowsx.h>

VOID SetupViewProjection();
//-----------------------------------------------------------------------------
// 전역 변수 
ZCamera*				g_pCamera = NULL;	// Camera 클래스

HWND hWnd;				// 메시지 박스 처리를 편하게 하기 위해 임시적으로 전역변수화


LPDIRECT3D9             g_pD3D = NULL; // Direct3D 객체 
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL; // 랜더링 장치 (비디오카드)


FLOAT xAngle;
FLOAT yAngle;
POINT mouseDownPt;

BOOL					boomed = FALSE; // 붐 확인. 게임오버 확인
BOOL					g_bBillBoard = FALSE;
BOOL					g_bWoodTexture = FALSE;
FLOAT					m_fDegree=0.f; // LookAt을 위한 변수
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL; // 버텍스 버퍼 
PDIRECT3DVERTEXBUFFER9  g_pVBTexture = NULL; // 텍스쳐 출력용버텍스 버퍼
PDIRECT3DVERTEXBUFFER9  g_pVBBox = NULL; // 육면체 텍스처 출력용 버텍스 버퍼
LPDIRECT3DTEXTURE9      g_pTexture = NULL; // 텍스쳐 로딩용 변수
LPDIRECT3DTEXTURE9      g_pTexture2 = NULL; // 바닥 텍스쳐 로딩용 변수
LPDIRECT3DTEXTURE9      g_pTexture3 = NULL; // 나무 텍스쳐 로딩용 변수
LPDIRECT3DTEXTURE9      g_pTexture4 = NULL; // 나무 텍스쳐 로딩용 변수2
PhysicsSystem			g_ps;

LPDIRECT3DINDEXBUFFER9	g_pIBBox = NULL; // 인덱스 버퍼
ID3DXMesh*				g_pMesh = NULL; // 메쉬
ID3DXMesh*				g_pMesh2 = NULL; //육각형 메쉬
ID3DXMesh*				g_pMesh3 = NULL; //아마도 팔면체

D3DXVECTOR3				playerPosition; // 플레이어 포지션
D3DXVECTOR3				ballPosition; // 공 포지션
D3DXVECTOR3				otherPersonPosition; // 다른 사람들 포지션

CXFileUtil              g_XFile;   // 공
CXFileUtil              g_XFile2;  //호랭이
CXFileUtil              g_XFile3;  // 비행기
CXFileUtil              g_XFile4; // 플레이어
CXFileUtil              g_XFile5; // 골기퍼?
CXFileUtil              g_XFile6; // 다른사람
PhysicalObj				g_PhyObject(0, 0, 0, 10); // 공
PhysicalObj				g_PhyObject2(0, 0, 0, 100); // 플레이어
PhysicalObj				g_PhyObject3(0, 0, 0, 500); // 골기퍼
PhysicalObj				g_PhyObject4(0, 0, 0, 100); // 다른 사람 오브젝트

FLOAT player_position[3] = { 0,0,0 };// 플레이어 포지션
FLOAT tiger_position[3] = { 0,0,0 }; // 호랑이 포지션

// 스프라이트 처리를 위한 구조체 
struct SPRITE {
	int spriteNumber;   // 전체 스프라이트 이미지 수
	int curIndex;       // 현재 출력해야 하는 스프라이트 인덱스 
	int frameCounter;   // 현재 스프라이트를 출력하고 지속된 프레임 수 
	int frameDelay;     // 스프라이트 변경 속도 조절을 위한 프레임 딜레이 
};
// 스프라이트 구조체 변수 선언 및 초기화
SPRITE g_Fire = { 15, 0, 0, 2 }; // 선언 & 구조체의 각 요소 초기화 


// 커스텀 버텍스 타입 구조체 
/*
struct CUSTOMVERTEX
{
	FLOAT x, y, z;    // 3D 좌표값
	DWORD color;      // 버텍스 색상
};
*/

// 커스텀 버텍스의 구조를 표시하기 위한 FVF(Flexible Vertex Format) 값 
// D3DFVF_XYZ(3D 월드 좌표) 와  D3DFVF_DIFFUSE(점의 색상) 특성을 가지도록.
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)



// 텍스쳐 좌표를 가지는 버텍스 구조체 정의
struct MYVERTEX
{
	D3DXVECTOR3     position;  // 버텍스의 위치
	D3DCOLOR        color;     // 버텍스의 색상
};

struct MYINDEX
{
	WORD	_0, _1, _2;
};

// 위 구조체의 구조를 표현하는 FVF 값 정의
#define D3DFVF_MYVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

//골기퍼 디워프 함수
void goalkeeper() {
	ballPosition = g_PhyObject.returnPositionVector();
	float dis[3] = {ballPosition.x,0,ballPosition.z- 280 };
	float length = sqrtf(dis[0] * dis[0] + dis[1] * dis[1] + dis[2] * dis[2]);
	if (boomed && ballPosition.z < 160 && length < 220) {
		if (dis[0] < 40 || dis[0] > -40) {
			//이 경우에만 움직여야함! ( 뒤에서 올 경우 신경쓰지 않는다)
			if (dis[0] > 60) dis[0] = 60;
			if (dis[0] < -60) dis[0] = -60;
			g_PhyObject3.SetPosition(dis[0], 0, 280);
		//MessageBox(hWnd, "BOOM", "BOOM", MB_OK);
			boomed = false;
		}
		//if만약 골이 될경우 게임 종료
		//MessageBox(hWnd, "게임종료", "골인!", MB_OK);
	}
	else {
		boomed = true;
		//MessageBox(hWnd, "BOOM", "BOOM", MB_OK);
	}
	//공이 골기퍼보다 앞에있을 경우(Z가 더 적은수일경우)
	//둘 사이의 거리가 일정거리보다 가까워지면
	//공의 x좌표를 따와 골기퍼의 x좌표를 바꾼다.

}

//플레이어가 아닌 디워프 함수
void otherPersonDo(){
	ballPosition = g_PhyObject.returnPositionVector();
	otherPersonPosition = g_PhyObject4.returnPositionVector();
	
	float dis[3] = { ballPosition.x - otherPersonPosition.x,
		 ballPosition.y - otherPersonPosition.y ,
		 ballPosition.z - otherPersonPosition.z };

	float length = sqrtf(dis[0] * dis[0] + dis[1] * dis[1] + dis[2] * dis[2]);
	
	//공과 사람간의 각도. LookAt
	m_fDegree = atan2f((float)dis[2], (float)dis[0]) * 180 / 3.1415f;

	if (length < 180 && length > 75 ){// && length > 100) {

		if (otherPersonPosition.x < ballPosition.x) // 안움직이는가!
			g_PhyObject4.AddVelocity(0.01f, 0, 0);
		else 
			g_PhyObject4.AddVelocity(-0.01f, 0, 0);

		if (otherPersonPosition.z < ballPosition.z)
			g_PhyObject4.AddVelocity(0, 0, 0.01f);
		else
			g_PhyObject4.AddVelocity(0, 0, -0.01f);
	}
	else {
		g_PhyObject4.SetVelocity(0, 0, 0);
	}

	// 공의 위치를 쳐다본다.
	//일정 거리이상 공이 다가오면
	//공쪽으로 천천히 움직여 공을 찬다. 
	//멀어지면 멈추거나 다시 원래의 자리로 돌아간다
}

//-----------------------------------------------------------------------------
// 이름: InitD3D()
// 기능: Direct3D 초기화 
//-----------------------------------------------------------------------------
HRESULT InitD3D(HWND hWnd)
{
	// Direct3D 객체 생성 
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	// 장치 생성용 데이타 준비

	D3DPRESENT_PARAMETERS d3dpp;         // 장치 생성용 정보 구조체 변수 선언

	ZeroMemory(&d3dpp, sizeof(d3dpp));                  // 구조체 클리어
	d3dpp.BackBufferWidth = 1024;               // 버퍼 해상도 넓이 설정
	d3dpp.BackBufferHeight = 768;               // 버퍼 해상도 높이 설정 
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;   // 버퍼 포맷 설정 
	d3dpp.BackBufferCount = 1;                 // 백버퍼 수 
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;  // 스왑 방법 설정
	d3dpp.hDeviceWindow = hWnd;              // 윈도우 핸들 설정 
	d3dpp.Windowed = true;              // 윈도우 모드로 실행 되도록 함 
	d3dpp.EnableAutoDepthStencil = true;              // 스탠실 버퍼를 사용하도록 함 
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;      // 스탠실 버퍼 포맷 설정 


	// D3D객체의 장치 생성 함수 호출 (디폴트 비디오카드 사용, HAL 사용,
	// 소프트웨어 버텍스 처리사용 지정)
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp, &g_pd3dDevice)))
	{
		return E_FAIL;
	}

	// 이제 장치가 정상적으로 생성되었음.

	// zbuffer 사용하도록 설정
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	// 기본컬링 없음
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	return S_OK;
}

//-----------------------------------------------------------------------------
// 이름: InitMeshData()
// 기능: Mesh에 관련된 각종 데이터를 초기화 한다. 
//-----------------------------------------------------------------------------
HRESULT InitMeshData()
{

	SetupViewProjection();

	g_XFile.XFileLoad(g_pd3dDevice, "./images/ball.x");// 소스코드가 있는 작업폴더/images/tiger.x
	g_XFile2.XFileLoad(g_pd3dDevice, "./images/tiger.x");
	g_XFile3.XFileLoad(g_pd3dDevice, "./images/airplane2.x");
	g_XFile4.XFileLoad(g_pd3dDevice, "./images/Dwarf.x");
	g_XFile5.XFileLoad(g_pd3dDevice, "./images/Dwarf.x");
	g_XFile6.XFileLoad(g_pd3dDevice, "./images/Dwarf.x");
	//	g_XFile.XFileLoad(g_pd3dDevice, "C:/Program Files/Microsoft DirectX SDK (August 2009)/Samples/Media/Dwarf/Dwarf.x");


	D3DXVECTOR3 min, max;
	D3DXVECTOR3 center;
	float radius;
	BYTE* v = 0;

	//공
	g_XFile.GetMesh()->LockVertexBuffer(0, (void**)&v);

	HRESULT hr = D3DXComputeBoundingBox(
		(D3DXVECTOR3*)v,
		g_XFile.GetMesh()->GetNumVertices(),
		D3DXGetFVFVertexSize(g_XFile.GetMesh()->GetFVF()),
		&min,
		&max);
	hr = D3DXComputeBoundingSphere(
		(D3DXVECTOR3*)v,
		g_XFile.GetMesh()->GetNumVertices(),
		D3DXGetFVFVertexSize(g_XFile.GetMesh()->GetFVF()),
		&center,
		&radius);
	g_XFile.GetMesh()->UnlockVertexBuffer();
	g_PhyObject.SetBoundingBox(min, max);
	g_PhyObject.SetBoundingSphere(center, radius);
	g_PhyObject.SetPosition(0, 0, -200);

	//플레이어
	g_XFile4.GetMesh()->LockVertexBuffer(0, (void**)&v);
	hr = D3DXComputeBoundingBox(
		(D3DXVECTOR3*)v,
		g_XFile4.GetMesh()->GetNumVertices(),
		D3DXGetFVFVertexSize(g_XFile4.GetMesh()->GetFVF()),
		&min,
		&max);
	hr = D3DXComputeBoundingSphere(
		(D3DXVECTOR3*)v,
		g_XFile4.GetMesh()->GetNumVertices(),
		D3DXGetFVFVertexSize(g_XFile4.GetMesh()->GetFVF()),
		&center,
		&radius);
	g_XFile4.GetMesh()->UnlockVertexBuffer();

	g_PhyObject2.SetBoundingBox(min, max);
	g_PhyObject2.SetBoundingSphere(center, radius);
	g_PhyObject2.SetPosition(0, 0, -300);
	g_PhyObject3.SetBoundingBox(min, max);
	g_PhyObject3.SetBoundingSphere(center, radius);
	g_PhyObject3.SetPosition(0, 0,280);
	g_PhyObject4.SetBoundingBox(min, max);
	g_PhyObject4.SetBoundingSphere(center, radius);
	g_PhyObject4.SetPosition(200, 0, 100);



	return S_OK;


}


//-----------------------------------------------------------------------------
// 이름: InitGeometry()
// 기능: 버텍스 버퍼를 생성한 후 버텍스로 채운다. 
//-----------------------------------------------------------------------------
HRESULT InitGeometry()
{
	g_ps.CreateDXBuffers2(g_pd3dDevice);
	//g_ps.CreateDXBuffers2(g_pd3dDevice);
	// 버텍스 버퍼에 넣을 버텍스 자료를 임시로 만든다. 
	CUSTOMVERTEX vertices[] =
	{
		{ -300.0f,  0.0f, 0.0f, 0xff00ff00, }, // x축 라인을 위한 버텍스 
		{  300.0f,  0.0f, 0.0f, 0xff00ff00, },

		{ 0.0f, 0.0f, -300.0f, 0xffffff00, },  // z축 라인을 위한 버텍스
		{ 0.0f, 0.0f,  300.0f, 0xffffff00, },

		{ 0.0f, 0.0f, 0.0f, 0xffff0000, },  // y축 라인을 위한 버텍스
		{ 0.0f,  100.0f, 0.0f, 0xffff0000, },

		{   0.0f, 50.0f, 0.0f, 0xffff0000, },  // 삼각형의 첫 번째 버텍스 
		{ -50.0f,  0.0f, 0.0f, 0xffff0000, },  // 삼각형의 두 번째 버텍스 
		{  50.0f,  0.0f, 0.0f, 0xffff0000, },  // 삼가형의 세 번째 버텍스 
	};

	// 버텍스 버퍼를 생성한다.
	// 각 버텍스의 포맷은 D3DFVF_CUSTOMVERTEX 라는 것도 전달 
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(9 * sizeof(CUSTOMVERTEX),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &g_pVB, NULL)))
	{
		return E_FAIL;
	}

	// 버텍스 버퍼에 락을 건 후 버텍스를 넣는다. 
	VOID* pVertices;
	if (FAILED(g_pVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0)))
		return E_FAIL;
	memcpy(pVertices, vertices, sizeof(vertices));
	g_pVB->Unlock();

	return S_OK;
}




//-----------------------------------------------------------------------------
// 이름: InitGeometryTexture()
// 기능: 텍스쳐 출력을 위한 버텍스 버퍼를 생성한 후 버텍스로 채운다. 
//-----------------------------------------------------------------------------
HRESULT InitGeometryTexture()
{
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	// 텍스쳐 로딩 
	if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, "./Images/lake.bmp", &g_pTexture))) //깃발
	{
		return E_FAIL;
	}
	if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, "./Images/green.bmp", &g_pTexture2))) //바닥
	{
		return E_FAIL;
	}
	if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, "./Images/tree02S.dds", &g_pTexture3))) //나무
	{
		return E_FAIL;
	}
	if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, "./Images/tree35S.dds", &g_pTexture4))) //나무2
	{
		return E_FAIL;
	}

	// 버텍스 버퍼 생성 
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(4 * sizeof(TEXTUREVERTEX), 0,
		D3DFVF_TEXTUREVERTEX, D3DPOOL_DEFAULT, &g_pVBTexture, NULL)))
	{
		return E_FAIL;
	}	


	// 버텍스 버퍼 설정  //이미지

	TEXTUREVERTEX* pVertices;
	if (FAILED(g_pVBTexture->Lock(0, 0, (void**)&pVertices, 0)))
		return E_FAIL;
	pVertices[0].position = D3DXVECTOR3(-50, 100, 100);  // 버텍스 위치 
	pVertices[0].color = 0xffffffff;                  // 버텍스 알파 및 색상 
	pVertices[0].tu = 0.0f;                        // 버텍스 U 텍스쳐 좌표 
	pVertices[0].tv = 0.0f;                        // 버텍스 V 텍스쳐 좌표 

	pVertices[1].position = D3DXVECTOR3(50, 100, 100);
	pVertices[1].color = 0xffffffff;
	pVertices[1].tu = 1.0f;
	pVertices[1].tv = 0.0f;

	pVertices[2].position = D3DXVECTOR3(-50, 0, 100);
	pVertices[2].color = 0xffffffff;
	pVertices[2].tu = 0.0f;
	pVertices[2].tv = 1.0f;

	pVertices[3].position = D3DXVECTOR3(50, 0, 100);
	pVertices[3].color = 0xffffffff;
	pVertices[3].tu = 1.0f;
	pVertices[3].tv =1.0f;




	g_pVBTexture->Unlock();

	return S_OK;
}

//-----------------------------------------------------------------------------
// 이름: InitGeometryMesh()
// 기능: Mesh를 이용하여 정점 및 인덱스 버퍼를 채운다.
// 참고: 텍스처는 적용되어 있지 않다.
//-----------------------------------------------------------------------------
HRESULT InitGeometryMesh()
{
	HRESULT hr = D3DXCreateMeshFVF(12, 8, D3DXMESH_MANAGED, D3DFVF_MYVERTEX, g_pd3dDevice, &g_pMesh);
	if (FAILED(hr))
		return E_FAIL;
	hr = D3DXCreateMeshFVF(12, 8, D3DXMESH_MANAGED, D3DFVF_MYVERTEX, g_pd3dDevice, &g_pMesh2);
	if (FAILED(hr))
		return E_FAIL;
	hr = D3DXCreateMeshFVF(12, 8, D3DXMESH_MANAGED, D3DFVF_MYVERTEX, g_pd3dDevice, &g_pMesh3);
	if (FAILED(hr))
		return E_FAIL;


	
	// 버텍스 버퍼 생성
	if( FAILED( g_pd3dDevice->CreateVertexBuffer( 8*sizeof(MYVERTEX), 0,
	D3DFVF_MYVERTEX, D3DPOOL_DEFAULT, &g_pVBBox, NULL ) ) )
	{
	return E_FAIL;
	}
	if ( FAILED( g_pd3dDevice->CreateIndexBuffer( 36*sizeof(D3DFMT_INDEX16), 0,
	D3DFMT_INDEX16, D3DPOOL_DEFAULT, &g_pIBBox, NULL ) ) )
	{
	return E_FAIL;
	}
	
	float vertices[8][3] = {
		{ -30,30,30 },
		{ 30,30,30 },
		{ 30,30,-30 },
		{ -30,30,-30 },
		{ -30,-30,30 },
		{ 30,-30,30 },
		{ 30,-30,-30 },
		{ -30,-30,-30 }
	};
	DWORD colors[8] = {
		0x0000ffff,
		0x00ffffff,
		0x00ffff00,
		0x0000ff00,
		0x000000ff,
		0x00ff00ff,
		0x00ff0000,
		0x00000000
	};

	//골대
	// 버텍스 버퍼 설정 
	MYVERTEX* pVertices;
	g_pMesh->LockVertexBuffer(0, (void**)&pVertices);

	//if( FAILED( g_pVBBox->Lock( 0, 0, (void**)&pVertices, 0 ) ) )
	//		return E_FAIL;
	for (int i = 0; i < 8; i++) {
		pVertices[i].position = D3DXVECTOR3(vertices[i]);
		pVertices[i].color = 0x00696865; 
	
	}

	for (int i = 0; i < 2; i++) { pVertices[i].color = 0x003f3e3c; 	}
	for (int i = 0; i < 2; i++) { pVertices[i + 4].color = 0x003f3e3c;}

	g_pMesh->UnlockVertexBuffer();
	//	g_pVBBox->Unlock();

	// 인덱스 버퍼 설정
	MYINDEX *pIndices;
	g_pMesh->LockIndexBuffer(0, (void**)&pIndices);
	//	if(FAILED(g_pIBBox->Lock(0, 0, (void**)&pIndices, 0)))
	//		return E_FAIL;

	WORD indeces[8][3] = {
	{ 0,1,2 },{ 0,2,3 }, // 위
	//{ 4,6,5 },{ 4,7,6 }, // 아래
	{ 0,3,7 },{ 0,7,4 }, // 왼
	{ 1,5,6 },{ 1,6,2 }, // 오른
	//{ 3,2,6 },{ 3,6,7 }, // 앞
	{ 0,4,5 },{ 0,5,1 }  // 뒤
	};

	for (int i = 0; i < 8; i++) {
		pIndices[i]._0 = indeces[i][0];
		pIndices[i]._1 = indeces[i][1];
		pIndices[i]._2 = indeces[i][2];
	}
	g_pMesh->UnlockIndexBuffer();
	//g_pIBBox->Unlock();

	
	LPDWORD attBuf = NULL; // LPDWORD = uunsigned long *
	g_pMesh->LockAttributeBuffer(0, &attBuf);//attBuf에 정수 12개 배열.
	
	//if (FAILED(g_pIBBox->Lock(0, 0, (void**)&pIndices, 0)))
	//	return E_FAIL;

	for (int i = 0; i < 8; i++)
		attBuf[i] = 0;
	//for(int i=6; i<12; i++)
	//		attBuf[i] = 1;

	g_pMesh->UnlockAttributeBuffer();
	//g_pIBBox->Unlock();
	

	//육면체
	// 버텍스 버퍼 설정 
	MYVERTEX *pVertices2;
	g_pMesh2->LockVertexBuffer(0, (void**)&pVertices2);

	//	if( FAILED( g_pVBBox->Lock( 0, 0, (void**)&pVertices2, 0 ) ) )
	//		return E_FAIL;
	for (int i = 0; i < 8; i++) {
		pVertices2[i].position = D3DXVECTOR3(vertices[i]);
		pVertices2[i].color = 0x004a4324;
	}
	g_pMesh2->UnlockVertexBuffer();

		g_pVBBox->Unlock();

	// 인덱스 버퍼 설정
	MYINDEX *pIndices2;
	g_pMesh2->LockIndexBuffer(0, (void**)&pIndices2);
	//	if(FAILED(g_pIBBox->Lock(0, 0, (void**)&pIndices, 0)))
	//		return E_FAIL;

	WORD indeces2[12][3] = {
		{ 0,1,2 },{ 0,2,3 }, // 위
		{ 4,6,5 },{ 4,7,6 }, // 아래
		{ 0,3,7 },{ 0,7,4 }, // 왼
		{ 1,5,6 },{ 1,6,2 }, // 오른
		{ 3,2,6 },{ 3,6,7 }, // 앞
		{ 0,4,5 },{ 0,5,1 }  // 뒤
	};

	for (int i = 0; i < 12; i++) {
		pIndices2[i]._0 = indeces2[i][0];
		pIndices2[i]._1 = indeces2[i][1];
		pIndices2[i]._2 = indeces2[i][2];
	}
	g_pMesh2->UnlockIndexBuffer();
	//	g_pIBBox->Unlock();

	LPDWORD attBuf2 = NULL; // LPDWORD = uunsigned long *
	g_pMesh2->LockAttributeBuffer(0, &attBuf2);//attBuf에 정수 12개 배열.
	for (int i = 0; i < 12; i++)
		attBuf2[i] = 0;
	//for(int i=6; i<12; i++)
	//		attBuf[i] = 1;

	g_pMesh2->UnlockAttributeBuffer();


	
	//팔면체
	float vertices2[6][3] = {
		{ 0,10,0}, // 위 점

		{ -5,0,5 },
		{ 5,0,5 },
		{ 5,0,-5 },
		{ -5,0,-5 }, // 중간네모

		{ 0,-10,0 } //밑 점
	};

	// 버텍스 버퍼 설정 
	MYVERTEX* pVertices3;
	g_pMesh3->LockVertexBuffer(0, (void**)&pVertices3);

	//	if( FAILED( g_pVBBox->Lock( 0, 0, (void**)&pVertices, 0 ) ) )
	//		return E_FAIL;
	for (int i = 0; i < 6; i++) {
		pVertices3[i].position = D3DXVECTOR3(vertices2[i]);
		pVertices3[i].color = 0x0000fff9;
	}
	pVertices3[0].color = 0x000000FF;
	pVertices3[5].color = 0x000000FF;

	g_pMesh3->UnlockVertexBuffer();
	//	g_pVBBox->Unlock();

	// 인덱스 버퍼 설정
	MYINDEX *pIndices3;
	g_pMesh3->LockIndexBuffer(0, (void**)&pIndices3);
	//	if(FAILED(g_pIBBox->Lock(0, 0, (void**)&pIndices, 0)))
	//		return E_FAIL;

	WORD indeces3[8][3] = { // 면의개수
		{0,1,2},{0,2,3},{0,3,4},{0,1,4}, // 위 사면체
		{5,1,2},{5,2,3},{5,3,4},{5,1,4} // 밑 사면체
	};

	for (int i = 0; i < 8; i++) {
		pIndices3[i]._0 = indeces3[i][0];
		pIndices3[i]._1 = indeces3[i][1];
		pIndices3[i]._2 = indeces3[i][2];
	}
	g_pMesh3->UnlockIndexBuffer();
	//	g_pIBBox->Unlock();

	LPDWORD attBuf3 = NULL; // LPDWORD = uunsigned long *
	g_pMesh3->LockAttributeBuffer(0, &attBuf3);//attBuf에 정수 12개 배열.
	for (int i = 0; i < 8; i++)
		attBuf3[i] = 0;
	//for(int i=6; i<12; i++)
	//		attBuf[i] = 1;

	g_pMesh3->UnlockAttributeBuffer();



	return S_OK;
}


//-----------------------------------------------------------------------------
// 이름: Cleanup()
// 기능: 초기화되었던 모든 객체들을 해제한다. 
//-----------------------------------------------------------------------------
VOID Cleanup()
{
	
	g_ps.DeleteDXBuffers();
	if (g_pVB != NULL)
		g_pVB->Release();
	if (g_pVBTexture != NULL)
		g_pVBTexture->Release();
	if (g_pTexture != NULL)
		g_pTexture->Release();
	if (g_pTexture2 != NULL)
		g_pTexture2->Release();
	if (g_pTexture3 != NULL)
		g_pTexture3->Release();
	if (g_pTexture4 != NULL)
		g_pTexture4->Release();

		if (g_pVBBox != NULL)
			g_pVBBox->Release();


	if (g_pd3dDevice != NULL)    // 장치 객체 해제 
		g_pd3dDevice->Release();

	if (g_pD3D != NULL)          // D3D 객체 해제 
		g_pD3D->Release();

	if (g_pMesh != NULL)
		g_pMesh->Release();

	if (g_pMesh2 != NULL)
		g_pMesh2->Release();

	if (g_pMesh3 != NULL)
		g_pMesh3->Release();
}



//-----------------------------------------------------------------------------
// 이름: SetupViewProjection()
// 기능: 뷰 변환과 프로젝션 변환을 설정한다. 
//-----------------------------------------------------------------------------
VOID SetupViewProjection()
{
	playerPosition = g_PhyObject2.returnPositionVector();
	//카메라위치랑 바라보는 지점을 수정해야함!
	// 뷰 변환 설정 
	D3DXVECTOR3 vEyePt(playerPosition[0], 250.0f, playerPosition[2] - 600);
	//D3DXVECTOR3 vEyePt(100.0f, 400.0f, -400.0f);    // 카메라의 위치 
	//D3DXVECTOR3 vLookatPt(playerPosition[0],20.0f, playerPosition[2] + 200);       // 바라보는 지점 
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);       // 바라보는 지점 
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);          // 업벡터 설정 
	D3DXMATRIXA16 matView;                           // 뷰변환용 매트릭스 
	// 뷰 매트릭스 설정 
	//D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
	// Direct3D 장치에 뷰 매트릭스 전달 
	//g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

	// 프로젝션 변환 설정 
	D3DXMATRIXA16 matProj;   // 프로젝션용 매트릭스 
	// 프로젝션 매트릭스 설정 
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 1000.0f);
	// Direct3D 장치로 프로젝션 매트릭스 전달 
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);

	/// 카메라 초기화

	g_pCamera->SetView(&vEyePt, &vLookatPt, &vUpVec);
	D3DXMATRIXA16*	pmatView = g_pCamera->GetViewMatrix();		// 카메라 행렬을 얻는다.
	g_pd3dDevice->SetTransform(D3DTS_VIEW, pmatView);			// 카메라 행렬 셋팅
}


// 색상을 미리 정해 놓으면 편리하다.
D3DCOLORVALUE black = { 0, 0, 0, 1 };
D3DCOLORVALUE dark_gray = { 0.2f, 0.2f, 0.2f, 1.0f };
D3DCOLORVALUE gray = { 0.5f, 0.5f, 0.5f, 1.0f };
D3DCOLORVALUE red = { 1.0f, 0.0f, 0.0f, 1.0f };
D3DCOLORVALUE white = { 1.0f, 1.0f, 1.0f, 1.0f };
VOID SetupLight()
{
	D3DLIGHT9 light;                         // Direct3D 9 조명 구조체 변수 선언

	ZeroMemory(&light, sizeof(D3DLIGHT9));
	light.Type = D3DLIGHT_DIRECTIONAL;       // 조명 타입을 디렉셔널로 설정
	light.Diffuse = white;                   // 조명의 색 설정
	light.Specular = white;
	light.Direction = D3DXVECTOR3(10, -10, 10);       //  조명의 방향 (진행하는 방향) 
													  //light.Direction = D3DXVECTOR3(20*sin(g_counter*0.01f), -10, 10);       //  조명의 방향 (진행하는 방향) 
													  //light.Direction = D3DXVECTOR3(10, 25, -40);       //  조명의 방향 (진행하는 방향) 
	g_pd3dDevice->SetLight(0, &light);      // 라이트 번호 지정 (여기에서는 0번)
	g_pd3dDevice->LightEnable(0, TRUE);     // 0번 라이트 켜기


	// 라이트 사용 기능을 TRUE로 함. (이 기능을 끄면 모든 라이트 사용은 중지됨)
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
	// 최종적으로 엠비언트 라이트 켜기 (환경광의 양을 결정)
	g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0x00202020);
}

HRESULT ChangeSpriteUV(SPRITE *sp)
{
	// for tree texture
	if (g_bWoodTexture)
		return S_OK;

	float u = (sp->curIndex * 64.0f) / 960.0f;       // 현재 인덱스를 이용한 u 계산 
	float u2 = ((sp->curIndex + 1) * 64.0f) / 960.0f;  // 현재 인덱스+1을 이용한 u 계산

	TEXTUREVERTEX* pVertices;   // 버텍스 버퍼 접근용 포인터
	if (FAILED(g_pVBTexture->Lock(0, 0, (void**)&pVertices, 0)))
		return E_FAIL;

	pVertices[0].tu = u;     // u 좌표 변경 
	pVertices[0].tv = 0.0f;  // v 좌표 변경 

	pVertices[1].tu = u2;
	pVertices[1].tv = 0.0f;

	pVertices[2].tu = u;
	pVertices[2].tv = 1.0f;

	pVertices[3].tu = u2;
	pVertices[3].tv = 1.0f;

	g_pVBTexture->Unlock();

	// 지정된 딜레이 프레임이 지난 경우 
	if (sp->frameCounter >= sp->frameDelay) {
		sp->curIndex = (sp->curIndex + 1) % sp->spriteNumber; // 인덱스 변경 
		sp->frameCounter = 0;   // 프레임 카운터 초기화 
	}
	else  // 아직 변경할 시간이 안된 경우 
		sp->frameCounter++;    // 프레임 카운터 증가 

	return S_OK;

}


//-----------------------------------------------------------------------------
// 이름: Render()
// 기능: 화면을 그린다.
//-----------------------------------------------------------------------------
VOID Render()
{
	if (NULL == g_pd3dDevice)  // 장치 객체가 생성되지 않았으면 리턴 
		return;

	// 뷰 및 프로젝션 변환 설정
	//SetupViewProjection();

	// 삼각형의 앞/뒤 변을 모두 렌더링하도록 컬링 기능을 끈다.
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// 조명을 끈다. (조명에 의한 색상이 아니고, 버텍스 자체의 색상을 사용하도록)
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	// 백버퍼를 지정된 색상으로 지운다.
	// 백버퍼를 클리어
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	// 화면 그리기 시작 
	if (SUCCEEDED(g_pd3dDevice->BeginScene()))
	{
		///// 버텍스 출력 
		// 버텍스 버퍼 지정 
		g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
		g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX); // 버텍스 포멧 지정 

		D3DXMATRIXA16 matWorld;  // 월드 변환용 매트릭스 선언 
		D3DXMATRIXA16 matWorld2;

		// 텍스쳐 출력 환경 설정
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

		/*
		// 텍스쳐 출력 환경 설정
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ); //텍스쳐정보
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE ); // 정점 정보
		*/


		SetupLight(); // 노멀벡터가 없어 조명의 영향을 받지 못함.
		//x 파일의 텍스쳐 설정
		//g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2); // 정점 색깔만 그림을 그린다.
		// X 파일 출력
		//D3DXMatrixScaling(&matWorld, 50.0f, 50.0f, 50.0f);

		//공
		matWorld = g_PhyObject.GetWorldMatrix();
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_XFile.XFileDisplay(g_pd3dDevice);
		//g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

		//플레이어
		matWorld = g_PhyObject2.GetWorldMatrix();
		D3DXMatrixRotationY(&matWorld2, 180.0f * (D3DX_PI / 180.0f)); // 180도 회전,,, 
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_XFile4.XFileDisplay(g_pd3dDevice);
	
		//골기퍼
		matWorld = g_PhyObject3.GetWorldMatrix();
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_XFile5.XFileDisplay(g_pd3dDevice);

		//다른사람
		matWorld = g_PhyObject4.GetWorldMatrix();
		D3DXMatrixRotationY(&matWorld2, -1 * (m_fDegree + 90) * (D3DX_PI / 180.0f)); 
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_XFile6.XFileDisplay(g_pd3dDevice);

		D3DXMatrixIdentity(&matWorld);
		
		// 조명 중지
		g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

		// 텍스쳐 설정 (텍스쳐 매핑을 위하여 g_pTexture를 사용하였다.)
		//D3DXMatrixTranslation(&matWorld2, 200, 0, 0);
		//g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_pd3dDevice->SetTexture(0, g_pTexture3);

		if (g_bBillBoard) {
			D3DXMATRIXA16*	pmatBill = g_pCamera->GetBillMatrix();		// 카메라 행렬을 얻는다.
			pmatBill->_41 = 0;
			pmatBill->_42 = 0;
			pmatBill->_43 = 0;
			g_pd3dDevice->SetTransform(D3DTS_WORLD, pmatBill);			// 카메라 행렬 셋팅
		}

		if (g_bWoodTexture) {
			g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		}
		else {
			g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE); // TRUE 로 변경
			g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		}

	
		// 출력할 버텍스 버퍼 설정
		g_pd3dDevice->SetStreamSource(0, g_pVBTexture, 0, sizeof(TEXTUREVERTEX));
		
		// FVF 값 설정
		g_pd3dDevice->SetFVF(D3DFVF_TEXTUREVERTEX);

		//바닥 텍스쳐 설정
		g_pd3dDevice->SetTexture(0, g_pTexture2);
		D3DXMatrixTranslation(&matWorld, 0,-50,-100);
		D3DXMatrixRotationX(&matWorld2,90*(D3DX_PI / 180.0f)); // Z축 90도 회전
		D3DXMatrixMultiply(&matWorld, &matWorld, &matWorld2);
		D3DXMatrixScaling(&matWorld2, 6.f,6.f,8.f );
		D3DXMatrixMultiply(&matWorld, &matWorld, &matWorld2);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

		//나무 텍스쳐
		g_pd3dDevice->SetTexture(0, g_pTexture3);
		D3DXMatrixTranslation(&matWorld, -200, 20, 430);
		D3DXMatrixScaling(&matWorld2, 2.f, 2.f, 0.f);
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

		D3DXMatrixTranslation(&matWorld, 200, 20, 430);
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

		g_pd3dDevice->SetTexture(0, g_pTexture4);
		D3DXMatrixTranslation(&matWorld, 0, 20, 480);
		D3DXMatrixScaling(&matWorld2, 2.f, 2.f, 0.f);
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);


		// 텍스쳐 설정 해제
		g_pd3dDevice->SetTexture(0, NULL);
		// 버텍스들의 알파셋에 대하여 블랜딩 설정
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

		//골대
		//D3DXMatrixIdentity(&matWorld);
		D3DXMatrixTranslation(&matWorld, 0, 60, 350);
		D3DXMatrixScaling(&matWorld2, 2.5f, 2.0f, 1.5f);
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_pMesh->DrawSubset(0);
		D3DXMatrixIdentity(&matWorld);


		//플레이어의 팔면체
		static float angle = 0;
		angle += 0.1f;
		playerPosition = g_PhyObject2.returnPositionVector();
		D3DXMatrixRotationY(&matWorld2, angle);
		D3DXMatrixTranslation(&matWorld, playerPosition[0], playerPosition[1] + 100, playerPosition[2]); // 플레이어의 머리 위. 플레이어의 x, z, y를 가져오자
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_pMesh3->DrawSubset(0);


		//테두리
		D3DXMatrixTranslation(&matWorld, 0, 15, 415);
		D3DXMatrixScaling(&matWorld2, 10.f, 0.5f, 0.5f);
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_pMesh2->DrawSubset(0);
		
		D3DXMatrixTranslation(&matWorld, 0, 15, -415);
		D3DXMatrixScaling(&matWorld2, 10.f, 0.5f, 0.5f);
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_pMesh2->DrawSubset(0);

		D3DXMatrixTranslation(&matWorld, -315, 15, 0);
		D3DXMatrixScaling(&matWorld2, 0.5f, 0.5f, 14.35f);
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_pMesh2->DrawSubset(0);

		D3DXMatrixTranslation(&matWorld, 315, 15, 0);
		D3DXMatrixScaling(&matWorld2, 0.5f, 0.5f, 14.35f);
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_pMesh2->DrawSubset(0);



		//Spring. 깃발
		//엔진 내부에서 힘을 께산, 위치계산
		//D3DXMatrixIdentity(&matWorld);
		D3DXMatrixTranslation(&matWorld, -300, 100,300);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_ps.CalcForce();
		g_ps.Move(0.1f);
		//DX버퍼를 갱신, 그림 그리기
		g_ps.UpdateBuffers();
		g_ps.RenderBuffers(g_pd3dDevice, g_pTexture);

		//깃대
		D3DXMatrixTranslation(&matWorld, -210, 100, 300);
		D3DXMatrixScaling(&matWorld2, 0.1f, 3.0f, 0.1f);
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_pMesh2->DrawSubset(0);


		// 화면 그리기 끝 
		g_pd3dDevice->EndScene();
	}

	// 백버퍼의 내용을 화면으로 보낸다. 
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}



//-----------------------------------------------------------------------------
// 이름 : MsgProc()
// 기능 : 윈도우 메시지 핸들러 
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//공이 안 움직일 경우 원래 위치로 돌아가게 하는 코드를 생성합시다
	switch (msg)
	{

	case WM_DESTROY:
		Cleanup();   // 프로그램 종료시 객체 해제를 위하여 호출함 
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		Render();    // 화면 출력을 담당하는 렌더링 함수 호출 
		ValidateRect(hWnd, NULL);
		return 0;

	case WM_LBUTTONDOWN: // 마우스입력
	{
		mouseDownPt.x = GET_X_LPARAM(lParam);
		mouseDownPt.y = GET_Y_LPARAM(lParam);

		SetCapture(hWnd);

		return 0;
	}
	case WM_MOUSEMOVE:
	{
		if (GetCapture() == hWnd) {
			int X = GET_X_LPARAM(lParam);
			int Y = GET_Y_LPARAM(lParam);

			xAngle += (Y - mouseDownPt.y) / 3.6;
			yAngle = (X - mouseDownPt.x) / 3.6;

			g_pCamera->RotateLocalX(yAngle/30);

			InvalidateRect(hWnd, NULL, FALSE);

			mouseDownPt.x = X;
			mouseDownPt.y = Y;
		}

		return 0;
	}
	case WM_LBUTTONUP:
	{
		ReleaseCapture();
		return 0;
	}

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_SPACE:
			//g_PhyObject2.AddVelocity(0.0f, 3.3f, 0); // 점프(?)
			break;
		case 'A':
			g_PhyObject2.AddVelocity(-0.1f, 0, 0);
			player_position[0] -= 3.0f;
			break;
		case 'D':
			g_PhyObject2.AddVelocity(0.1f, 0, 0);
			player_position[0] += 3.0f;
			break;
		case 'W':
			g_PhyObject2.AddVelocity(0, 0, 0.1f);
			player_position[2] += 3.0f;
			break;
		case 'S':
			g_PhyObject2.AddVelocity(0, 0, -0.1f);
			player_position[2] -= 3.0f;
			break;
		case VK_SHIFT:
			g_PhyObject.SetPosition(0, 0, -200);
			g_PhyObject.SetVelocity(0, 0, 0);
			g_PhyObject2.SetPosition(0, 0, -300);
			g_PhyObject2.SetVelocity(0, 0, 0);
			g_PhyObject3.SetPosition(0, 0, 280);
			g_PhyObject4.SetPosition(200, 0, 100);
			g_PhyObject4.SetVelocity(0, 0, 0);
			break;
		}
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void ProcessKey(void)
{
	D3DXVECTOR3 p = { playerPosition[0], 200.0f, playerPosition[2] - 300 };
	g_pCamera->MoveTo(&p);

	if (GetAsyncKeyState(VK_LEFT)) {
		//g_pCamera->MoveLocalX(-2.5f);	// 카메라 왼쪽
		g_pCamera->RotateLocalY(-.01f);
	}
	if (GetAsyncKeyState(VK_RIGHT)) {
		//g_pCamera->MoveLocalX(2.5f);	// 카메라 오른쪽
		g_pCamera->RotateLocalY(.01f);
	}

	D3DXMATRIXA16*	pmatView = g_pCamera->GetViewMatrix();		// 카메라 행렬을 얻는다.
	g_pd3dDevice->SetTransform(D3DTS_VIEW, pmatView);			// 카메라 행렬 셋팅

}


void Program_Action()
{
	
	// 저절로 움직임은 없고, 카메라 이동만 수행
	ProcessKey(); //카메라이동
	g_PhyObject.Move(); // 매쉬이동
	g_PhyObject2.Move();
	g_PhyObject2.Collision(&g_PhyObject);
	g_PhyObject3.Collision(&g_PhyObject);
	g_PhyObject4.Collision(&g_PhyObject);
	g_PhyObject2.Collision(&g_PhyObject4);
	otherPersonDo();
	g_PhyObject4.Move();
	goalkeeper();
	Render();
}

//-----------------------------------------------------------------------------
// 이름: WinMain()
// 기능: 프로그램의 시작점 
//-----------------------------------------------------------------------------
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	// 윈도우 클래스 변수 선언 및 설정 
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
					  GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
					  "D3D Game", NULL };
	// 윈도우 클래스 등록 
	RegisterClassEx(&wc);

	// 윈도우 생성 
	hWnd = CreateWindow("D3D Game", "D3D Game Program",
		WS_OVERLAPPEDWINDOW, 0, 0, 1024, 768,
		GetDesktopWindow(), NULL, wc.hInstance, NULL);
	g_pCamera = new ZCamera;

	// Direct3D 초기화에 성공하면 진행하고, 실패하면 종료한다.
	if (SUCCEEDED(InitD3D(hWnd)) &&       // Direct3D의 초기화도 성공하고 
		SUCCEEDED(InitGeometry()) &&         // 버텍스 버퍼 생성도 성공하고
		SUCCEEDED(InitGeometryTexture()) &&  // 텍스쳐 버텍스 버퍼 생성도 성공하고 
		SUCCEEDED(InitMeshData()) &&         // 기타 게임 데이타 로드 
		SUCCEEDED(InitGeometryMesh()))   // 텍스쳐 버텍스 버퍼 생성도 성공하고
	{
		// 윈도우 출력 
		ShowWindow(hWnd, SW_SHOWDEFAULT);
		UpdateWindow(hWnd);

		// 메시지 루프 시작하기
		MSG msg;
		ZeroMemory(&msg, sizeof(msg));
		while (msg.message != WM_QUIT)
		{
			// 메시자가 있으면 가져 온다. 
			if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
				Program_Action();
		}
	}

	delete g_pCamera;
	UnregisterClass("D3D Game", wc.hInstance);
	return 0;
}



