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
// ���� ���� 
ZCamera*				g_pCamera = NULL;	// Camera Ŭ����

HWND hWnd;				// �޽��� �ڽ� ó���� ���ϰ� �ϱ� ���� �ӽ������� ��������ȭ


LPDIRECT3D9             g_pD3D = NULL; // Direct3D ��ü 
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL; // ������ ��ġ (����ī��)


FLOAT xAngle;
FLOAT yAngle;
POINT mouseDownPt;

BOOL					boomed = FALSE; // �� Ȯ��. ���ӿ��� Ȯ��
BOOL					g_bBillBoard = FALSE;
BOOL					g_bWoodTexture = FALSE;
FLOAT					m_fDegree=0.f; // LookAt�� ���� ����
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL; // ���ؽ� ���� 
PDIRECT3DVERTEXBUFFER9  g_pVBTexture = NULL; // �ؽ��� ��¿���ؽ� ����
PDIRECT3DVERTEXBUFFER9  g_pVBBox = NULL; // ����ü �ؽ�ó ��¿� ���ؽ� ����
LPDIRECT3DTEXTURE9      g_pTexture = NULL; // �ؽ��� �ε��� ����
LPDIRECT3DTEXTURE9      g_pTexture2 = NULL; // �ٴ� �ؽ��� �ε��� ����
LPDIRECT3DTEXTURE9      g_pTexture3 = NULL; // ���� �ؽ��� �ε��� ����
LPDIRECT3DTEXTURE9      g_pTexture4 = NULL; // ���� �ؽ��� �ε��� ����2
PhysicsSystem			g_ps;

LPDIRECT3DINDEXBUFFER9	g_pIBBox = NULL; // �ε��� ����
ID3DXMesh*				g_pMesh = NULL; // �޽�
ID3DXMesh*				g_pMesh2 = NULL; //������ �޽�
ID3DXMesh*				g_pMesh3 = NULL; //�Ƹ��� �ȸ�ü

D3DXVECTOR3				playerPosition; // �÷��̾� ������
D3DXVECTOR3				ballPosition; // �� ������
D3DXVECTOR3				otherPersonPosition; // �ٸ� ����� ������

CXFileUtil              g_XFile;   // ��
CXFileUtil              g_XFile2;  //ȣ����
CXFileUtil              g_XFile3;  // �����
CXFileUtil              g_XFile4; // �÷��̾�
CXFileUtil              g_XFile5; // �����?
CXFileUtil              g_XFile6; // �ٸ����
PhysicalObj				g_PhyObject(0, 0, 0, 10); // ��
PhysicalObj				g_PhyObject2(0, 0, 0, 100); // �÷��̾�
PhysicalObj				g_PhyObject3(0, 0, 0, 500); // �����
PhysicalObj				g_PhyObject4(0, 0, 0, 100); // �ٸ� ��� ������Ʈ

FLOAT player_position[3] = { 0,0,0 };// �÷��̾� ������
FLOAT tiger_position[3] = { 0,0,0 }; // ȣ���� ������

// ��������Ʈ ó���� ���� ����ü 
struct SPRITE {
	int spriteNumber;   // ��ü ��������Ʈ �̹��� ��
	int curIndex;       // ���� ����ؾ� �ϴ� ��������Ʈ �ε��� 
	int frameCounter;   // ���� ��������Ʈ�� ����ϰ� ���ӵ� ������ �� 
	int frameDelay;     // ��������Ʈ ���� �ӵ� ������ ���� ������ ������ 
};
// ��������Ʈ ����ü ���� ���� �� �ʱ�ȭ
SPRITE g_Fire = { 15, 0, 0, 2 }; // ���� & ����ü�� �� ��� �ʱ�ȭ 


// Ŀ���� ���ؽ� Ÿ�� ����ü 
/*
struct CUSTOMVERTEX
{
	FLOAT x, y, z;    // 3D ��ǥ��
	DWORD color;      // ���ؽ� ����
};
*/

// Ŀ���� ���ؽ��� ������ ǥ���ϱ� ���� FVF(Flexible Vertex Format) �� 
// D3DFVF_XYZ(3D ���� ��ǥ) ��  D3DFVF_DIFFUSE(���� ����) Ư���� ��������.
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)



