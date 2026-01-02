# 3D Robot Model Renderer

A hierarchical 3D Gundam-style robot model built from scratch using OpenGL Fixed Pipeline and Win32 API. This project demonstrates deep understanding of computer graphics fundamentals through manual implementation of transformation matrices, lighting calculations, and hierarchical modeling techniques.



## 🎯 Project Overview

This renderer creates a fully articulated robot model with multiple joints and limbs, all rendered using the OpenGL fixed-function pipeline. Every transformation, from local joint rotations to world-space positioning, is calculated manually without relying on modern shader abstractions.

**Key Achievement**: Built a complete 3D graphics application from first principles, implementing the mathematics and rendering pipeline that modern engines abstract away.

## ✨ Features

### Core Graphics Implementation
- **Hierarchical Transformation System** - Parent-child joint relationships with proper matrix stack management
- **Manual Matrix Mathematics** - Hand-coded 4x4 matrix operations for rotation, translation, scaling, and projection
- **Custom Camera System** - Orbital camera with smooth interpolation and user controls
- **Real-time Lighting** - Phong lighting model with ambient, diffuse, and specular components
- **Material Properties** - Different surface properties for metallic and plastic robot parts

### Interactive Controls
- **Mouse Navigation** - Click and drag to rotate camera around the model
- **Keyboard Controls** - Manipulate individual joints and limbs in real-time
- **Animation System** - Pre-programmed poses and transformation sequences
- **Viewport Management** - Dynamic window resizing with proper aspect ratio handling

### Technical Implementation
- **Win32 Integration** - Native Windows application with custom message handling
- **Double Buffering** - Smooth rendering without screen tearing
- **Depth Testing** - Proper occlusion and z-ordering of model components
- **Optimized Rendering** - Efficient vertex submission and state management

## 🖼️ Screenshots

<table>
  <tr>
    <td><img width="1580" height="1193" alt="Screenshot 2026-01-02 145246" src="https://github.com/user-attachments/assets/31f18734-433d-433f-90cd-c3faa548d2e7" /></td>    
    <td><img width="1576" height="1182" alt="Screenshot 2026-01-02 145303" src="https://github.com/user-attachments/assets/d19615a1-045c-43cb-9130-910b26efd9dc" /></td>
  </tr>
  <tr>
    <td><img width="1585" height="1132" alt="Screenshot 2026-01-02 150130" src="https://github.com/user-attachments/assets/5ccb04fd-e5e3-41e2-9261-74ae7993f114" /></td>
    <td><img width="1582" height="1150" alt="Screenshot 2026-01-02 145519" src="https://github.com/user-attachments/assets/b871dcc9-bf8a-48c0-a569-2291c4aec575" /></td>
  </tr>

</table>

## 🛠️ Technologies Used

- **C++** - Core application logic
- **OpenGL (Fixed Pipeline)** - Graphics rendering and transformation pipeline
- **Win32 API** - Window management and OS integration
- **GLU** - Utility functions for projection and rendering
- **3D Mathematics** - Vector operations, matrix transformations, quaternions

## 🚀 Getting Started

### Prerequisites
```
Visual Studio 2019 or later
OpenGL 1.1+ (included with Windows)
Windows SDK
```

### Build Instructions
1. Clone the repository
```bash
git clone https://github.com/Montmont20z/3D-Robot-Model-Renderer.git
cd 3d-robot-model-renderer
```

2. Open the solution file in Visual Studio
```
RobotRenderer.sln
```

3. Build and run (F5)

### Controls
```
Mouse:
  Right Click + Drag  - Rotate camera around model
  Middle Click + Drag - Pan camera around model
  Scroll Wheel       - Zoom in/out

Keyboard:
  A key is move head to the left side
  D key is move head to the right side
  W key is move body to the left side
  S key is move body to the right side
  Q key is rotate head 360 degree
  E key is rotate body 360 degree
  
  R key moves left hand in yaw direction
  Shift + R key reverse left hand in yaw direction
  T key moves left hand in pitch direction  
  Shift + T key reverse left hand in pitch direction
  Y key moves left elbow 
  Shift + Y key reverse left elbow 
  U key move left wrist 
  Shift + U reverse left wrist
  I key moves left finger
  Shift + I key reverse left finger
  
  F key moves right hand in yaw direction
  Shift + F key reverse right hand in yaw direction
  G key moves right hand in pitch direction  
  Shift + G key reverse right hand in pitch direction
  H key moves right elbow 
  Shift + H key reverse right elbow 
  J key move right wrist 
  Shift + J reverse right wrist
  K key moves right finger
  Shift + K key reverse right finger
  
  B key to launch shield block animation
  
  Press N to grow saber.
  Press again to shrink.
  
  Press M to move the legs.
  Press again to reset to standing pose.

```

