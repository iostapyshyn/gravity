# Gravity
Simulation of Newton's law of universal gravitation written in C language.

![program window](screenshot.png?raw=true)

## Usage

### Compiling

```
mkdir build
cd build
cmake ..
make
```

Compilation is done using CMake.
GLFW and OpenGL libraries are required to compile this program.

 * [GLFW](http://www.glfw.org) 
 
### Keybindings

 * Z: decrease mass of the next object
 * X: increase mass of the next object
 * C: clear the objects array
 * P: pause
 * D: remove object closest to the mouse cursor
 * A: slow down
 * S: speed up
 * ESC: quit
 
## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
 