// �ؽ��� ��ǥ�� ������ ���ؽ� ����ü ����
struct MYVERTEX
{
	D3DXVECTOR3     position;  // ���ؽ��� ��ġ
	D3DCOLOR        color;     // ���ؽ��� ����
};

struct MYINDEX
{
	WORD	_0, _1, _2;
};

// �� ����ü�� ������ ǥ���ϴ� FVF �� ����
#define D3DFVF_MYVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

//����� ����� �Լ�
void goalkeeper() {
	ballPosition = g_PhyObject.returnPositionVector();
	float dis[3] = {ballPosition.x,0,ballPosition.z- 280 };
	float length = sqrtf(dis[0] * dis[0] + dis[1] * dis[1] + dis[2] * dis[2]);
	if (boomed && ballPosition.z < 160 && length < 220) {
		if (dis[0] < 40 || dis[0] > -40) {
			//�� ��쿡�� ����������! ( �ڿ��� �� ��� �Ű澲�� �ʴ´�)
			if (dis[0] > 60) dis[0] = 60;
			if (dis[0] < -60) dis[0] = -60;
			g_PhyObject3.SetPosition(dis[0], 0, 280);
		//MessageBox(hWnd, "BOOM", "BOOM", MB_OK);
			boomed = false;
		}
		//if���� ���� �ɰ�� ���� ����
		//MessageBox(hWnd, "��������", "����!", MB_OK);
	}
	else {
		boomed = true;
		//MessageBox(hWnd, "BOOM", "BOOM", MB_OK);
	}
	//���� ����ۺ��� �տ����� ���(Z�� �� �������ϰ��)
	//�� ������ �Ÿ��� �����Ÿ����� ���������
	//���� x��ǥ�� ���� ������� x��ǥ�� �ٲ۴�.

}

//�÷��̾ �ƴ� ����� �Լ�
void otherPersonDo(){
	ballPosition = g_PhyObject.returnPositionVector();
	otherPersonPosition = g_PhyObject4.returnPositionVector();
	
	float dis[3] = { ballPosition.x - otherPersonPosition.x,
		 ballPosition.y - otherPersonPosition.y ,
		 ballPosition.z - otherPersonPosition.z };

	float length = sqrtf(dis[0] * dis[0] + dis[1] * dis[1] + dis[2] * dis[2]);
	
	//���� ������� ����. LookAt
	m_fDegree = atan2f((float)dis[2], (float)dis[0]) * 180 / 3.1415f;

	if (length < 180 && length > 75 ){// && length > 100) {

		if (otherPersonPosition.x < ballPosition.x) // �ȿ����̴°�!
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

	// ���� ��ġ�� �Ĵٺ���.
	//���� �Ÿ��̻� ���� �ٰ�����
	//�������� õõ�� ������ ���� ����. 
	//�־����� ���߰ų� �ٽ� ������ �ڸ��� ���ư���
}

//-----------------------------------------------------------------------------
// �̸�: InitD3D()
// ���: Direct3D �ʱ�ȭ 
//-----------------------------------------------------------------------------
HRESULT InitD3D(HWND hWnd)
{
	// Direct3D ��ü ���� 
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	// ��ġ ������ ����Ÿ �غ�

	D3DPRESENT_PARAMETERS d3dpp;         // ��ġ ������ ���� ����ü ���� ����

	ZeroMemory(&d3dpp, sizeof(d3dpp));                  // ����ü Ŭ����
	d3dpp.BackBufferWidth = 1024;               // ���� �ػ� ���� ����
	d3dpp.BackBufferHeight = 768;               // ���� �ػ� ���� ���� 
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;   // ���� ���� ���� 
	d3dpp.BackBufferCount = 1;                 // ����� �� 
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;  // ���� ��� ����
	d3dpp.hDeviceWindow = hWnd;              // ������ �ڵ� ���� 
	d3dpp.Windowed = true;              // ������ ���� ���� �ǵ��� �� 
	d3dpp.EnableAutoDepthStencil = true;              // ���Ľ� ���۸� ����ϵ��� �� 
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;      // ���Ľ� ���� ���� ���� 


	// D3D��ü�� ��ġ ���� �Լ� ȣ�� (����Ʈ ����ī�� ���, HAL ���,
	// ����Ʈ���� ���ؽ� ó����� ����)
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp, &g_pd3dDevice)))
	{
		return E_FAIL;
	}

	// ���� ��ġ�� ���������� �����Ǿ���.

	// zbuffer ����ϵ��� ����
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	// �⺻�ø� ����
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	return S_OK;
}

