# Box-Box Collision Detection

SIMD optimized box box collision detection.

A default cube is located at origin, has xyz coordinates (±1, ±1, ±1). All 3d cubes can be considered as some transformation applied to a default cube. 
In particular, scale on xyz axis first, then rotate, then translation.

This code is a fast way to test if 2 cubes has collision 

## How to use it

```cpp
void test(){
  Cube c1,c2; // create 2 default cubes
  // init c1
  {
    c1.m_rotation = vec4(x,y,z,w);  // set quaternion
    c1.m_translation = vec4(x,y,z,1);  // set translation on xyz axis, last element 1 doesn't matter here
    c1.m_scale = vec4(x,y,z,1); // set scale on xyz axis, last element 1 doesn't matter here
  }
  // init c2
  {
    // ...
  }
  bool collision = c1.Collision(c2); // return true if collision, false no collision
}
```

## Others
1. Matrix inverse is not optimized yet.
2. Not tested, but I think my code should work for all affine transformations, not just scale, rotate, translation.
