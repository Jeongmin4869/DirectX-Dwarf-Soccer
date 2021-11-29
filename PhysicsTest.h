#pragma once
#include <Eigen/dense>
#include "d3dx9.h"


struct CUSTOMVERTEX
{
	FLOAT x, y, z;    // 3D ��ǥ��
	DWORD color;      // ���ؽ� ����
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

// �ؽ��� ��ǥ�� ������ ���ؽ� ����ü ����
struct TEXTUREVERTEX
{
	D3DXVECTOR3     position;  // ���ؽ��� ��ġ
	D3DCOLOR        color;     // ���ؽ��� ����
	FLOAT           tu, tv;    // �ؽ��� ��ǥ 
};

// �� ����ü�� ������ ǥ���ϴ� FVF �� ����
#define D3DFVF_TEXTUREVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)



using namespace std;
using namespace Eigen;

class Particle { //����, ��ġ, �ӵ�
public:
	float m; // ���� Mess
	Vector3d p, v, a; // ��ġ, �ӵ�, ���ӵ�
	Vector3d f; // ��
};
class Spring {
public:
	static float ks; // ���������
	static float r; // �ڿ������� ����
	static float kd;
	int i, j; // i, j�� ���� particle ��ȣ
};
float Spring::ks = 250.0;
float Spring::r = 10;
float Spring::kd = 4.5;
//PDIRECT3DVERTEXBUFFER9  g_pVBTexture2 = NULL; // �ؽ��� ��¿���ؽ� ����
//LPDIRECT3DTEXTURE9      g_pTexture2 = NULL; // �ؽ��� �ε��� ����

class PhysicsSystem {
	static const int SIZE = 10;
	LPDIRECT3DVERTEXBUFFER9 _pVB; // ���ؽ� ����
	LPDIRECT3DINDEXBUFFER9 _pIB; // �ε��� ����
public:
	Particle p[SIZE * SIZE];
	Spring s[2 * (SIZE - 1)*SIZE];
	PhysicsSystem();
	//~PhysicsSystem(){ DeleteDXBuffers();}
	void CreateDXBuffers(LPDIRECT3DDEVICE9 g_pd3dDevice);
	void CreateDXBuffers2(LPDIRECT3DDEVICE9 g_pd3dDevice);
	void DeleteDXBuffers();

	//���� ���ο��� ���� ����, ��ġ���
	void CalcForce();
	void Move(float dt);
	//DX���۸� ����, �׸� �׸���
	void UpdateBuffers();
	void RenderBuffers(LPDIRECT3DDEVICE9 g_pd3dDevice, LPDIRECT3DTEXTURE9 g_pTexture);


};

//dx�� �̿��Ͽ� VB, IB�� ȭ�鿡 �׸���.
void PhysicsSystem::RenderBuffers(LPDIRECT3DDEVICE9 g_pd3dDevice, LPDIRECT3DTEXTURE9 g_pTexture) {
	//g_pd3dDevice->SetStreamSource(0, _pVB, 0, sizeof(TEXTUREVERTEX));
	g_pd3dDevice->SetIndices(_pIB);

		// �ؽ��� ����
		g_pd3dDevice->SetTexture(0, g_pTexture);

		// �ؽ��� ��� ȯ�� ����
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1); // D3DTOP_SELECTARG1 ���� �����غ���
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		// ����� ���ؽ� ���� ����
		g_pd3dDevice->SetStreamSource(0, _pVB, 0, sizeof(TEXTUREVERTEX));
		//g_pd3dDevice->SetIndices(_pIB);
		// FVF �� ����
		g_pd3dDevice->SetFVF(D3DFVF_TEXTUREVERTEX);
		// ����ü
		//g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 12);
		g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, SIZE*SIZE, 0, (SIZE - 1)*SIZE * 2);
		// �ؽ��� ���� ����
		g_pd3dDevice->SetTexture(0, NULL);
	
}




//���� ������ ����� ������ DX buffer�� �����Ѵ�
void PhysicsSystem::UpdateBuffers() {
	//��������
	TEXTUREVERTEX* pVertices;
	if (FAILED(_pVB->Lock(0, 0, (void**)&pVertices, 0))) //  sizeof(vertices)�� 0���� �ϸ� ��ü �޸𸮸� �����´�.
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

	//�ε��� ����s=�� �ѹ��� ���� �Է��ϸ� ����� ��찡 ����.
	unsigned short *pIndices;
	_pIB->Lock(0, 0, (void**)&pIndices, 0);
	for (int i = 0; i < 2 * (SIZE - 1)*SIZE; i++) { // ������������ȣ�� �̿�
		pIndices[i * 2] = s[i].i;
		pIndices[i * 2 + 1] = s[i].j;
	}
	_pIB->Unlock();


}

