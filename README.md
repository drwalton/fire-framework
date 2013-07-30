fire-framework
==============

Framework for particle-based fire &amp; lighting simulation (Dissertation Project, 2013).

Summary
-------

Simple library designed to construct and render small scenes illuminated by particle based fires. 

Usage
-----

The user should first construct a Scene object, to which Renderable objects may be added. These objects will be rendered with a single call to Scene::render().

Libraries
---------

* All rendering is performed by OpenGL. 
* GLEW is employed for portability. 
* FreeGLUT is used to control the main rendering loops and user input. 
* SOIL is used for texture loading. 
* Assimp is used to load 3D meshes.
* GLSW is used to handle shader source files.
