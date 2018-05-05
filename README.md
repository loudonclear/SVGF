# CS224 Final Project &mdash; Render Denoising

## Features

 - Separation of albedo, direct lighting, and indirect lighting channels
 - Spatial wavelet filtering -- taking into account normals, color edges, and depth/world-position differences
 - Temporal accumulation -- Accumulation of color from previous frames, and variance estimation (to improve edge blurring).
 - Motion vectors -- reprojection to match up points across frames, even w/ movement.
 - Fast approximate anti-aliasing

## Known Issues

 - Reflections and refractions can be blurry (esp. on flat surfaces)
 -

## TODO

- ** Move the temporary buffers created each frame into member variables **

- ** NaN Values accumulated in history **: leads to growing black squares

- NaN values
