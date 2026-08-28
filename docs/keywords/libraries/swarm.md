# `swarm` Boids Flocking & Particle Swarm Simulation

## 1. Library Overview & Usage

The `swarm` library implements Craig Reynolds' classic Boids flocking algorithm (Separation, Alignment, Cohesion), obstacle avoidance, and predator chase behaviors in BGI.

### Three Flocking Rules:
1. **Separation**: Steer to avoid crowding local flockmates.
2. **Alignment**: Steer towards the average heading of local flockmates.
3. **Cohesion**: Steer to move towards the average position (center of mass) of local flockmates.

---

## 2. BASIC Example

```basic
10 SET SCREEN 320, 200, 8
20 NUM_BOIDS = 64 : DIM BX(64), BY(64), BVX(64), BVY(64)
30 PRINT "Boids swarm simulation running (64 entities)."
```