//-----------------------------------------------------------------------------
// �̸�: InitMeshData()
// ���: Mesh�� ���õ� ���� �����͸� �ʱ�ȭ �Ѵ�. 
//-----------------------------------------------------------------------------
HRESULT InitMeshData()
{

	SetupViewProjection();

	g_XFile.XFileLoad(g_pd3dDevice, "./images/ball.x");// �ҽ��ڵ尡 �ִ� �۾�����/images/tiger.x
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

	//��
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

	//�÷��̾�
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
// �̸�: InitGeometry()
// ���: ���ؽ� ���۸� ������ �� ���ؽ��� ä���. 
//-----------------------------------------------------------------------------
HRESULT InitGeometry()
{
	g_ps.CreateDXBuffers2(g_pd3dDevice);
	//g_ps.CreateDXBuffers2(g_pd3dDevice);
	// ���ؽ� ���ۿ� ���� ���ؽ� �ڷḦ �ӽ÷� �����. 
	CUSTOMVERTEX vertices[] =
	{
		{ -300.0f,  0.0f, 0.0f, 0xff00ff00, }, // x�� ������ ���� ���ؽ� 
		{  300.0f,  0.0f, 0.0f, 0xff00ff00, },

		{ 0.0f, 0.0f, -300.0f, 0xffffff00, },  // z�� ������ ���� ���ؽ�
		{ 0.0f, 0.0f,  300.0f, 0xffffff00, },

		{ 0.0f, 0.0f, 0.0f, 0xffff0000, },  // y�� ������ ���� ���ؽ�
		{ 0.0f,  100.0f, 0.0f, 0xffff0000, },

		{   0.0f, 50.0f, 0.0f, 0xffff0000, },  // �ﰢ���� ù ��° ���ؽ� 
		{ -50.0f,  0.0f, 0.0f, 0xffff0000, },  // �ﰢ���� �� ��° ���ؽ� 
		{  50.0f,  0.0f, 0.0f, 0xffff0000, },  // �ﰡ���� �� ��° ���ؽ� 
	};

	// ���ؽ� ���۸� �����Ѵ�.
	// �� ���ؽ��� ������ D3DFVF_CUSTOMVERTEX ��� �͵� ���� 
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(9 * sizeof(CUSTOMVERTEX),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &g_pVB, NULL)))
	{
		return E_FAIL;
	}

	// ���ؽ� ���ۿ� ���� �� �� ���ؽ��� �ִ´�. 
	VOID* pVertices;
	if (FAILED(g_pVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0)))
		return E_FAIL;
	memcpy(pVertices, vertices, sizeof(vertices));
	g_pVB->Unlock();

	return S_OK;
}




