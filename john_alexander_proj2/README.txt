John Alexander
8327934
jta00@umail.ucsb.edu

I have not made any changes to the Visual Studio solution that would cause it to compile differently. Locally, I have been running it from "Developer Command Prompt for VS2015" as follows:

>msbuild MazeViewer.sln
>Debug\MazeViewer.exe maze1.txt

As with th initial code, pressing space switches between software and OpenGL rendering modes. The game starts in OpenGL rendering mode.

As described in the assignment, triangles are clipped if any of their vertices are outside the screen. The triangles are initialized with random colors, and both triangles in each square are the same color.

As expected, debugging took the most time during this project. Frequently, my implementation of the functions was correct, but I had messed up an earlier call that was intended to test them and got erroneous behavior as a result of that.

One of the most interesting things in this assignment was the massive performance improvement when I implemented the iterative algorithm for checking if a point lies inside a triangle. Using a separate, non-iterative function, the program ran at 2-5 frames per second in software rendering mode, but with the iterative method, my framerate immediately jumped past thirty FPS. This large difference also probably had something to do with the high resolution of my monitor.
