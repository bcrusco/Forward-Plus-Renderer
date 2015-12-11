Forward+ Renderer
================

**University of Pennsylvania, CIS 565: GPU Programming and Architecture, Final Project**

* **Bradley Crusco** (Personal Site: http://bradleycrusco.com, LinkedIn: https://linkedin.com/in/bcrusco
* **Megan Moore** (Personal Site: http://momeg0.wix.com/meganemoore, LinkedIn: https://linkedin.com/in/megan-moore-386076a6)
* Tested on: Windows 10, i7-3770K @ 3.50GHz 16GB, GTX 980 4096MB (Personal Computer)



## Description

![](screenshots/Preview.png "Preview Sponza")


## Features


### Normal Maps

### Depth Buffer

![](screenshots/depth buffer.png "Depth Buffer")

### Lights per Tile (500 Lights, Radius = 30)

![](screenshots/light debug (500 lights - 30r).png "Lights per Tile (500 Lights, Radius = 30)")

### Lights per Tile (1024 Lights, Radius = 30)

![](screenshots/light debug (1024 lights - 30r).png "Lights per Tile (1024 Lights, Radius = 30)")

### Lights per Tile (500 Lights, Radius = 50)

![](screenshots/light debug (500 lights - 50r).png "Lights per Tile (500 Lights, Radius = 50)")


## Performance Analysis

### Forward+ vs. Forward Rendering Frame Rate

In the a traditional forward renderer, for each fragment we calculate the light contribution from each light in the scene. This is essentially like the culling stage of our Forward+ renderer failing to cull any lights from any of the tiles.

![](data/Frame Rates.png "Forward vs. Forward+ Rendering Frame Rate")

## Build Instructions

## Interactive Controls

## Acknowledgements
