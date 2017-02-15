#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>
#include "Vertex.h"
#include "Triangle.h"

class Matrix {
private:
	float m[16];

public:
	Matrix() {
		for (int i = 0; i < 16; i++) {
			m[i] = 0;
		}
	}

	Matrix(float a1, float a2, float a3, float a4,
		float b1, float b2, float b3, float b4,
		float c1, float c2, float c3, float c4,
		float d1, float d2, float d3, float d4) {
		m[0] = a1;
		m[1] = a2;
		m[2] = a3;
		m[3] = a4;
		m[4] = b1;
		m[5] = b2;
		m[6] = b3;
		m[7] = b4;
		m[8] = c1;
		m[9] = c2;
		m[10] = c3;
		m[11] = c4;
		m[12] = d1;
		m[13] = d2;
		m[14] = d3;
		m[15] = d4;
	}

	void print() {
		for (int i = 0; i < 4; i++) {
			fprintf(stderr, "[%f %f %f %f]\n", m[i * 4], m[i * 4 + 1], m[i * 4 + 2], m[i * 4 + 3]);
		}
	}

	float elem(int i, int j) {
		return m[i * 4 + j];
	}

	friend Matrix operator!(const Matrix& x);
	friend Matrix operator*(const Matrix& x, const Matrix& y);
	friend Matrix operator*(float x, const Matrix& y);
	friend Vertex operator*(const Matrix& x, const Vertex& y);
};

Vertex cross(Vertex a, Vertex b);
float dot(Vertex a, Vertex b, bool useThree = true);
bool insideTriangle(float x1, float y1, float x2, float y2, float x3, float y3, float x, float y);
float heron(Vertex p1, Vertex p2, Vertex p3);

#endif