//-----------------------------------------------------------------------------
// �̸�: InitGeometryTexture()
// ���: �ؽ��� ����� ���� ���ؽ� ���۸� ������ �� ���ؽ��� ä���. 
//-----------------------------------------------------------------------------
HRESULT InitGeometryTexture()
{
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	// �ؽ��� �ε� 
	if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, "./Images/lake.bmp", &g_pTexture))) //���
	{
		return E_FAIL;
	}
	if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, "./Images/green.bmp", &g_pTexture2))) //�ٴ�
	{
		return E_FAIL;
	}
	if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, "./Images/tree02S.dds", &g_pTexture3))) //����
	{
		return E_FAIL;
	}
	if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, "./Images/tree35S.dds", &g_pTexture4))) //����2
	{
		return E_FAIL;
	}

	// ���ؽ� ���� ���� 
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(4 * sizeof(TEXTUREVERTEX), 0,
		D3DFVF_TEXTUREVERTEX, D3DPOOL_DEFAULT, &g_pVBTexture, NULL)))
	{
		return E_FAIL;
	}	


	// ���ؽ� ���� ����  //�̹���

	TEXTUREVERTEX* pVertices;
	if (FAILED(g_pVBTexture->Lock(0, 0, (void**)&pVertices, 0)))
		return E_FAIL;
	pVertices[0].position = D3DXVECTOR3(-50, 100, 100);  // ���ؽ� ��ġ 
	pVertices[0].color = 0xffffffff;                  // ���ؽ� ���� �� ���� 
	pVertices[0].tu = 0.0f;                        // ���ؽ� U �ؽ��� ��ǥ 
	pVertices[0].tv = 0.0f;                        // ���ؽ� V �ؽ��� ��ǥ 

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
// �̸�: InitGeometryMesh()
// ���: Mesh�� �̿��Ͽ� ���� �� �ε��� ���۸� ä���.
// ����: �ؽ�ó�� ����Ǿ� ���� �ʴ�.
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


	
	// ���ؽ� ���� ����
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

	//���
	// ���ؽ� ���� ���� 
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

	// �ε��� ���� ����
	MYINDEX *pIndices;
	g_pMesh->LockIndexBuffer(0, (void**)&pIndices);
	//	if(FAILED(g_pIBBox->Lock(0, 0, (void**)&pIndices, 0)))
	//		return E_FAIL;

	WORD indeces[8][3] = {
	{ 0,1,2 },{ 0,2,3 }, // ��
	//{ 4,6,5 },{ 4,7,6 }, // �Ʒ�
	{ 0,3,7 },{ 0,7,4 }, // ��
	{ 1,5,6 },{ 1,6,2 }, // ����
	//{ 3,2,6 },{ 3,6,7 }, // ��
	{ 0,4,5 },{ 0,5,1 }  // ��
	};

	for (int i = 0; i < 8; i++) {
		pIndices[i]._0 = indeces[i][0];
		pIndices[i]._1 = indeces[i][1];
		pIndices[i]._2 = indeces[i][2];
	}
	g_pMesh->UnlockIndexBuffer();
	//g_pIBBox->Unlock();

	
	LPDWORD attBuf = NULL; // LPDWORD = uunsigned long *
	g_pMesh->LockAttributeBuffer(0, &attBuf);//attBuf�� ���� 12�� �迭.
	
	//if (FAILED(g_pIBBox->Lock(0, 0, (void**)&pIndices, 0)))
	//	return E_FAIL;

	for (int i = 0; i < 8; i++)
		attBuf[i] = 0;
	//for(int i=6; i<12; i++)
	//		attBuf[i] = 1;

	g_pMesh->UnlockAttributeBuffer();
	//g_pIBBox->Unlock();
	

	//����ü
	// ���ؽ� ���� ���� 
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

	// �ε��� ���� ����
	MYINDEX *pIndices2;
	g_pMesh2->LockIndexBuffer(0, (void**)&pIndices2);
	//	if(FAILED(g_pIBBox->Lock(0, 0, (void**)&pIndices, 0)))
	//		return E_FAIL;

	WORD indeces2[12][3] = {
		{ 0,1,2 },{ 0,2,3 }, // ��
		{ 4,6,5 },{ 4,7,6 }, // �Ʒ�
		{ 0,3,7 },{ 0,7,4 }, // ��
		{ 1,5,6 },{ 1,6,2 }, // ����
		{ 3,2,6 },{ 3,6,7 }, // ��
		{ 0,4,5 },{ 0,5,1 }  // ��
	};

	for (int i = 0; i < 12; i++) {
		pIndices2[i]._0 = indeces2[i][0];
		pIndices2[i]._1 = indeces2[i][1];
		pIndices2[i]._2 = indeces2[i][2];
	}
	g_pMesh2->UnlockIndexBuffer();
	//	g_pIBBox->Unlock();

	LPDWORD attBuf2 = NULL; // LPDWORD = uunsigned long *
	g_pMesh2->LockAttributeBuffer(0, &attBuf2);//attBuf�� ���� 12�� �迭.
	for (int i = 0; i < 12; i++)
		attBuf2[i] = 0;
	//for(int i=6; i<12; i++)
	//		attBuf[i] = 1;

	g_pMesh2->UnlockAttributeBuffer();


	
	//�ȸ�ü
	float vertices2[6][3] = {
		{ 0,10,0}, // �� ��

		{ -5,0,5 },
		{ 5,0,5 },
		{ 5,0,-5 },
		{ -5,0,-5 }, // �߰��׸�

		{ 0,-10,0 } //�� ��
	};

	// ���ؽ� ���� ���� 
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

	// �ε��� ���� ����
	MYINDEX *pIndices3;
	g_pMesh3->LockIndexBuffer(0, (void**)&pIndices3);
	//	if(FAILED(g_pIBBox->Lock(0, 0, (void**)&pIndices, 0)))
	//		return E_FAIL;

	WORD indeces3[8][3] = { // ���ǰ���
		{0,1,2},{0,2,3},{0,3,4},{0,1,4}, // �� ���ü
		{5,1,2},{5,2,3},{5,3,4},{5,1,4} // �� ���ü
	};

	for (int i = 0; i < 8; i++) {
		pIndices3[i]._0 = indeces3[i][0];
		pIndices3[i]._1 = indeces3[i][1];
		pIndices3[i]._2 = indeces3[i][2];
	}
	g_pMesh3->UnlockIndexBuffer();
	//	g_pIBBox->Unlock();

	LPDWORD attBuf3 = NULL; // LPDWORD = uunsigned long *
	g_pMesh3->LockAttributeBuffer(0, &attBuf3);//attBuf�� ���� 12�� �迭.
	for (int i = 0; i < 8; i++)
		attBuf3[i] = 0;
	//for(int i=6; i<12; i++)
	//		attBuf[i] = 1;

	g_pMesh3->UnlockAttributeBuffer();



	return S_OK;
}


