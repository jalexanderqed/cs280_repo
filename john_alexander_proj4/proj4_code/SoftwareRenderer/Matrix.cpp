#include "Matrix.h"

Matrix operator!(const Matrix& x) {
	Matrix res;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			res.m[j * 4 + i] = x.m[i * 4 + j];
		}
	}
	return res;
}

Matrix operator*(float x, const Matrix& y) {
	return Matrix(x * y.m[0], x * y.m[1], x * y.m[2], x * y.m[3],
		x * y.m[4], x * y.m[5], x * y.m[6], x * y.m[7],
		x * y.m[8], x * y.m[9], x * y.m[10], x * y.m[11],
		x * y.m[12], x * y.m[13], x * y.m[14], x * y.m[15]);
}

Matrix operator*(const Matrix& x, const Matrix& y) {
	Matrix res;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				res.m[i * 4 + j] += x.m[i * 4 + k] * y.m[k * 4 + j];
			}
		}
	}
	return res;
}

Vertex operator*(const Matrix& x, const Vertex& y) {
	return Vertex(x.m[0] * y.x + x.m[1] * y.y + x.m[2] * y.z + x.m[3] * y.w,
		x.m[4] * y.x + x.m[5] * y.y + x.m[6] * y.z + x.m[7] * y.w,
		x.m[8] * y.x + x.m[9] * y.y + x.m[10] * y.z + x.m[11] * y.w,
		x.m[12] * y.x + x.m[13] * y.y + x.m[14] * y.z + x.m[15] * y.w);
}

Vertex cross(Vertex a, Vertex b) {
	return Vertex(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x,
		1
	);
}

float dot(Vertex a, Vertex b, bool useThree) {
	if (useThree) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}
	else {
		return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	}
}

bool insideTriangle(float x1, float y1, float x2, float y2, float x3, float y3, float x, float y) {
	Vertex v0(x1, y1, 1, 1);
	Vertex v1(x2, y2, 1, 1);
	Vertex v2(x3, y3, 1, 1);
	Vertex f0 = cross(v0, v1);
	Vertex f1 = cross(v1, v2);
	Vertex f2 = cross(v2, v0);

	Vertex p(x, y, 1, 1);
	if ((dot(p, f0, true) > 0 &&
		dot(p, f1, true) > 0 &&
		dot(p, f2, true) > 0) ||
		(dot(p, f0, true) < 0 &&
			dot(p, f1, true) < 0 &&
			dot(p, f2, true) < 0)) {
		return true;
	}

	return false;
}

float distance(const Vertex& p1, const Vertex& p2) {
	float xDif = p1.x - p2.x;
	float yDif = p1.y - p2.y;
	float zDif = p1.z - p2.z;
	return sqrt(xDif * xDif + yDif * yDif + zDif * zDif);
}

float heron(const Vertex& p1, const Vertex& p2, const Vertex& p3) {
	float l1 = distance(p1, p2);
	float l2 = distance(p2, p3);
	float l3 = distance(p3, p1);
	float p = (l1 + l2 + l3) / 2.0f;
	return sqrt(p * (p - l1) * (p - l2) * (p - l3));
}