//Ʈ���̾ޱ� ����Ʈ�� ���... �簢���� ������ 6�� �ʿ�!
void PhysicsSystem::CreateDXBuffers2(LPDIRECT3DDEVICE9 g_pd3dDevice) {
	
	//���ؽ�����
	HRESULT hr = g_pd3dDevice->CreateVertexBuffer(SIZE*SIZE * sizeof(TEXTUREVERTEX), 0,
		D3DFVF_TEXTUREVERTEX, D3DPOOL_DEFAULT, &_pVB, NULL);
	if (FAILED(hr))
		return;

	//�ε�������?
	hr = g_pd3dDevice->CreateIndexBuffer((SIZE - 1) * (SIZE - 1) * sizeof(unsigned short), 0,
		D3DFMT_INDEX16, D3DPOOL_DEFAULT, &_pIB, NULL); // (SIZE-1) * (SIZE-1) * 6 ??,,
 	if (FAILED(hr))
		return;

	//�ε��� ����s=�� �ѹ��� ���� �Է��ϸ� ����� ��찡 ����.
	unsigned short *pIndices;
	_pIB->Lock(0, 0, (void**)&pIndices, 0);

	int index = 0;
	for (int i = 0; i < SIZE-1; i++) { // i�� SIZE -2 ������ Ŀ��...
		for (int j = 0; j < SIZE-1; j++) { //  j�� SIZE -2 ������ Ŀ��...
			//s�� SIZE * SIZE -1 �� ���...
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


//�� ���ڿ� ���ؼ� �������� ���
void PhysicsSystem::Move(float dt) {
	for (int i = 0; i < SIZE * SIZE; i++) {
		//f = ma, a = f/m
		p[i].a = p[i].f / p[i].m;
		p[i].v += p[i].a * dt;
		p[i].p += p[i].v * dt;
	}
}
//�� ���ڿ� �ݿ��Ǵ� ���� �� �� ���
void PhysicsSystem::CalcForce() {
	//���� �ʱ�ȭ
	for (int i = 0; i < SIZE * SIZE; i++) {
		//p[i].f = Vector3d(0, -9.8 * p[i].m, 0); // p[i].m == 1 ����. �߷�.
		p[i].f = Vector3d(-1,-0.2 , 0); // p[i].m == 1 ����. �߷�. �߷��� �ʹ� ���غ����� �ٿ�����
	}
	for (int i = 0; i < 2 * (SIZE - 1)*SIZE; i++) {
		int from = s[i].i;
		int to = s[i].j;
		Vector3d dis = p[to].p - p[from].p; // �� ��ü ������ ����-
		//f = kx
		double x = dis.norm() / s[i].r - 1; // ������º��� �� �� �þ�°�

		Vector3d force = s[i].ks * x * (dis / dis.norm());
		//damping
		dis.normalize();
		Vector3d damping =
			s[i].kd * ((p[to].v - p[from].v) / s[i].r).dot(dis)*dis;
		force += damping;

		p[from].f += force;
		p[to].f -= force;
	}

	p[SIZE - 1].f = Vector3d(0, 0, 0); //������
	p[SIZE * SIZE -1].f = Vector3d(0, 0, 0); // ������
}
PhysicsSystem::PhysicsSystem() { // ������
	//DX Buffer �ʱ�ȭ
	_pVB = NULL;
	_pIB = NULL;

	//Particle �⺻����
	Vector3d  vel(0, 0, 0), acc(0, 0, 0), force(0, 0, 0);
	for (int y = 0; y < SIZE; y++) {
		for (int x = 0; x < SIZE; x++) {
			Vector3d pos(x * 10, y * 10, 0);
			p[y*SIZE + x] = { 1, pos, vel, acc,force };
		}
	}
	
	//���ι��� spring �⺻ ����
	//y�� x ��
	int index = 0;
	for (int y = 0; y < SIZE; y++) {
		for (int x = 0; x < SIZE - 1; x++) {
			s[index].i = y * SIZE + x;
			s[index].j = y * SIZE + x + 1;
			index++;
		}
	}

	//���ι��� spring �⺻ ����
	for (int y = 0; y < SIZE - 1; y++) {
		for (int x = 0; x < SIZE; x++) {
			s[index].i = y * SIZE + x;
			s[index].j = y * SIZE + x + SIZE;
			index++;
		}
	}



}