//-----------------------------------------------------------------------------
// �̸�: Cleanup()
// ���: �ʱ�ȭ�Ǿ��� ��� ��ü���� �����Ѵ�. 
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


	if (g_pd3dDevice != NULL)    // ��ġ ��ü ���� 
		g_pd3dDevice->Release();

	if (g_pD3D != NULL)          // D3D ��ü ���� 
		g_pD3D->Release();

	if (g_pMesh != NULL)
		g_pMesh->Release();

	if (g_pMesh2 != NULL)
		g_pMesh2->Release();

	if (g_pMesh3 != NULL)
		g_pMesh3->Release();
}



//-----------------------------------------------------------------------------
// �̸�: SetupViewProjection()
// ���: �� ��ȯ�� �������� ��ȯ�� �����Ѵ�. 
//-----------------------------------------------------------------------------
VOID SetupViewProjection()
{
	playerPosition = g_PhyObject2.returnPositionVector();
	//ī�޶���ġ�� �ٶ󺸴� ������ �����ؾ���!
	// �� ��ȯ ���� 
	D3DXVECTOR3 vEyePt(playerPosition[0], 250.0f, playerPosition[2] - 600);
	//D3DXVECTOR3 vEyePt(100.0f, 400.0f, -400.0f);    // ī�޶��� ��ġ 
	//D3DXVECTOR3 vLookatPt(playerPosition[0],20.0f, playerPosition[2] + 200);       // �ٶ󺸴� ���� 
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);       // �ٶ󺸴� ���� 
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);          // ������ ���� 
	D3DXMATRIXA16 matView;                           // �亯ȯ�� ��Ʈ���� 
	// �� ��Ʈ���� ���� 
	//D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
	// Direct3D ��ġ�� �� ��Ʈ���� ���� 
	//g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

	// �������� ��ȯ ���� 
	D3DXMATRIXA16 matProj;   // �������ǿ� ��Ʈ���� 
	// �������� ��Ʈ���� ���� 
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 1000.0f);
	// Direct3D ��ġ�� �������� ��Ʈ���� ���� 
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);

	/// ī�޶� �ʱ�ȭ

	g_pCamera->SetView(&vEyePt, &vLookatPt, &vUpVec);
	D3DXMATRIXA16*	pmatView = g_pCamera->GetViewMatrix();		// ī�޶� ����� ��´�.
	g_pd3dDevice->SetTransform(D3DTS_VIEW, pmatView);			// ī�޶� ��� ����
}