## 📚 What I Learned

### Graphics Pipeline Deep Dive
Working with the fixed pipeline provided invaluable insight into how modern graphics APIs work under the hood:

- **Transformation Pipeline** - Understanding model-view-projection matrices and coordinate space conversions
- **Vertex Processing** - How vertices flow through transformation stages before rasterization
- **Lighting Fundamentals** - Material, Lighting, Phong Shading
- **Rasterization Fundamentals** - Triangle setup, interpolation, and fragment generation
- **Fragment Operations** - Depth testing, blending, and the framebuffer
- **Projection** - Orthgraphic Projection vs Pespective Projection

### Mathematical Foundations
Implementing matrix operations manually revealed the elegant mathematics behind 3D graphics:

- Hand-coded matrix multiplication, inversion, and composition
- Gimbal lock challenges and rotation order dependencies  
- Homogeneous coordinates and perspective division
- Interpolation techniques for smooth animations

### Software Architecture
Building a graphics application from scratch taught valuable architectural lessons:

- Managing hierarchical scene graphs with proper encapsulation
- Event-driven programming with Win32 message loops
- State management in immediate-mode rendering

### Historical Context
This experience illuminated why the industry moved to programmable shaders:

- Flexibility limitations of fixed-function pipelines
- Performance advantages of GPU-side computation
- Expressiveness of shader languages for complex effects
- Trade-offs between control and convenience

## 🎓 Technical Highlights

### Hierarchical Modeling System
The robot is constructed using a scene graph where each limb is a child of its parent joint. Transformations propagate correctly through the hierarchy:

```cpp
Robot
├─ Torso
├─ Head
├─ Left & Right Arm
│  ├─ Upper Arm
│  └─ Lower Arm
│  │  |  └─ Palm
|  |  |  |  └─ Finger
└─ Left & Right Legs
```

Each node maintains its local transformation, and the final world position is computed by multiplying all parent transforms.

### Matrix Stack Management
Proper use of OpenGL's matrix stack ensures transformations don't leak between hierarchy levels:

```cpp
glPushMatrix();  // Save parent transform
  glTranslatef(...);  // Position limb
  glRotatef(...);     // Orient limb
  drawLimb();         // Render geometry
  drawChildren();     // Recurse to children
glPopMatrix();   // Restore parent transform
```

### Custom Camera Implementation
The camera system uses spherical coordinates for intuitive orbital controls:

- Smooth interpolation between viewpoints
- Automatic aspect ratio adjustment
- Configurable near/far clipping planes
- Look-at matrix construction

## 🔍 Why Fixed Pipeline?

This was an academic project specifically designed to teach graphics fundamentals by removing modern abstractions. This educational approach provided:

1. **Conceptual Foundation** - Understanding what shaders actually replace
2. **Mathematical Intuition** - Hands-on experience with transformation math
3. **Historical Perspective** - Appreciation for modern graphics APIs
4. **Debugging Skills** - Diagnosing issues without sophisticated tools

The knowledge gained translates directly to modern graphics programming - the math is identical, just executed in different places (CPU vs GPU).

## 🔮 Future Enhancements

If I were to extend this project, interesting additions would include:

- **Skeletal Animation System** - Keyframe interpolation and animation blending
- **Inverse Kinematics** - Procedural limb positioning for natural poses
- **Physics Integration** - Ragdoll effects and collision detection
- **Texture Mapping** - Surface detail through UV coordinates
- **Modern OpenGL Port** - Shader-based implementation for comparison

## 📄 License

This project is open source and available under the MIT License.


---

*This project was developed as part of [Course Name] at [University/Institution] to explore fundamental computer graphics concepts.*
