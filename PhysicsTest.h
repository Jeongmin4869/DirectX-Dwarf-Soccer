#pragma once
#include <Eigen/dense>
#include "d3dx9.h"


struct CUSTOMVERTEX
{
	FLOAT x, y, z;    // 3D 좌표값
	DWORD color;      // 버텍스 색상
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

// 텍스쳐 좌표를 가지는 버텍스 구조체 정의
struct TEXTUREVERTEX
{
	D3DXVECTOR3     position;  // 버텍스의 위치
	D3DCOLOR        color;     // 버텍스의 색상
	FLOAT           tu, tv;    // 텍스쳐 좌표 
};

// 위 구조체의 구조를 표현하는 FVF 값 정의
#define D3DFVF_TEXTUREVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)



using namespace std;
using namespace Eigen;

class Particle { //질량, 위치, 속도
public:
	float m; // 질랑 Mess
	Vector3d p, v, a; // 위치, 속도, 가속도
	Vector3d f; // 힘
};
class Spring {
public:
	static float ks; // 스프링상수
	static float r; // 자연상태의 길이
	static float kd;
	int i, j; // i, j는 각각 particle 번호
};
float Spring::ks = 250.0;
float Spring::r = 10;
float Spring::kd = 4.5;
//PDIRECT3DVERTEXBUFFER9  g_pVBTexture2 = NULL; // 텍스쳐 출력용버텍스 버퍼
//LPDIRECT3DTEXTURE9      g_pTexture2 = NULL; // 텍스쳐 로딩용 변수

class PhysicsSystem {
	static const int SIZE = 10;
	LPDIRECT3DVERTEXBUFFER9 _pVB; // 버텍스 버퍼
	LPDIRECT3DINDEXBUFFER9 _pIB; // 인덱스 버퍼
public:
	Particle p[SIZE * SIZE];
	Spring s[2 * (SIZE - 1)*SIZE];
	PhysicsSystem();
	//~PhysicsSystem(){ DeleteDXBuffers();}
	void CreateDXBuffers(LPDIRECT3DDEVICE9 g_pd3dDevice);
	void CreateDXBuffers2(LPDIRECT3DDEVICE9 g_pd3dDevice);
	void DeleteDXBuffers();

	//엔진 내부에서 힘을 께산, 위치계산
	void CalcForce();
	void Move(float dt);
	//DX버퍼를 갱신, 그림 그리기
	void UpdateBuffers();
	void RenderBuffers(LPDIRECT3DDEVICE9 g_pd3dDevice, LPDIRECT3DTEXTURE9 g_pTexture);


};

//dx를 이용하여 VB, IB를 화면에 그린다.
void PhysicsSystem::RenderBuffers(LPDIRECT3DDEVICE9 g_pd3dDevice, LPDIRECT3DTEXTURE9 g_pTexture) {
	//g_pd3dDevice->SetStreamSource(0, _pVB, 0, sizeof(TEXTUREVERTEX));
	g_pd3dDevice->SetIndices(_pIB);

		// 텍스쳐 설정
		g_pd3dDevice->SetTexture(0, g_pTexture);

		// 텍스쳐 출력 환경 설정
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1); // D3DTOP_SELECTARG1 으로 변경해보자
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		// 출력할 버텍스 버퍼 설정
		g_pd3dDevice->SetStreamSource(0, _pVB, 0, sizeof(TEXTUREVERTEX));
		//g_pd3dDevice->SetIndices(_pIB);
		// FVF 값 설정
		g_pd3dDevice->SetFVF(D3DFVF_TEXTUREVERTEX);
		// 육면체
		//g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 12);
		g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, SIZE*SIZE, 0, (SIZE - 1)*SIZE * 2);
		// 텍스쳐 설정 해제
		g_pd3dDevice->SetTexture(0, NULL);
	
}




//물리 엔진에 저장된 정보를 DX buffer에 저장한다
void PhysicsSystem::UpdateBuffers() {
	//정점버퍼
	TEXTUREVERTEX* pVertices;
	if (FAILED(_pVB->Lock(0, 0, (void**)&pVertices, 0))) //  sizeof(vertices)를 0으로 하면 전체 메모리를 가져온다.
		return;
	for (int i = 0; i < SIZE * SIZE; i++) {//0, 1, 2 : x, y z 
		pVertices[i].position[0] = (float)p[i].p[0];
		pVertices[i].position[1] = (float)p[i].p[1];
		pVertices[i].position[2] = (float)p[i].p[2];
		pVertices[i].color = 0x0000ffff; // cyan
		
	}

	
	int index = 0;
	for (int i = SIZE-1; i >-1; i--) {
		for (int j = 0; j < SIZE; j++) {
			pVertices[index].tu = j; pVertices[index].tv = i; 
			index++;
		}
	}
	

	_pVB->Unlock();
}
void PhysicsSystem::DeleteDXBuffers() {
	if (_pVB) {
		_pVB->Release();
		_pVB = NULL;
	}
	if (_pIB) {
		_pIB->Release();
		_pIB = NULL;
	}


}
void PhysicsSystem::CreateDXBuffers(LPDIRECT3DDEVICE9 g_pd3dDevice) {

	HRESULT hr = g_pd3dDevice->CreateVertexBuffer(SIZE*SIZE * sizeof(TEXTUREVERTEX), 0,
		D3DFVF_TEXTUREVERTEX, D3DPOOL_DEFAULT, &_pVB, NULL);
	if (FAILED(hr))
		return;
	hr = g_pd3dDevice->CreateIndexBuffer((SIZE - 1)*SIZE * 2 * 2 * sizeof(unsigned short), 0,
		D3DFMT_INDEX16, D3DPOOL_DEFAULT, &_pIB, NULL);
	if (FAILED(hr))
		return;

	//인덱스 버퍼s=는 한번만 값을 입력하면 변경될 경우가 없다.
	unsigned short *pIndices;
	_pIB->Lock(0, 0, (void**)&pIndices, 0);
	for (int i = 0; i < 2 * (SIZE - 1)*SIZE; i++) { // 스프링정점번호를 이용
		pIndices[i * 2] = s[i].i;
		pIndices[i * 2 + 1] = s[i].j;
	}
	_pIB->Unlock();


}

