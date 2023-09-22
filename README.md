# Assignment Three: Red Tribe and Blue Tribe
### Boid-Monkeys

This assignment applies boid rules on monkey head objects

Run Instructions:
- cd to the debug folder in the assignment structure.
- Open a command prompt in the debug folder
- The following command can be used to run the animation:
    - assignment3_100739773

Scene and Controls:
- Z to zoom in / X to zoom out
- A to move left / D to move right
- W to move up / S to move down

The mesh structure used in assignment one from CSCI3090 is used to load and store the obj files for this assignment. The structure was altered to include vec3s for velocity and position as well as a string to hold the tribe for each monkey.

The ground plane and all of the monkeys are stored in a vector after being loaded into a mesh. When the ground is loaded a vector called ground_vert is added to whenever the ground has a y-value greater than 0.

The functions groupTogether, avoidMonkeys, and matchMotion were used for tribe rules 2 - 5. Tribe rule 1 (the ground collision avoidance) was done using the function groundCollision.

Note: pseudo code from the following site was used to aid in the coding of rules 2-5
http://www.kfish.org/boids/pseudocode.html
