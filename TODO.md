==TODO==

1. Model materials
 * Solids will have per-vertex materials. 
 * A uniform will contain an array of material properties, and
    each vertex will have an associated material index. The material
    values will be pulled out in the vertex shader, and smoothly passed
    to the frag shader.
 * Appropriate aspects of these materials will be used in PRT preprocesses.
