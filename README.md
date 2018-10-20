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

 * Dragging mouse creates new object
 * `Z`: Decrease mass of the new object
 * `X`: Increase mass of the new object
 * `C`: Clear the objects array
 * `P`: Pause the simulation
 * `D`: Remove object closest to the mouse cursor
 * `A`: Slow down the simulation
 * `S`: Speed up the simulation
 * `ESC`: Quit

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
