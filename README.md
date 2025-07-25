# Fluid Dynamics Simulator
Yap abt project

## First Implementation
I chose a Smoothed-particle hydrodynamic model for my first implementation because handling the fluid as a set of particles was more familiar to me. This is known as a Lagrangian approach due to the "computational mesh" or fluid being a collection of individual points with their own data that is manipulated according the equations that govern the system. This means its **a** easier to handle free surfaces like breaking waves, **b** we can kinda ignore mass conservation, **c** interactions with other moving bodies are easy, and fluid properties are easy.

**a**) It's particle based so boundaries are defined inherently - so basically it solves itself

**b**) Each particle has the same mass \
No mass enters/exits the system

**c**) Same logic as **a**. These interactions are calculated based on nearby particles

### Understanding the math
Yap abt how I suck at math so other people derived the equations for me and it was still hard to understand

#### Finding density
This is done by looping through all the particles and for each particle **i**, we loop through all the particles again **j** and sum their density contributions, apply some weight to it and multiply by the mass **m** of particle **j**.\
![alt text](image.png)\
**r** is the vector pointing from particle **i** to particle **j** so ||**r**|| is the distance between the two particles.\
\
**W** is the smoothing kernel, which is a type of weighted moving average. This spreads the influence of one particle over some small radius **h**. Particles outside that radius will be ignored. **k** is the kernel constant for the poly6 kernel, which was designed by Müller et al for particle density approximations.\
![alt text](image-1.png)

#### Finding pressure
Since the density at each point is known, the pressure can then be solved for using an approximation of the Tait Equation of State. **k** is the gas constant and **ρ**<sub>0</sub> is the density at equilibrium. This can be linearized as: \
![alt text](image-2.png)


#### Calculating Forces
We want to find the total force for every particle. We do so by summing the force due to pressure, viscosity, and gravity. \
![alt text](image-3.png) \
Once again we loop through all particles **i**, and for each **i**, we loop through all other particles **j** and sum their pressure, viscosity force contributions.
##### Pressure
In our simulation, pressure is responsible for the force that pushes particles from regions of high concentration to regions of low concentration. Mathematically, this is represented as a gradient, whose value at a point gives the direction and magnitude of the maximum rate of increase. So the pressure can be represented by: \
![alt text](image-4.png) \
But we are dealing with a bunch of particles, not a continuous field, so we must adapt it for a discrete field. \
![alt text](image-5.png) \
But this force is not symmetric, which will lead to an unstable fluid. The symmetrical form of the force on particle **i** due to particle **j** is: \
![alt text](image-6.png) \
**W** is now a gradient of the Spiky smoothing kernel, where **h** is the same as earlier.\
![alt text](spiky.png)