#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdlib.h>
#include <atlimage.h>
#include "globals.h"

/* implements the framebuffer. you might want to modify it to add depth buffering and other capabilities */
class FrameBuffer {
	private:
		/* all the buffers are defined from the lower left corner of the screen */
		u08 *color_buffer;		/* color buffer is unsigned bytes buffer size 3*w*h */
		float *depth_buffer;

	public:
		int width, height;

		/* constructor */
		FrameBuffer(int _w, int _h) {
			width = _w;		height = _h;

			/* allocate the buffer */
			color_buffer = (u08 *)malloc(sizeof(u08) * width * height * 3);
			depth_buffer = (float*)malloc(sizeof(float) * width * height);
		};

		/* destructor */
		~FrameBuffer() {
			if (color_buffer)
				free(color_buffer);
		};

		void zero() {
			memset(color_buffer, 0, width * height * 3);
			memset(depth_buffer, -1, width * height * sizeof(float));
		}

		/* get color ptr */
		u08 *getColorPtr(int x, int y) {
			return (color_buffer + ((y * width + x)*3));
		};

		bool updateDepthBuffer(int x, int y, float z) {
			float depthVal = depth_buffer[y * width + x];
			if (isnan(depthVal) || z < depthVal) {
				depth_buffer[y * width + x] = z;
				return true;
			}
			else {
				return false;
			}
		}

		void copyColorBuffer(CImage *image);
		void dumpColorBufferToFile(char *name);
		void dumpToScreen(void);
};

#endif		/* FRAMEBUFFER_H */
