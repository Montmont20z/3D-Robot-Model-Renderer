## 3D Robot Model (OpenGL Fixed Pipeline)

A hierarchical 3D Gundam/robot model renderer built with OpenGL fixed pipeline and Win32 API.

### Technical Highlights
- **Hierarchical Modeling**: Parent-child joint relationships with proper transformation stacking
- **Manual Matrix Math**: Hand-coded rotation, translation, and projection matrices
- **Win32 Integration**: Custom window procedure and message handling
- **Interactive Controls**: Keyboard/mouse input for camera and model manipulation

### What I Learned
- Deep understanding of transformation matrices and coordinate spaces
- Graphics pipeline fundamentals (vertex transformation, rasterization, fragment shading)
- The motivation behind modern programmable shaders
- Trade-offs between immediate mode and retained mode rendering

### Technologies
C++, OpenGL (Fixed Pipeline), Win32 API, GLU, 3D Mathematics

### Why Fixed Pipeline?
This was an academic project focused on understanding graphics fundamentals. 
The experience with manual matrix operations and pipeline stages provided 
excellent foundation for working with modern OpenGL shaders.
