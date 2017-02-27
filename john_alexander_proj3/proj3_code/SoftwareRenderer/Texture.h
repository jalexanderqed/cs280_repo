#ifndef TEXTURE_H
#define TEXTURE_H

#include <windows.h>
#include <atlimage.h>

#include "globals.h"

#if OPENGL_TEST
/* this includes opengl calls in the code */

#include <stdio.h>

#include "gl.h"
#include "glu.h"
#include "glut.h"
#include "gl_ext.h"

#endif


#include <math.h>


/* implements the texture mapping class so that the triangles
   can be textured in our software renderer */
class Texture {
	private:
		int width, height;	/* width and height for the texture */
		u08 *data;			/* contains the color data for the texture */

		/* for opengl only */
		GLuint	tex;		/* handle to opengl texture (only used by opengl */
		void copyTextureData(CImage *image);

	public:
		/* constructor takes the name of the file to use as a texture */
		Texture(char *name);

		/* destructor */
		~Texture() {
			if (data)
				free(data);
		};

		void getFloatColor(int u, int v, float *color) {
			u08 *ptr = data + (((v * width) + u) * 3);
			color[0] = (float) *(ptr);
			color[1] = (float) *(ptr + 1);
			color[2] = (float) *(ptr + 2);
		}

		void getNearestColor(float uf, float vf, u08 *color) {
			int u = (int)roundf(uf * width) % width;
			int v = (int)roundf(vf * height) % height;
			u08 *ptr = data + (((v * width) + u) * 3);
			color[0] = *(ptr);
			color[1] = *(ptr + 1);
			color[2] = *(ptr + 2);
		}

		void getInterColor(float uf, float vf, u08 *color) {
			float u = uf * width;
			float v = vf * height;
			int uMin = (int)floor(u) % width;
			int uMax = (int)ceil(u) % width;
			int vMin = (int)floor(v) % height;
			int vMax = (int)ceil(v) % height;

			u08 *a, *b, *c, *d;
			a = data + (((vMax * width) + uMin) * 3);
			b = data + (((vMax * width) + uMax) * 3);
			c = data + (((vMin * width) + uMin) * 3);
			d = data + (((vMin * width) + uMax) * 3);

			float alpha = u - uMin;
			float beta = v - vMin;

			for (int i = 0; i < 3; i++) {
				color[i] = beta * ((1 - alpha) * a[i] + alpha * b[i]) +
					(1 - beta) * ((1 - alpha) * c[i] + alpha * d[i]);
			}
		}
		
		/* switches between nearest neighbor and bilinear interpolation */
		void switchTextureFiltering(bool flag);


		/* for opengl only */
		void bind(void) {
			glBindTexture(GL_TEXTURE_2D, tex);
			glEnable(GL_TEXTURE_2D);
		};

		void release(void) {
			glDisable(GL_TEXTURE_2D);
		};

};

#endif		/* TEXTURE_H */