# LaClaw Lab

[Português (Brasil)](README.md) | **English**

A graphics library using OOP in **C11** and a 3D claw machine simulator built
with **OpenGL and freeglut**. The arm has a base, shoulder, elbow, wrist and
two articulated fingers. Control the joints, capture three aliens and release
them into the delivery tray. Capture is deterministic; the selected mode sets the time limit and bounce strength.

The visuals are inspired by the **Pizza Planet claw machine from Toy Story**.

Developed for **scenario 3 — Robotic Arm Simulator**, CG 2026/2,
CEFET/RJ, UnED Petrópolis.

The architecture follows the `Geometry`, embedded base struct, methods with
`self` and factory patterns from
[Enz0Moura/primeiro-trabalho-cg](https://github.com/Enz0Moura/primeiro-trabalho-cg/tree/c12a40e4254ea511ad8906883b9ca91b4a429dc0),
with 3D math, a scene graph, destructors and explicit memory ownership.

## Build and run

Dependencies: a C11 compiler, Make, and OpenGL and freeglut headers/libraries.
On CachyOS/Arch: `sudo pacman -S --needed base-devel freeglut libglvnd`.
A graphical display is required to open the windows; tests do not open a window.

```sh
make lib       # build/libcg.a, independent of the application
make demo      # build build/library_demo
make example   # run the standalone example
make run       # build and run build/prize_machine
make test      # math, states, paths and allocation failures
make sanitize  # the same tests with Clang, ASan and UBSan
make smoke     # full automated graphical session; requires a display
make clean     # remove build/
```

`make` builds both executables. The simulator and example link against the same
`build/libcg.a`. Choose a mode in the menu with the mouse or **1–4**.
To play: click a prize; select a joint with **1–4**;
use **←/→** or **+/−**; **C** closes, **O** opens, **H** shows help.
The right mouse button and wheel control the camera. **R** resets; **D** toggles
debug views. The simulator's interface is in Portuguese; the English manual
explains its labels while preserving the actual names and key bindings.
Drag with the middle mouse button to pan. **F** toggles the free camera:
right-drag looks around and scrolling moves forward or backward. **Home** resets the camera.

## Game modes

| Mode | Time | Collisions |
| --- | --- | --- |
| Sandbox | Unlimited | Gentle bounces for practice |
| Easy (Fácil) | 4 minutes | Light bounces and stronger damping |
| Medium (Médio) | 3 minutes | Moderate bounces |
| Hard (Difícil) | 2 minutes | Strong bounces and weaker damping |

Released aliens bounce off the floor, cabinet and each other. Delivery counts only
once an alien settles directly on the tray, entirely inside it. **M** opens the menu;
choosing a mode starts a new game. The menu and help pause the simulation and timer.
**R** restarts the current mode. Victory or timeout ends the round.

## Documentation

See the [user manual and first delivery walkthrough](docs/USER_MANUAL.md).

Validated on CachyOS Linux, freeglut 3.8.0 and OpenGL 4.6 compatibility/NVIDIA.
Uses conservative AABBs, gravity, bouncing collisions and simplified friction in
a limited region inside the cabinet. Does not implement arm self-collision or physical prize rotation.