//트라이앵글 리스트를 사용... 사각형에 꼭지점 6개 필요!
void PhysicsSystem::CreateDXBuffers2(LPDIRECT3DDEVICE9 g_pd3dDevice) {
	
	//버텍스버퍼
	HRESULT hr = g_pd3dDevice->CreateVertexBuffer(SIZE*SIZE * sizeof(TEXTUREVERTEX), 0,
		D3DFVF_TEXTUREVERTEX, D3DPOOL_DEFAULT, &_pVB, NULL);
	if (FAILED(hr))
		return;

	//인덱스버퍼?
	hr = g_pd3dDevice->CreateIndexBuffer((SIZE - 1) * (SIZE - 1) * sizeof(unsigned short), 0,
		D3DFMT_INDEX16, D3DPOOL_DEFAULT, &_pIB, NULL); // (SIZE-1) * (SIZE-1) * 6 ??,,
 	if (FAILED(hr))
		return;

	//인덱스 버퍼s=는 한번만 값을 입력하면 변경될 경우가 없다.
	unsigned short *pIndices;
	_pIB->Lock(0, 0, (void**)&pIndices, 0);

	int index = 0;
	for (int i = 0; i < SIZE-1; i++) { // i는 SIZE -2 까지만 커짐...
		for (int j = 0; j < SIZE-1; j++) { //  j는 SIZE -2 까지만 커짐...
			//s는 SIZE * SIZE -1 의 행렬...
			pIndices[index * 6]     = s[(SIZE-1) * i + j].i;
			pIndices[index * 6 + 1] = s[(SIZE - 1) * i + j].j;
			pIndices[index * 6 + 2] = s[(SIZE - 1) * (i + 1) + j].i;
			
			pIndices[index * 6 + 3] = s[(SIZE - 1) * i + j].j;
			pIndices[index * 6 + 4] = s[(SIZE - 1) * (i + 1) + j].i; 
			pIndices[index * 6 + 5] = s[(SIZE - 1) * (i + 1) + j].j;
			
			index++;
		}
	}

	
	_pIB->Unlock();

}


//각 입자에 대해서 움직임을 계산
void PhysicsSystem::Move(float dt) {
	for (int i = 0; i < SIZE * SIZE; i++) {
		//f = ma, a = f/m
		p[i].a = p[i].f / p[i].m;
		p[i].v += p[i].a * dt;
		p[i].p += p[i].v * dt;
	}
}
//각 입자에 반영되는 힘의 총 합 계산
void PhysicsSystem::CalcForce() {
	//힘의 초기화
	for (int i = 0; i < SIZE * SIZE; i++) {
		//p[i].f = Vector3d(0, -9.8 * p[i].m, 0); // p[i].m == 1 가정. 중력.
		p[i].f = Vector3d(-1,-0.2 , 0); // p[i].m == 1 가정. 중력. 중력이 너무 강해보여서 줄여줬음
	}
	for (int i = 0; i < 2 * (SIZE - 1)*SIZE; i++) {
		int from = s[i].i;
		int to = s[i].j;
		Vector3d dis = p[to].p - p[from].p; // 두 물체 사이의 변위-
		//f = kx
		double x = dis.norm() / s[i].r - 1; // 정상상태보다 몇 배 늘어났는가

		Vector3d force = s[i].ks * x * (dis / dis.norm());
		//damping
		dis.normalize();
		Vector3d damping =
			s[i].kd * ((p[to].v - p[from].v) / s[i].r).dot(dis)*dis;
		force += damping;

		p[from].f += force;
		p[to].f -= force;
	}

	p[SIZE - 1].f = Vector3d(0, 0, 0); //오른밑
	p[SIZE * SIZE -1].f = Vector3d(0, 0, 0); // 오른위
}
PhysicsSystem::PhysicsSystem() { // 생성자
	//DX Buffer 초기화
	_pVB = NULL;
	_pIB = NULL;

	//Particle 기본설정
	Vector3d  vel(0, 0, 0), acc(0, 0, 0), force(0, 0, 0);
	for (int y = 0; y < SIZE; y++) {
		for (int x = 0; x < SIZE; x++) {
			Vector3d pos(x * 10, y * 10, 0);
			p[y*SIZE + x] = { 1, pos, vel, acc,force };
		}
	}
	
	//가로방향 spring 기본 설정
	//y행 x 열
	int index = 0;
	for (int y = 0; y < SIZE; y++) {
		for (int x = 0; x < SIZE - 1; x++) {
			s[index].i = y * SIZE + x;
			s[index].j = y * SIZE + x + 1;
			index++;
		}
	}

	//세로방향 spring 기본 설정
	for (int y = 0; y < SIZE - 1; y++) {
		for (int x = 0; x < SIZE; x++) {
			s[index].i = y * SIZE + x;
			s[index].j = y * SIZE + x + SIZE;
			index++;
		}
	}



}
