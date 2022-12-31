#include "vec.h"
#include "cube.h"
#include <iostream>



void test1() {
	Cube c1, c2;
	{
		c2.m_translation = vec4(2.001, 1.999, -0.141211, 1.0f);
		c2.m_scale = vec4(0.815121, 1, 1.16, 1);
		c2.m_rotation = vec4(0.158525, -0.133174, -0.222687, 0.952652);
	}

	std::cout << std::boolalpha << "C1 collision with C2 " << c1.Collision(c2) << ". It should be true" << std::endl;
}

void test2() {
	Cube c1, c2;
	{
		c2.m_translation = vec4(2.001, 1.999, -0.141211, 1.0f);
		c2.m_scale = vec4(0.815121, 1, 1.13318, 1);
		c2.m_rotation = vec4(0.158525, -0.133174, -0.222687, 0.952652);
	}

	std::cout << std::boolalpha << "C1 collision with C2 " << c1.Collision(c2) << ". It should be false" << std::endl;
}

int main()
{
	test1();
	test2();
}
