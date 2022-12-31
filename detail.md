# Hyperplane separation theorem

From the [hyperplane separation theorem](https://en.wikipedia.org/wiki/Hyperplane_separation_theorem), it tells us, if there exists an plane, which separates two cubes, then two cubes have no collision. If such plane exist, we can find a line, project all points inside these two cubes onto this line, we can separate these two sets of projected points. But it's impossible to test all the lines.

# Cube Transformation
As we know, all 3d cubes are transformed from the default cube. If two cubes have collision, then we transform the coordinate system to the first cube, so the first cube becomes to the default cube again. It's not hard to imagine two cubes still have collision. So to test collision, we can always transform the first cube to default cube, by using `cube1.transform_matrix.inverse`. Then transform the second cube use `cube1.transform_matrix.inverse` as well, and then test if these two new cubes have collision.

# Collision Detection
Because one of the cube is default cube, so the detection becomes much easier.

The way to test it is
+ For each xy, xz, yz plane
    + project all 16 points onto this plane
    + for each edge in this plane, compute the normal direction (24 edges in total, but only 5 normal directions, first cube has 2 (1,0) and (0,1), second cube has 3 direction)
    + project all 16 points onto each normal ( some points clearly have the projection value)
    +  If 2 sets of projected values can be separated, then no collision
If it can never separated, then it has collision
## Project to the first cube edges
The first cube is axis aligned. For second cube, we can simply test, if all x coordinates are larger than `1`, or smaller than `-1`, then such plane exist, and no collision. Similarly for y and z coordinates.
## Project to the second cube edges
Though, this is complicated, but we still use the information that the first cube only has coordinates (-1,-1) (-1,1) (1,-1) (1,1)

However, this is not enough. See page 9 of [this](https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/previousinformation/physics4collisiondetection/2017%20Tutorial%204%20-%20Collision%20Detection.pdf).

So we also need to test it other way around, which is transform the second cube to the default cube. And test it again.

Return the value `cube1.collision_single_side(cube2) && cube1.collision_single_side(cube2)`. If either returns false, which means the separating axis exist, so it has no collision.

# Implementation

If two cubes have transformation matrix $T_{1}$ $T_{2}$, with their inverse $R_{1}$ $R_{2}$. We set $P_{1} = R_{1} \cdot T_{2} $

let
$$
P_{1} = \begin{pmatrix}
p_{00} & p_{01} & p_{02} & p_{03} \\
p_{10} & p_{11} & p_{12} & p_{13} \\
p_{20} & p_{21} & p_{22} & p_{23} \\
p_{30} & p_{31} & p_{32} & p_{33} \\
\end{pmatrix}
$$

So, the second cube has the coordinates $ P_{1} \cdot (\pm 1, \pm 1, \pm 1, 1)^{T}$.

## Project to the first cube edges
To test if x coordinates are all larger than 1 or smaller than -1. Clearly, the largest x coordinates is $X_{\max} = p_{03} + |p_{00}| + |p_{01}| + |p_{02}|$, and smallest coordinates is  $X_{\min} = p_{03} - |p_{00}| - |p_{01}| - |p_{02}|$.

We need to test
$$
X_{\max} +1 <0  \ \ \ \ \text{or} \\ 
X_{\min} -1 >0
$$,
which is equivalent to
$$
|p_{00}| + |p_{01}| + |p_{02}| + 1 + p_{03} <0 \ \ \text{or} \\
|p_{00}| + |p_{01}| + |p_{02}| + 1 - p_{03} <0
$$
which is equivalent to
$$
|p_{00}| + |p_{01}| + |p_{02}| + 1 - |p_{03}| < 0 
$$

For y, z coordinates, it works the same way, so we can combine them together using one simd register.
```cpp
struct vec4{
    __m128 data;
};
const vec4 abscol0; // convert all values in 0th column of matrix P to its absolute value
const vec4 abscol1; // convert all values in 1st column of matrix P to its absolute value
const vec4 abscol2; // convert all values in 2nd column of matrix P to its absolute value
const vec4 abscol3; // convert all values in 3rd column of matrix P to its absolute value
const vec4 fourone; // a vector with four 1
const vec4 test = abscol0 + abscol1 + abscol2 + fourone - abscol3;
if (test.HasOneNegative())
    return false; // If one of that is smaller than 0, then axis exist, no collision
```

## Project to the second cube edge

In default cube, it can only have 3 unique edge directions (1,0,0) (0,1,0) (0,0,1).

So, the edges after transformation $P$ are $P \cdot (1,0,0)$ ,$P \cdot (0,1,0)$ ,$P \cdot (0,0,1)$, which are the first, second, third column of matrix $P$.

+ For the first edge, which is $P \cdot (1,0,0)$,
    + In xy plane, the projected edge is $(p_{00}, p_{10})$, its normal is $-(p_{10}, p_{00})$.
    + We project all 4 vertices from the first cube, which has the value $\pm 1 \cdot p_{00} \pm 1 \cdot p_{10}$. Clearly the value is in the range $[-| p_{00}|-| p_{10}|, | p_{00}|+| p_{10}|]$ 
    + We project all 8 vertices from the second cube, which has the value $p_{00}\cdot ( \pm p_{11} \pm p_{12} ) + p_{10} *(\pm p_{01} \pm p_{02}) + p_{00}\cdot p_{13} - p_{10}\cdot p_{13} $ (4 unique values)
    + The value is in the range $[p_{00}\cdot p_{13} - p_{10}\cdot p_{13} - |p_{00}|\cdot ( |p_{11}| + |p_{12}| ) - | p_{10}| \cdot( |p_{01}| + |p_{02}|) ,p_{00}\cdot p_{13} - p_{10}\cdot p_{13} + |p_{00} | \cdot ( |p_{11}| + |p_{12}|)  + | p_{10}| \cdot( |p_{01}| + |p_{02}|)  ]$
    + So we need to test the if 
    $$
    | p_{00}|+| p_{10}| < p_{00}\cdot p_{13} - p_{10}\cdot p_{13} - |p_{00}|\cdot ( | p_{11} |+ |p_{12}|) - | p_{10} \cdot( |p_{01} |+ |p_{02}|) \ \ \  \text{or} \\
    -| p_{00}|-| p_{10}| > p_{00}\cdot p_{13} - p_{10}\cdot p_{13} + |p_{00}|\cdot ( |p_{11}| + |p_{12} |) + | p_{10} |\cdot( |p_{01} |+ |p_{02}|)
    $$, 
    + which is equivalent to
    $$

    |p_{00}|\cdot ( |p_{11}| + |p_{12}|) + | p_{10}| \cdot( |p_{01}| + |p_{02}|)  + |p_{00}| + |p_{10}| - | p_{00}\cdot p_{13} + p_{10}\cdot p_{03}| <0
    $$

    We can have the similar equation for yz, xz plane, and put it into one register.

    To compute $| p_{00}|+| p_{10}|$, we can do this by add `col0` and `col1shuffle`, where $\text{col1shuffle} = (p_{10}, p_{20},p_{00},p_{30})^T $. So we can compute $| p_{00}|+| p_{10}|$, $| p_{10}|+| p_{20}|$, $| p_{20}+| p_{00}|$ altogether. For other part, we can use the same shuffle.


    ```cpp
    const vec4 abscol0shuffle = _mm_permute_ps(abscol0.data, 0b11010010); // shuffle the absolute value of the first column 
    const vec4 abscol1shuffle = _mm_permute_ps(abscol1.data, 0b11010010); // shuffle the absolute value of the second column 
    const vec4 abscol2shuffle = _mm_permute_ps(abscol2.data, 0b11010010); // shuffle the absolute value of the third column 

    const vec4 col0shuffle = _mm_permute_ps(transpose.row0.data, 0b11010010); // shuffle the value of the first column ! not absolute value
    const vec4 col1shuffle = _mm_permute_ps(transpose.row1.data, 0b11010010); // shuffle the value of the second column ! not absolute value
    const vec4 col2shuffle = _mm_permute_ps(transpose.row2.data, 0b11010010); // shuffle the value of the third column ! not absolute value
    const vec4 col3shuffle = _mm_permute_ps(transpose.row3.data, 0b11010010); // shuffle the value of the fourth column ! not absolute value

    const vec4 threshold = abscol0 + abscol0shuffle; // first element is |p00|+|p10|

    const vec4 range11 = _mm_add_ps(_mm_mul_ps(abscol0.data, abscol1shufle.data), _mm_mul_ps(abscol0shuffle.data, abscol1.data)); // first element is |p_{00}| * |p_{11}| + |p_{10}| * |p_{01}|
    const vec4 range12 = _mm_add_ps(_mm_mul_ps(abscol0.data, abscol2shufle.data), _mm_mul_ps(abscol0shuffle.data, abscol2.data)); // first element is  |p_{00}| * |p_{12}| + |p_{10}| * |p_{02}| 
    const vec4 range1 = range11 + range12;
    const vec4 center = _mm_sub_ps(_mm_mul_ps(transpose.row0.data, col3shufle.data), _mm_mul_ps(col0shuffle.data, transpose.row3.data)); // first element is  p_{00} * p_{13} - p_{10} * p_{03} 
    const vec4 abscenter = _mm_and_ps(absfourmask, center.data);
    const vec4 test = threshold + range1 - abscenter;

    if (test.HasOneNegative()) // if in any of xy, xz, yz plane, value is smaller than 0, there exist a separating axis 
        return false;
    ```


Then we apply the same computation to the second and third edge, which is the second and the third column of matrix $P$.


    
 



# Others
1. It maybe can even optimized more, such as using column major matrix, so there is no need to call `transform.Transpose()`. Or some other micro optimizations. But I am not going to do that.