// ������ �̸� ���� ������ ���ϴ�.
D3DCOLORVALUE black = { 0, 0, 0, 1 };
D3DCOLORVALUE dark_gray = { 0.2f, 0.2f, 0.2f, 1.0f };
D3DCOLORVALUE gray = { 0.5f, 0.5f, 0.5f, 1.0f };
D3DCOLORVALUE red = { 1.0f, 0.0f, 0.0f, 1.0f };
D3DCOLORVALUE white = { 1.0f, 1.0f, 1.0f, 1.0f };
VOID SetupLight()
{
	D3DLIGHT9 light;                         // Direct3D 9 ���� ����ü ���� ����

	ZeroMemory(&light, sizeof(D3DLIGHT9));
	light.Type = D3DLIGHT_DIRECTIONAL;       // ���� Ÿ���� �𷺼ųη� ����
	light.Diffuse = white;                   // ������ �� ����
	light.Specular = white;
	light.Direction = D3DXVECTOR3(10, -10, 10);       //  ������ ���� (�����ϴ� ����) 
													  //light.Direction = D3DXVECTOR3(20*sin(g_counter*0.01f), -10, 10);       //  ������ ���� (�����ϴ� ����) 
													  //light.Direction = D3DXVECTOR3(10, 25, -40);       //  ������ ���� (�����ϴ� ����) 
	g_pd3dDevice->SetLight(0, &light);      // ����Ʈ ��ȣ ���� (���⿡���� 0��)
	g_pd3dDevice->LightEnable(0, TRUE);     // 0�� ����Ʈ �ѱ�


	// ����Ʈ ��� ����� TRUE�� ��. (�� ����� ���� ��� ����Ʈ ����� ������)
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
	// ���������� �����Ʈ ����Ʈ �ѱ� (ȯ�汤�� ���� ����)
	g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0x00202020);
}

HRESULT ChangeSpriteUV(SPRITE *sp)
{
	// for tree texture
	if (g_bWoodTexture)
		return S_OK;

	float u = (sp->curIndex * 64.0f) / 960.0f;       // ���� �ε����� �̿��� u ��� 
	float u2 = ((sp->curIndex + 1) * 64.0f) / 960.0f;  // ���� �ε���+1�� �̿��� u ���

	TEXTUREVERTEX* pVertices;   // ���ؽ� ���� ���ٿ� ������
	if (FAILED(g_pVBTexture->Lock(0, 0, (void**)&pVertices, 0)))
		return E_FAIL;

	pVertices[0].tu = u;     // u ��ǥ ���� 
	pVertices[0].tv = 0.0f;  // v ��ǥ ���� 

	pVertices[1].tu = u2;
	pVertices[1].tv = 0.0f;

	pVertices[2].tu = u;
	pVertices[2].tv = 1.0f;

	pVertices[3].tu = u2;
	pVertices[3].tv = 1.0f;

	g_pVBTexture->Unlock();

	// ������ ������ �������� ���� ��� 
	if (sp->frameCounter >= sp->frameDelay) {
		sp->curIndex = (sp->curIndex + 1) % sp->spriteNumber; // �ε��� ���� 
		sp->frameCounter = 0;   // ������ ī���� �ʱ�ȭ 
	}
	else  // ���� ������ �ð��� �ȵ� ��� 
		sp->frameCounter++;    // ������ ī���� ���� 

	return S_OK;

}


