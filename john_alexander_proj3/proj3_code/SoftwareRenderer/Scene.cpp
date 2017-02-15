#include "Scene.h"
#include <iostream>

using namespace std;

extern FrameBuffer fb;
extern Scene scene;

/* set the perspective projection matrix given the following values */
void setPerspectiveProjection(float eye_fov, float aspect_ratio, float nearC, float farC) {
	float f = 1.0f / tan((eye_fov * (M_PI / 180.0f)) / 2.0f);
	scene.perspectiveMat = Matrix(
		f / aspect_ratio, 0, 0, 0,
		0, f, 0, 0,
		0, 0, (nearC + farC) / (nearC - farC), (2 * nearC * farC) / (nearC - farC),
		0, 0, -1, 0);
}


/* set the modelview matrix with the given parameters */
void setModelviewMatrix(float *eye_pos, float eye_theta, float eye_phi) {
	float thetaR = -eye_theta * (M_PI / 180.0f);
	float phiR = -eye_phi * (M_PI / 180.0f);
	float cT = cos(thetaR);
	float sT = sin(thetaR);
	float cP = cos(phiR);
	float sP = sin(phiR);
	Matrix rx(
		1, 0, 0, 0,
		0, cP, -sP, 0,
		0, sP, cP, 0,
		0, 0, 0, 1);
	Matrix ry(
		cT, -sT, 0, 0,
		sT, cT, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
	Matrix trans(
		1, 0, 0, -eye_pos[0],
		0, 1, 0, -eye_pos[1],
		0, 0, 1, -eye_pos[2],
		0, 0, 0, 1);
	scene.modelViewMat = rx * ry * trans;
}

void setViewspaceTransform() {
	scene.viewspaceMat = Matrix(
		fb.width / 2.0f, 0, 0, fb.width / 2.0f,
		0, fb.height / 2.0f, 0, fb.height / 2.0f,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
}

void Scene::rasterize(Triangle oTriangle) {
	Triangle t(
		oTriangle.v[0] * (1.0f / oTriangle.v[0].w),
		oTriangle.v[1] * (1.0f / oTriangle.v[1].w),
		oTriangle.v[2] * (1.0f / oTriangle.v[2].w)
	);

	for (int i = 0; i < 3; i++) {
		Vertex coord = viewspaceMat * t.v[i];
		t.setScreenCoords(i, coord.x, coord.y);
	}

	float xMin = fb.width + 1;
	float xMax = -1;
	float yMin = fb.height + 1;
	float yMax = -1;

	for (int i = 0; i < 3; i++) {
		xMax = max(t.screenCoords[i][0], xMax);
		xMin = min(t.screenCoords[i][0], xMin);
		yMax = max(t.screenCoords[i][1], yMax);
		yMin = min(t.screenCoords[i][1], yMin);
	}

	Vertex v0(t.screenCoords[0][0], t.screenCoords[0][1], 1, 1);
	Vertex v1(t.screenCoords[1][0], t.screenCoords[1][1], 1, 1);
	Vertex v2(t.screenCoords[2][0], t.screenCoords[2][1], 1, 1);
	Vertex f0 = cross(v0, v1);
	Vertex f1 = cross(v1, v2);
	Vertex f2 = cross(v2, v0);
	Vertex p(xMin, yMin, 1, 1);

	float e0 = dot(p, f0);
	float e1 = dot(p, f1);
	float e2 = dot(p, f2);

	for (int i = xMin; i <= xMax && i < fb.width; i++) {
		for (int j = yMin; j <= yMax && j < fb.height; j++) {
			if ((e0 >= 0 &&
				e1 >= 0 &&
				e2 >= 0) ||
				(e0 <= 0 &&
					e1 <= 0 &&
					e2 <= 0)) {
				Vertex currentP(i, j, 1, 1);
				float a0 = heron(v1, v2, currentP);
				float a1 = heron(v0, v2, currentP);
				float a2 = heron(v0, v1, currentP);
				float weight0 = a0 / (a0 + a1 + a2);
				float weight1 = a1 / (a0 + a1 + a2);
				float weight2 = a2 / (a0 + a1 + a2);

				float texX = weight0 * oTriangle.coords[0][0]
					+ weight1 * oTriangle.coords[1][0]
					+ weight2 * oTriangle.coords[2][0];
				float texY = weight0 * oTriangle.coords[0][1]
					+ weight1 * oTriangle.coords[1][1]
					+ weight2 * oTriangle.coords[2][1];

				float zVal = weight0 * t.v[0].z +
					weight1 * t.v[1].z +
					weight2 * t.v[2].z;
				if (fb.updateDepthBuffer(i, j, zVal)) {
					u08* pix = fb.getColorPtr(i, j);
					oTriangle.tex->getNearestColor(texX, texY, pix);

					/*
					pix[0] = (u08)(weight0 * 255);
					pix[1] = (u08)(weight1 * 255);
					pix[2] = (u08)(weight2 * 255);*/
				}
			}
			e0 += f0.y;
			e1 += f1.y;
			e2 += f2.y;
		}
		e0 += -(yMax - yMin + 1) * f0.y + f0.x;
		e1 += -(yMax - yMin + 1) * f1.y + f1.x;
		e2 += -(yMax - yMin + 1) * f2.y + f2.x;
	}
}

bool Scene::shouldDisplay(Triangle t) {
	for (int i = 0; i < 3; i++) {
		if (t.v[i].x > t.v[i].w ||
			t.v[i].x < -t.v[i].w) return false;
		if (t.v[i].y > t.v[i].w ||
			t.v[i].y < -t.v[i].w) return false;
		if (t.v[i].z > t.v[i].w ||
			t.v[i].z < -t.v[i].w) return false;
	}
	return true;
}

Triangle Scene::vertTransform(Triangle t) {
	Triangle ret(t);
	ret.v[0] = perspectiveMat * modelViewMat * t.v[0];
	ret.v[1] = perspectiveMat * modelViewMat * t.v[1];
	ret.v[2] = perspectiveMat * modelViewMat * t.v[2];
	return ret;
}

/* this implements the software rendering of the scene */
void Scene::renderSceneSoftware(void) {
	/* this is the function you will write.  you will need to take the linked list of triangles
	   given by *original_head and draw them to the framebuffer fb */

	fb.zero();

	setViewspaceTransform();

	for (TriangleList* ptr = original_head; ptr; ptr = ptr->next) {
		ptr->t->renderOpenGL();
		Triangle transformed = vertTransform(*(ptr->t));

		if (shouldDisplay(transformed)) {
			rasterize(transformed);
		}
	}

	return;
}