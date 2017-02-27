John Alexander
8327934
jta00@umail.ucsb.edu

I have not made any changes to the Visual Studio solution that would cause it to compile differently. Locally, I have been running it from "Developer Command Prompt for VS2015" as follows:

>msbuild MazeViewer.sln
>Debug\MazeViewer.exe maze1.txt

As with the initial code, pressing space switches between software and OpenGL rendering modes. The game starts in OpenGL rendering mode. Note that per the assignment, I have modified the OpenGL rendering so that only triangles lying completely inside the scene are drawn.

The triangles drawn the software rendering pipeline should match up almost perfectly with those drawn with OpenGL. There may be a slight amount of vertical stretching or compression because the title bar of the window appears to be included in the height value used by the software rendering but is not included by OpenGL. I have done my best to rectify this by reducing the height value when constructing the matrix that transforms to screen coordinates, but this is probably display-specific.

The massive drop in framerate when using the software rendering is due to the repeated calculation of Barycentric coordinates for each pixel of each triangle. This is as requested in the assignment and will be rectified in the next lab.

As requested, I have implemented bilinear interpolation of the texture values. This can best be seen when comparing the OpenGL rendering of the Mona Lisa (which can be seen from the starting position if the camera is rotated) to the software version. The OpenGL version is quite pixelated while my rendering is much smoother.