//-----------------------------------------------------------------------------
// �̸�: Render()
// ���: ȭ���� �׸���.
//-----------------------------------------------------------------------------
VOID Render()
{
	if (NULL == g_pd3dDevice)  // ��ġ ��ü�� �������� �ʾ����� ���� 
		return;

	// �� �� �������� ��ȯ ����
	//SetupViewProjection();

	// �ﰢ���� ��/�� ���� ��� �������ϵ��� �ø� ����� ����.
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// ������ ����. (���� ���� ������ �ƴϰ�, ���ؽ� ��ü�� ������ ����ϵ���)
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	// ����۸� ������ �������� �����.
	// ����۸� Ŭ����
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	// ȭ�� �׸��� ���� 
	if (SUCCEEDED(g_pd3dDevice->BeginScene()))
	{
		///// ���ؽ� ��� 
		// ���ؽ� ���� ���� 
		g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
		g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX); // ���ؽ� ���� ���� 

		D3DXMATRIXA16 matWorld;  // ���� ��ȯ�� ��Ʈ���� ���� 
		D3DXMATRIXA16 matWorld2;

		// �ؽ��� ��� ȯ�� ����
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

		/*
		// �ؽ��� ��� ȯ�� ����
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ); //�ؽ�������
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE ); // ���� ����
		*/


		SetupLight(); // ��ֺ��Ͱ� ���� ������ ������ ���� ����.
		//x ������ �ؽ��� ����
		//g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2); // ���� ���� �׸��� �׸���.
		// X ���� ���
		//D3DXMatrixScaling(&matWorld, 50.0f, 50.0f, 50.0f);

		//��
		matWorld = g_PhyObject.GetWorldMatrix();
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_XFile.XFileDisplay(g_pd3dDevice);
		//g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

		//�÷��̾�
		matWorld = g_PhyObject2.GetWorldMatrix();
		D3DXMatrixRotationY(&matWorld2, 180.0f * (D3DX_PI / 180.0f)); // 180�� ȸ��,,, 
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_XFile4.XFileDisplay(g_pd3dDevice);
	
		//�����
		matWorld = g_PhyObject3.GetWorldMatrix();
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_XFile5.XFileDisplay(g_pd3dDevice);

		//�ٸ����
		matWorld = g_PhyObject4.GetWorldMatrix();
		D3DXMatrixRotationY(&matWorld2, -1 * (m_fDegree + 90) * (D3DX_PI / 180.0f)); 
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_XFile6.XFileDisplay(g_pd3dDevice);

		D3DXMatrixIdentity(&matWorld);
		
		// ���� ����
		g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

		// �ؽ��� ���� (�ؽ��� ������ ���Ͽ� g_pTexture�� ����Ͽ���.)
		//D3DXMatrixTranslation(&matWorld2, 200, 0, 0);
		//g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_pd3dDevice->SetTexture(0, g_pTexture3);

		if (g_bBillBoard) {
			D3DXMATRIXA16*	pmatBill = g_pCamera->GetBillMatrix();		// ī�޶� ����� ��´�.
			pmatBill->_41 = 0;
			pmatBill->_42 = 0;
			pmatBill->_43 = 0;
			g_pd3dDevice->SetTransform(D3DTS_WORLD, pmatBill);			// ī�޶� ��� ����
		}

		if (g_bWoodTexture) {
			g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		}
		else {
			g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE); // TRUE �� ����
			g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		}

	
		// ����� ���ؽ� ���� ����
		g_pd3dDevice->SetStreamSource(0, g_pVBTexture, 0, sizeof(TEXTUREVERTEX));
		
		// FVF �� ����
		g_pd3dDevice->SetFVF(D3DFVF_TEXTUREVERTEX);

		//�ٴ� �ؽ��� ����
		g_pd3dDevice->SetTexture(0, g_pTexture2);
		D3DXMatrixTranslation(&matWorld, 0,-50,-100);
		D3DXMatrixRotationX(&matWorld2,90*(D3DX_PI / 180.0f)); // Z�� 90�� ȸ��
		D3DXMatrixMultiply(&matWorld, &matWorld, &matWorld2);
		D3DXMatrixScaling(&matWorld2, 6.f,6.f,8.f );
		D3DXMatrixMultiply(&matWorld, &matWorld, &matWorld2);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

		//���� �ؽ���
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


		// �ؽ��� ���� ����
		g_pd3dDevice->SetTexture(0, NULL);
		// ���ؽ����� ���ļ¿� ���Ͽ� ���� ����
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

		//���
		//D3DXMatrixIdentity(&matWorld);
		D3DXMatrixTranslation(&matWorld, 0, 60, 350);
		D3DXMatrixScaling(&matWorld2, 2.5f, 2.0f, 1.5f);
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_pMesh->DrawSubset(0);
		D3DXMatrixIdentity(&matWorld);


		//�÷��̾��� �ȸ�ü
		static float angle = 0;
		angle += 0.1f;
		playerPosition = g_PhyObject2.returnPositionVector();
		D3DXMatrixRotationY(&matWorld2, angle);
		D3DXMatrixTranslation(&matWorld, playerPosition[0], playerPosition[1] + 100, playerPosition[2]); // �÷��̾��� �Ӹ� ��. �÷��̾��� x, z, y�� ��������
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_pMesh3->DrawSubset(0);


		//�׵θ�
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



		//Spring. ���
		//���� ���ο��� ���� ����, ��ġ���
		//D3DXMatrixIdentity(&matWorld);
		D3DXMatrixTranslation(&matWorld, -300, 100,300);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_ps.CalcForce();
		g_ps.Move(0.1f);
		//DX���۸� ����, �׸� �׸���
		g_ps.UpdateBuffers();
		g_ps.RenderBuffers(g_pd3dDevice, g_pTexture);

		//���
		D3DXMatrixTranslation(&matWorld, -210, 100, 300);
		D3DXMatrixScaling(&matWorld2, 0.1f, 3.0f, 0.1f);
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_pMesh2->DrawSubset(0);


		// ȭ�� �׸��� �� 
		g_pd3dDevice->EndScene();
	}

	// ������� ������ ȭ������ ������. 
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}



