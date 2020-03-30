# Assignment 2: Model

Out: Oct 31\. Due: Nov 18.

### Introduction

In your second assignment, you will implement a fragment shader and smooth curves and surfaces and add textures to a simple OpenGL renderer on the GPU. This will show you how a remarkable amount of complexity can be quickly achieved in interactive rendering applications.

You are to perform this assignment using C++. To ease your development, we are providing a simple C++ framework to represent the scene, perform basic mathematical calculations, and save your image results. The framework also contain simple test scenes to judge the correctness of your algorithm. These test scenes are encoded in JSON, a readable ASCII format. You can compare to the correct output images we supply. To build the framework, you can use either Visual Studio Express 2013 on Windows or XCode 6 on OS X. We will use external libraries to help us interact with the system, including GLFW for window management and GLEW to access OpenGL functions on Windows.

### Framework Overview

We suggest you use our framework to create your renderer. We have removed from the code all the function implementations your will need to provide, but we have left the function declarations which can aid you in planning your solution. All code in the framework is documented, so please read the documentation for further information. Following is a brief description of the content of the framework.

*   **common.h** includes the basic files from the standard library and contains general utilities such as print messages, handle errors, and enable Python-style foreach loops;
*   **vmath.h** includes various math functions from the standard library, and math types specific to graphics; `vecXX`s are 2d, 3d and 4d tuples, both float and integerers, with related arithmetic options and functions - you should use this type for point, vectors, colors, etc.; `frame3f`s are 3d frames with transformations to and from the frame for points, vectors, normals, etc.; `mat4f` defines a 4x4 matrix with matrix-vector operations and functions to create transform matrices and convert frames
*   **image.h/image.cpp** defines a color image, with pixel access operations and image loading/saving operations
*   **lodepng.h/lodepng.cpp** provide support for the PNG file format
*   **json.h/json.cpp/picojson.h** provide support for the JSON file format
*   **scene.h/scene.cpp** defines the scene data structure and provide JSON scene loading
*   **model.cpp** implements the OpenGL renderer and the interaction (you are only to implement the former): _your texture code goes here_
*   **model_vertex.glsl/model_fragment.glsl**: vertex and fragment shaders: _your fragment shader code goes here_
*   **tesselation.h/tesselation.cpp**: implements smooth curves and surfaces: _your curve/surface code goes here_

In this homework, scenes are becoming more complex. A `Scene` is comprised of a `Camera`, a list of `Mesh`es and a list of `Light`s. The `Camera` is defined by its frame, the size and distance of the image plane and the focus distance (used for interaction). Each `Mesh` is a collection of triangles and quads, lines and splies, centered with respect to its frame, and colored according to a Blinn-Phong `Material` with diffuse, specular coefficients. For triangles and quads, each `Mesh` is represented as an indexed polygonal mesh, with vertex position normals and texture coordinates (for now, please ignore this last vector). For lines and splines, a `Mesh` is a collection of `line` and `spline` segments, with color disabled. Each `line` segment is a reference to the two end points of the segment, just like indexed meshes. Each `spline` segment is the reference to the four vertices of the Bezier. We packed both surfaces and curves into one structure to ease coding, even if this is clearly bad programmaing practice. Each `Light` is a point light centered with respect to its frame and with given intensity. The scene also includes the background color, the ambient illumination, the image resolution and the samples per pixel.

We provide very simple interaction for your viewer. Clicking and moving the mouse lets you rotate the model. If you want to save, please restart the program to avoid issues, and press `s`. For debugging purposes, it is helpful to see face edges. You can do so with wireframing, enabled with `w`.

Since we perform a lot of computation, we suggest you compile in `Release` mode. You `Debug` mode only when deemed stricly necessary. You can also modify the scenes, including the amount of samples while debugging.

### Requirements

You are to implement the code left blank in `model_fragment.glsl` for fragment shading and `tesselation.cpp` for curves and surfaces. You will implement these features.

