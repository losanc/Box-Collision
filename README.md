# Box-Box Collision Detection

SIMD optimized box box collision detection.

A default cube is located at origin, has xyz coordinates (±1, ±1, ±1). All 3d cubes can be considered as some transformation applied to the default cube. 
In particular, scale on xyz axis first, then rotate, then translation.

This code is a fast way to test if 2 cubes has collision.

See [here](detail.md) for the explanation.

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
## How to verify the result

To verify the result, you can use [Blender](https://www.blender.org/). When you open Blender, it will create a default cube automatically. On the right side of screen, there is a `Transform` panel, you can set `Location`, `Rotation`, `Scale`. Remember to change `Rotation Mode` to `Quaternion`. Then you can create another cube, set its property. Then see if they have collision. 

## Others
1. Matrix inverse is not optimized yet.
2. Not tested, but I think my code should work for all affine transformations, not just scale, rotate, translation.