//-----------------------------------------------------------------------------
// �̸� : MsgProc()
// ��� : ������ �޽��� �ڵ鷯 
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//���� �� ������ ��� ���� ��ġ�� ���ư��� �ϴ� �ڵ带 �����սô�
	switch (msg)
	{

	case WM_DESTROY:
		Cleanup();   // ���α׷� ����� ��ü ������ ���Ͽ� ȣ���� 
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		Render();    // ȭ�� ����� ����ϴ� ������ �Լ� ȣ�� 
		ValidateRect(hWnd, NULL);
		return 0;

	case WM_LBUTTONDOWN: // ���콺�Է�
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
			//g_PhyObject2.AddVelocity(0.0f, 3.3f, 0); // ����(?)
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
		//g_pCamera->MoveLocalX(-2.5f);	// ī�޶� ����
		g_pCamera->RotateLocalY(-.01f);
	}
	if (GetAsyncKeyState(VK_RIGHT)) {
		//g_pCamera->MoveLocalX(2.5f);	// ī�޶� ������
		g_pCamera->RotateLocalY(.01f);
	}

	D3DXMATRIXA16*	pmatView = g_pCamera->GetViewMatrix();		// ī�޶� ����� ��´�.
	g_pd3dDevice->SetTransform(D3DTS_VIEW, pmatView);			// ī�޶� ��� ����

}


void Program_Action()
{
	
	// ������ �������� ����, ī�޶� �̵��� ����
	ProcessKey(); //ī�޶��̵�
	g_PhyObject.Move(); // �Ž��̵�
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
// �̸�: WinMain()
// ���: ���α׷��� ������ 
//-----------------------------------------------------------------------------
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	// ������ Ŭ���� ���� ���� �� ���� 
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
					  GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
					  "D3D Game", NULL };
	// ������ Ŭ���� ��� 
	RegisterClassEx(&wc);

	// ������ ���� 
	hWnd = CreateWindow("D3D Game", "D3D Game Program",
		WS_OVERLAPPEDWINDOW, 0, 0, 1024, 768,
		GetDesktopWindow(), NULL, wc.hInstance, NULL);
	g_pCamera = new ZCamera;

	// Direct3D �ʱ�ȭ�� �����ϸ� �����ϰ�, �����ϸ� �����Ѵ�.
	if (SUCCEEDED(InitD3D(hWnd)) &&       // Direct3D�� �ʱ�ȭ�� �����ϰ� 
		SUCCEEDED(InitGeometry()) &&         // ���ؽ� ���� ������ �����ϰ�
		SUCCEEDED(InitGeometryTexture()) &&  // �ؽ��� ���ؽ� ���� ������ �����ϰ� 
		SUCCEEDED(InitMeshData()) &&         // ��Ÿ ���� ����Ÿ �ε� 
		SUCCEEDED(InitGeometryMesh()))   // �ؽ��� ���ؽ� ���� ������ �����ϰ�
	{
		// ������ ��� 
		ShowWindow(hWnd, SW_SHOWDEFAULT);
		UpdateWindow(hWnd);

		// �޽��� ���� �����ϱ�
		MSG msg;
		ZeroMemory(&msg, sizeof(msg));
		while (msg.message != WM_QUIT)
		{
			// �޽��ڰ� ������ ���� �´�. 
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



