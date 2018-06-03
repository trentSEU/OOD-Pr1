#include <iostream>
#include "p5.h"

void p1::say() {
	std::cout << "I am p5\n
				  and this is a open package";
}

int main() {
	p5 p;
	p.say();
	return 0;
}