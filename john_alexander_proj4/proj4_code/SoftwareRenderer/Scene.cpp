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
	int actualHeight = fb.height - 20;
	scene.viewspaceMat = Matrix(
		fb.width / 2.0f, 0, 0, fb.width / 2.0f,
		0, actualHeight / 2.0f, 0, actualHeight / 2.0f,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
}

void Scene::rasterize(const Triangle& oTriangle) {
	Triangle t(
		oTriangle.v[0] * (1.0f / oTriangle.v[0].w),
		oTriangle.v[1] * (1.0f / oTriangle.v[1].w),
		oTriangle.v[2] * (1.0f / oTriangle.v[2].w)
	);

	for (int i = 0; i < 3; i++) {
		Vertex coord = viewspaceMat * t.v[i];
		t.setScreenCoords(i, coord.x, coord.y);
	}

	Vertex screenCoords[3] = {
		Vertex(t.screenCoords[0][0], t.screenCoords[0][1], 1, 1),
		Vertex(t.screenCoords[1][0], t.screenCoords[1][1], 1, 1),
		Vertex(t.screenCoords[2][0], t.screenCoords[2][1], 1, 1)
	};

	float invW[3] = {
		1.0f / oTriangle.v[0].w,
		1.0f / oTriangle.v[1].w,
		1.0f / oTriangle.v[2].w
	};

	Vertex interpolants[3] = {
		Vertex(oTriangle.coords[0][0] * invW[0], oTriangle.coords[0][1] * invW[0], invW[0], screenCoords[0].x),
		Vertex(oTriangle.coords[1][0] * invW[1], oTriangle.coords[1][1] * invW[1], invW[1], screenCoords[1].x),
		Vertex(oTriangle.coords[2][0] * invW[2], oTriangle.coords[2][1] * invW[2], invW[2], screenCoords[2].x)
	};

	int maxYInd = 0;
	int minYInd = 0;
	for (int i = 1; i < 3; i++) {
		if (screenCoords[i].y > screenCoords[maxYInd].y) maxYInd = i;
		if (screenCoords[i].y < screenCoords[minYInd].y) minYInd = i;
	}
	int otherInd = 3 - maxYInd - minYInd;

	float temp;

	Vertex minToMax = (interpolants[maxYInd] - interpolants[minYInd]) *
		(1.0f / (screenCoords[maxYInd].y - screenCoords[minYInd].y));
	Vertex minToOther = (interpolants[otherInd] - interpolants[minYInd]) *
		(1.0f / (screenCoords[otherInd].y - screenCoords[minYInd].y));
	Vertex otherToMax = (interpolants[maxYInd] - interpolants[otherInd]) *
		(1.0f / (screenCoords[maxYInd].y - screenCoords[otherInd].y));

	Vertex* leftLine;
	Vertex* rightLine;
	Vertex* leftBase = &interpolants[minYInd];
	Vertex* rightBase = &interpolants[minYInd];
	float leftBaseY = screenCoords[minYInd].y;
	float rightBaseY = screenCoords[minYInd].y;

	bool minMaxOnLeft = cross(screenCoords[maxYInd] - screenCoords[minYInd], screenCoords[otherInd] - screenCoords[minYInd]).z < 0;

	if (minMaxOnLeft) {
		leftLine = &minToMax;
		rightLine = &minToOther;
	}
	else {
		leftLine = &minToOther;
		rightLine = &minToMax;
	}

	bool switched = false;

	for (int y = (int)ceil(screenCoords[minYInd].y); y <= screenCoords[maxYInd].y && y >= 0 && y < fb.height; y++) {
		if (!switched && y >= screenCoords[otherInd].y) {
			switched = true;

			if (minMaxOnLeft) {
				rightLine = &otherToMax;
				rightBase = &interpolants[otherInd];
				rightBaseY = screenCoords[otherInd].y;
			}
			else {
				leftLine = &otherToMax;
				leftBase = &interpolants[otherInd];
				leftBaseY = screenCoords[otherInd].y;
			}
		}

		Vertex leftP = *leftBase + (*leftLine * (y - leftBaseY));
		Vertex rightP = *rightBase + (*rightLine * (y - rightBaseY));
		Vertex horizInterpol = (rightP - leftP) * (1.0f / (rightP.w - leftP.w));

		for (int x = (int)leftP.w; x <= rightP.w && x >= 0 && x < fb.width; x++) {
			Vertex currentP = leftP + (horizInterpol * (x - leftP.w));
			float invW = 1.0f / currentP.z;
			currentP.x *= invW;
			currentP.y *= invW;

			if (fb.updateDepthBuffer(x, y, invW)) {
				oTriangle.tex->getInterColor(currentP.x, currentP.y, fb.getColorPtr(x, y));
			}
		}
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

void Scene::clip(TriangleList** head, TriangleList** tail, Triangle t, int axis, int sign) {
	int outside = 0;
	int outsideVerts[3];

	for (int i = 0; i < 3; i++) {
		if (sign == 1 ? t.v[i][axis] > t.v[i].w :
			t.v[i][axis] < -t.v[i].w) {
			outsideVerts[outside] = i;
			outside++;
		}
	}

	Vertex outNormal(
		axis == 0 ? sign * 0.70710678118 : 0,
		axis == 1 ? sign * 0.70710678118 : 0,
		axis == 2 ? sign * 0.70710678118 : 0,
		-0.70710678118
	);

	Vertex inNormal(
		axis == 0 ? -sign * 0.70710678118 : 0,
		axis == 1 ? -sign * 0.70710678118 : 0,
		axis == 2 ? -sign * 0.70710678118 : 0,
		0.70710678118
	);

	if (outside == 3) {
		return;
	}
	else if (outside == 0) {
		addTriangle(head, tail, new Triangle(t));
	}
	else if (outside == 2) {
		int nonOutside = 3 - outsideVerts[0] - outsideVerts[1];
		float d = dot(inNormal, t.v[nonOutside], false);
		float d0 = dot(outNormal, t.v[outsideVerts[0]], false);
		float d1 = dot(outNormal, t.v[outsideVerts[1]], false);
		float a0 = d / (d + d0);
		Vertex newPoint0 = (t.v[nonOutside] * (1 - a0)) + (t.v[outsideVerts[0]] * a0);
		float a1 = d / (d + d1);
		Vertex newPoint1 = (t.v[nonOutside] * (1 - a1)) + (t.v[outsideVerts[1]] * a1);

		Triangle* newTri = new Triangle();

		newTri->v[nonOutside] = t.v[nonOutside];
		newTri->v[outsideVerts[0]] = newPoint0;
		newTri->v[outsideVerts[1]] = newPoint1;
		
		newTri->setCoords(
			nonOutside,
			t.coords[nonOutside][0],
			t.coords[nonOutside][1]
		);
		newTri->setCoords(
			outsideVerts[0],
			t.coords[nonOutside][0] * (1 - a0) + t.coords[outsideVerts[0]][0] * a0,
			t.coords[nonOutside][1] * (1 - a0) + t.coords[outsideVerts[0]][1] * a0
		);
		newTri->setCoords(
			outsideVerts[1],
			t.coords[nonOutside][0] * (1 - a1) + t.coords[outsideVerts[1]][0] * a1,
			t.coords[nonOutside][1] * (1 - a1) + t.coords[outsideVerts[1]][1] * a1
		);

		newTri->setTexture(t.tex);

		addTriangle(head, tail, newTri);
	}
	else {
		int insideVerts[2];
		int outsideVert = outsideVerts[0];
		switch (outsideVert)
		{
		case 0:
			insideVerts[0] = 1;
			insideVerts[1] = 2;
			break;
		case 1:
			insideVerts[0] = 0;
			insideVerts[1] = 2;
			break;
		default:
			insideVerts[0] = 0;
			insideVerts[1] = 1;
			break;
		}

		float d = dot(outNormal, t.v[outsideVert], false);
		float d0 = dot(inNormal, t.v[insideVerts[0]], false);
		float d1 = dot(inNormal, t.v[insideVerts[1]], false);
		float a0 = d / (d + d0);
		Vertex newPoint0 = (t.v[outsideVert] * (1 - a0)) + (t.v[insideVerts[0]] * a0);
		float a1 = d / (d + d1);
		Vertex newPoint1 = (t.v[outsideVert] * (1 - a1)) + (t.v[insideVerts[1]] * a1);

		Triangle* newTri0 = new Triangle();

		newTri0->v[outsideVert] = newPoint0;
		newTri0->v[insideVerts[0]] = t.v[insideVerts[0]];
		newTri0->v[insideVerts[1]] = t.v[insideVerts[1]];

		newTri0->setCoords(
			outsideVert,
			t.coords[outsideVert][0] * (1 - a0) + t.coords[insideVerts[0]][0] * a0,
			t.coords[outsideVert][1] * (1 - a0) + t.coords[insideVerts[0]][1] * a0
		);
		newTri0->setCoords(
			insideVerts[0],
			t.coords[insideVerts[0]][0],
			t.coords[insideVerts[0]][1]
		);
		newTri0->setCoords(
			insideVerts[1],
			t.coords[insideVerts[1]][0],
			t.coords[insideVerts[1]][1]
		);

		Triangle* newTri1 = new Triangle();

		newTri1->v[outsideVert] = newPoint0;
		newTri1->v[insideVerts[0]] = newPoint1;
		newTri1->v[insideVerts[1]] = t.v[insideVerts[1]];

		newTri1->setCoords(
			outsideVert,
			t.coords[outsideVert][0] * (1 - a0) + t.coords[insideVerts[0]][0] * a0,
			t.coords[outsideVert][1] * (1 - a0) + t.coords[insideVerts[0]][1] * a0
		);
		newTri1->setCoords(
			insideVerts[0],
			t.coords[outsideVert][0] * (1 - a1) + t.coords[insideVerts[1]][0] * a1,
			t.coords[outsideVert][1] * (1 - a1) + t.coords[insideVerts[1]][1] * a1
		);
		newTri1->setCoords(
			insideVerts[1],
			t.coords[insideVerts[1]][0],
			t.coords[insideVerts[1]][1]
		);

		newTri0->setTexture(t.tex);
		newTri1->setTexture(t.tex);

		addTriangle(head, tail, newTri0);
		addTriangle(head, tail, newTri1);
	}
}

/* this implements the software rendering of the scene */
void Scene::renderSceneSoftware(void) {
	/* this is the function you will write.  you will need to take the linked list of triangles
	   given by *original_head and draw them to the framebuffer fb */

	fb.zero();

	setViewspaceTransform();

	TriangleList* myHead = NULL;
	TriangleList* myTail = NULL;
	TriangleList* currentHead = NULL;
	TriangleList* currentTail = NULL;

	for (TriangleList* ptr = original_head; ptr; ptr = ptr->next) {
		addTriangle(&currentHead, &currentTail, new Triangle(vertTransform(*(ptr->t))));
	}

	for (int i = 0; i < 3; i++) {
		for (int j = -1; j <= 1; j += 2) {
			for (TriangleList* ptr = currentHead; ptr; ptr = ptr->next) {
				clip(&myHead, &myTail, *(ptr->t), i, j);
			}
			destroyList(&currentHead, &currentTail);
			currentHead = myHead;
			currentTail = myTail;
			myHead = myTail = NULL;
		}
	}

	for (TriangleList* ptr = currentHead; ptr; ptr = ptr->next) {
		rasterize(*(ptr->t));
	}

	destroyList(&currentHead, &currentTail);

	return;
}