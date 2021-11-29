// PhysicalObj.cpp: implementation of the PhysicalObj class.
//
//////////////////////////////////////////////////////////////////////

#include "PhysicalObj.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


PhysicalObj::PhysicalObj(float x, float y, float z, float weight)
{
	p.x = p.y = p.z = 0;
	v.x = v.y = v.z = 0;
	a.x = a.y = a.z = 0;
	a.y = -.8f;
	clock = 0;
	scale = 50;
	min = max = D3DXVECTOR3(0, 0, 0);
	m = weight;
}

PhysicalObj::~PhysicalObj()
{

}

//추가한 코드
D3DXVECTOR3  PhysicalObj::returnPositionVector() {
	return p;
};

void PhysicalObj::SetPosition(float x, float y, float z)
{
	p.x = x;
	p.y = y;
	p.z = z;
}

void PhysicalObj::SetVelocity(float x, float y, float z)
{
	v.x = x;
	v.y = y;
	v.z = z;
}
void PhysicalObj::AddVelocity(float x, float y, float z)
{
	v.x += x;
	v.y += y;
	v.z += z;
}

void PhysicalObj::SetAcceleration(float x, float y, float z)
{
	a.x = x;
	a.y = y;
	a.z = z;
}

void PhysicalObj::BoundCheck()
{
	float e = 1.0f;
	/*
		if(p.y <= 0) {
			if( fabs(v.y) < 1.0 ) { // stop condition
				p.y = 0;
				v.y = 0;
			} else {
				v.y = (float)fabs(v.y) * e;
			}
		}
		if(p.x < -200) {
			p.x = -200;
			v.x = (float)fabs(v.x) * e;
		}
		if(p.x > 200) {
			p.x = 200;
			v.x = (float)-fabs(v.x) * e;
		}
		if(p.z < -200) {
			p.z = -200;
			v.z = (float)fabs(v.z) * e;
		}
		if(p.z > 200) {
			p.z = 200;
			v.z = (float)-fabs(v.z) * e;
		}
	*/

	if (p.y + min.y < 0) {
		if (fabs(v.y) < 1.0) { // stop condition
			p.y = -min.y;
			v.y = 0;
		}
		else {
			v.y = (float)fabs(v.y) * e;
		}
	}
	if (p.x + min.x < -300) {
		p.x = -300 - min.x;
		v.x = (float)fabs(v.x) * e;
	}
	if (p.x + max.x > 300) {
		p.x = 300 - max.x;
		v.x = (float)-fabs(v.x) * e;
	}
	if (p.z + min.z < -400) {
		p.z = -400 - min.z;
		v.z = (float)fabs(v.z) * e;
	}
	if (p.z + max.z > 400) {
		p.z = 400 - max.z;
		v.z = (float)-fabs(v.z) * e;
	}

}
void PhysicalObj::Move(float current)
{

	if (current == -1) { // defafult

		p.x += v.x + 0.5f*a.x;
		p.y += v.y + 0.5f*a.y;
		p.z += v.z + 0.5f*a.z;

		v.x += a.x;
		v.y += a.y;
		v.z += a.z;
	}

	BoundCheck();

}

void PhysicalObj::Collision(PhysicalObj *target)
{
	D3DXVECTOR3 distance = (p + center) - (target->p + target->center); // 엄밀하게는 scale도 포함!
	float length = D3DXVec3Length(&distance);
	float rsum = radius + target->radius;
	if (rsum > length) { // collision!

		// exit(0); //충돌일때 종료
		D3DXVECTOR3 d = (target->p + target->center) - (p + center); // normal
		D3DXVec3Normalize(&d, &d);

		D3DXVECTOR3 d1 = D3DXVec3Dot(&d, &v) * d;
		D3DXVECTOR3 t1 = v - d1;

		D3DXVECTOR3 d2 = D3DXVec3Dot(&d, &target->v) * d;
		D3DXVECTOR3 t2 = target->v - d2;

		// u1은 첫번째 물체의 충돌 전 속도. u2는 2번 문제의 충돌전 속도
		//(d1*(m1-m2) + 2*m2*d2)/(m1+m2)
		v = (d1 * (m - target->m) + 2 * target->m*d2) / (m + target->m);
		target->v = (d2 * (target->m - m) + 2 * m*d1) / (m + target->m);
	}

}
void PhysicalObj::SetBoundingBox(D3DXVECTOR3 m, D3DXVECTOR3 M)
{
	min = m * scale;
	max = M * scale;
}

void PhysicalObj::SetBoundingSphere(D3DXVECTOR3 c, float r)
{
	center = c * scale;
	radius = r * scale;
}


D3DXMATRIXA16 PhysicalObj::GetWorldMatrix()
{
	D3DXMATRIXA16 matWorld, matScale;
	D3DXMatrixTranslation(&matWorld, p.x, p.y, p.z);
	D3DXMatrixScaling(&matScale, scale, scale, scale);

	D3DXMatrixMultiply(&matWorld, &matScale, &matWorld);
	return matWorld;
}
