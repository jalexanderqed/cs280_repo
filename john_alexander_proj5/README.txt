John Alexander
8327934

Mouse to rotate, WASD to move
(I removed the click-and-drag rotation)

   - Press L to switch to light movement mode
     While in this mode, press SPACE to switch lights
     WASD will move the lights according to the player's orientation
   - Press M to switch between SSAO and full rendering mode

I did not implement SSDO because I ran into a bug with GLSL compilation (seriously, I can demonstrate it), and decided enough was enough.

Everything else should work correctly. The scene has three light sources in it, one red, one green, and one blue. Shadows are calculated separately for each one.

The ambient occlusion uses fifteen vectors when running in full rendering mode and thirty in SSAO mode. Using 30 in full rendering mode significantly impacts framerate (at least on my computer).

Some implementation details were drawn from learnopengl.com, but all code is my own.
