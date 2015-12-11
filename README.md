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

In the a traditional forward renderer, for each fragment we calculate the light contribution from each light in the scene. This is essentially like the culling stage of our Forward+ renderer failing to cull any lights from any of the tiles. We see a massive performance gain when using the Forward+ technique vs. the Forward one. With 1024 lights in the scene (our maximum), a light radius of 10, tile size of 16 pixels squared, and 1080p resolution, we were able to achieve an average framerate of 89.867 frames per second over our 60 second benchmark. Rendering the same scene under the same conditions, Forward rendering only achieved an average of 1.7 frames per second. Below are two videos, one for the Forward renderer and the other for the Forward+, running our benchmark. The Forward+ is able to handle rendering the scene with ease, while the Forward renderer resembles a slide show.

More

![](data/Frame Rates.png "Forward vs. Forward+ Rendering Frame Rate")

## Future Work


## Build Instructions

## Interactive Controls

## Acknowledgements
