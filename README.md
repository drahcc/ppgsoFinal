# 🌊 Underwater World — PPGSO Final Project

## 🌟 1. Introduction

This project represents a fully animated **3D Underwater World**, created as a final assignment for the PPGSO course.  
The scene showcases a dynamic marine environment filled with animated fish, corals, seaweed, bubbles, particles, and atmospheric underwater lighting.

The primary goals of the project are to demonstrate:
- Object animation and hierarchical modeling  
- Procedural motion techniques  
- Realistic underwater lighting and shading  
- Use of particle systems for bubbles and dust  
- Smooth, cinematic keyframe camera animation  
- Scene structuring using modern OpenGL and GLSL techniques  

By combining procedural animation, shader effects, and a rich environment, the project delivers an immersive underwater experience inspired by real marine ecosystems.

---

## 🎨 2. Object Behavior and Graphics Effects

### 🌊 Objects in the Scene

**🐟 Fish**
- Animated 3D models moving along procedural curved paths  
- Individual speed variations  
- Sinusoidal tail oscillation for realistic swimming  
- Smooth rotation based on movement direction  

**🌿 Seaweed**
- Hierarchically animated  
- Sinusoidal bending to simulate underwater currents  

**🪨 Rocks & Coral**
- Static 3D meshes  
- Procedurally distributed across the seabed  
- Serve as environmental decoration and collision geometry  

**🫧 Bubbles**
- Particle system with randomized size, speed, lifetime  
- Rising transparency-based particles  
- Simulates underwater air pockets and floating dust  

**🏝 Sand Floor**
- Large textured plane  
- Normal-mapped for additional detail  

**📷 Camera**
- Follows predefined keyframe animation path  
- Smooth interpolation for cinematic movement  

---

## 💡 Graphics and Animation Techniques

### 🔦 Lighting
- Simulated sunlight filtered through the water surface  
- Ambient scattered light for underwater fog  
- Blinn–Phong shading  
- Depth-based color attenuation  

### 🧵 Textures & Materials
- UV-mapped textures for fish, rocks, coral  
- Normal mapping on seabed and coral  
- Transparency for bubbles  

### 🌊 Procedural Animation
- Fish: sinusoidal movement + tail motion  
- Seaweed: wave-based bending animation  
- Bubbles: random upward movement  

### ✨ Particle System
- Used for bubbles and suspended particles (“dust”)  
- Semi-transparent billboards  
- Sorted and rendered back-to-front  

### 🌌 Environment Mapping
- Underwater skybox with caustic effects  

### 🎥 Post-Processing
- Bloom for bright highlights  
- Blue–green underwater color grading  
- Optional depth blur  

---

## 🛠 3. Technical Overview

**Programming Language:** C++  
**Graphics API:** OpenGL 3.3+  

### 📚 Libraries
- GLFW — windowing and input  
- GLAD — OpenGL function loader  
- GLM — math library  
- PPGSO — course framework  

**Development Environment:** CLion  
**Shaders:** GLSL vertex & fragment shaders  
**Architecture:** Scene graph with hierarchical objects  

### 🎬 Animation System
- Keyframe-based camera motion  
- Procedural movement for fish  
- Wave deformation for seaweed  
- Particle simulation for bubbles  

**Expected Duration:** ~2 minutes  
**Difficulty:** Intermediate  

---

## 📝 4. Summary

This project demonstrates both fundamental and intermediate-level computer graphics principles through a fully animated **3D underwater environment**.

It showcases:
- Procedural animation  
- Hierarchical object modeling  
- Texturing and shading  
- Underwater lighting and attenuation  
- Particle systems (bubbles, dust)  
- Smooth cinematic camera animation  
- Environment design and scene composition  

The final result is an immersive underwater scene featuring animated fish, glowing jellyfish, coral ecosystems, dynamic light scattering, and particle effects.  
The project meets all evaluation criteria by integrating animation, shading, particles, lighting, and camera systems into one cohesive and visually engaging demo.

---

## 👤 Author
**Valentin Kitsonov**
PPGSO — Final Project
