==TODO==

1. Model materials
 * Solids will have per-vertex materials. done!
 * A uniform will contain an array of material properties, and
    each vertex will have an associated material index. The material
    values will be pulled out in the vertex shader, and smoothly passed
    to the frag shader. done! (For Solid.glsl only)
 * Appropriate aspects of these materials will be used in PRT preprocesses. ...not done.

2. Shader pointers
 * Each renderable has a shader of type Shader*.
 * This does not do call the correct virtual functions for the particular
    derived renderable's shader type!
 * Fix this somehow?