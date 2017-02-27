#ifndef VERTEX_H
#define VERTEX_H

#include <stdio.h>

/* represents a vertex in homogenous coordinates */
class Vertex {
	private:
	public:
		float x, y, z, w;

		Vertex() {
			set(0, 0, 0);
		};

		Vertex(float _x, float _y, float _z) {
			set(_x, _y, _z);
		};

		Vertex(float _x, float _y, float _z, float _w) {
			x = _x;		y = _y;		z = _z;		w = _w;
		};

		void set(float _x, float _y, float _z) {
			x = _x;		y = _y;		z = _z;		w = 1;
		};

		/* for debugging */
		void print(void) {
			fprintf(stderr,"[%f %f %f %f]\n", x, y, z, w);
		}

		Vertex operator*(float val) {
			return Vertex(val * x, val * y, val * z, val * w);
		}

		Vertex operator*=(float val) {
			x *= val;
			y *= val;
			z *= val;
			w *= val;
			return *this;
		}

		Vertex operator+(const Vertex& v) {
			return Vertex(x + v.x, y + v.y, z + v.z, w + v.w);
		}

		Vertex operator+=(const Vertex& v) {
			x += v.x;
			y += v.y;
			z += v.z;
			w += v.w;
			return *this;
		}
};

#endif		/* VERTEX_H */s