1.  Compute fragment colors (`shade_fragment.glsl`). Compute fragment colors by implementing ambient light and Blinn-Phong shading with points lights. All variables are already bound for you and the vertex shader is providing the proper transformed data. This code will look remarkably similar to your raytracer code. Note that for this to work, you have to symlink (or manually copy) the shaders in the test directory.

2.  Bezier curves (`tesselation.cpp#subdivide_bezier`). Implement Bezier subdivision using the De Casteljau algorithm applied recursively to each `bezier` segment. In doing so, ensure that the the vertex shared by the two splines for each subdivision has the same index. Implement subdivision for position only, then call the `smooth_tangent` function. Ignore texture coordinates.

3.  Catmull-Clark subdivision (`tesselation.cpp#subdivide_catmullclark`). Implement recursive Catmull-Clark subdivision with the simplified algorithm given in the class. Implement subdivision only for positions `pos`. You will compute normals with either `facet_normals` or `smooth_normals` based on the `smooth` parameter. To simplify development, we have given you a class to compute unique edges `EdgeMap`. To use it, add all triangles and quads, and then look up the edge index with `EdgeMap::edge_index`. To test your mesh, use `facet_normals` right before returning the mesh. For triangles, split them in three quads still using the edge vertices and face center.

4.  Normal smoothing (`tesselation.cpp#smooth_normals`). Implement normal smoothing with the pseudocode given in class. Add normal smoothing to the subdivision code.

5.  Textures.

    1.  Textures are loded into OpenGL for you and texture coordinates are already specified. You only have to implement the code in the fagment shader for it.
    2.  Diffuse and Specular texturing (`model_fragment.glsl`). Compute diffuse color `kd` by multiply the material diffuse response `material_kd` by the value looked up in the texture `material_kd_txt` if the texture is enabled `material_kd_txt_on` (if not, just use the material color). Do the same for the specular color. Use these values to compute lighting.
    3.  Normal mapping (`model_fragment.glsl`). If enabled, copy normal values directly from the normal map. Normals are encoded scaled ans shifted, so get them from the texture by multiplying by 2, subtracting 1 and normalizing them.

### OpenGL and GLSL

OpenGL/GLSL is a large API for programming interactive graphics applications. It is used on all platforms making it the lingua franca of interactive graphics. It is a large API with a design comprised by backwards compatibility. Furthermore, changes to the API in recent years make writing code and getting help from the web cumbersome. To try to be as compatible as possible, we use OpenGL 2.1 / GLSL 1.20, an old version that should run on all desktops and laptops today.

We strongly suggest that you follow the tutorial given in the lecture notes rather than attempting to learn all the API. You are not to learn all of it, in fact you should learn the smallest amount possible. The idea is to give you a gentle introduction to OpenGL and make you understand the interplay between CPU/C++ and GPU/GLSL. For OpenGL/GLSL documentation search the web, especially at [www.khronos.org/](http://www.khronos.org/).

### Hints

We suggest to implement the renderer following the steps presented above. To debug subdivision code, you can use the wireframe mode.

Note that the given screenshots might differ slighly from yours, which would be ok. Also on retina screens, your screenshots might be larger than ours.

### Submission

Please sent an email to `pellacini@di.uniroma1.it` with your code as well as the images generated as a .zip file. **Please submit only code and images and do not resubmit the data or binaries.**

### Extra Credit

1.  Implement displacement mapping. Take a heavily subdivided quad and a bump map (greyscale image with heights). Move the vertex position along the vertex normal by a quantity proportional to the bump map. Smooth normals at the end.

2.  Implement hair growing. Take a triangle mesh and grow random hair uniformly on it. For each hair, you need to follow three steps:

    1.  Pick a triangle. For a not so good look, just randomly pick a triangle. For a better distribution, picka triangle proportinal to its area (using `std::discrete_distribution` where the weight are the triangle areas).
    2.  Pick a point on a triangle by selecting baricentric coordinates as _u = 1 - sqrt(r1)_ and _v = r2 * sqrt(r1)_ where _r1_ and _r2_ are random numbers on _[0,1)_.
    3.  Grow hair. For example create a line from the position _p_ to _p + n * l_, where _n_ is the normal and _l_ the hair length (maybe random).