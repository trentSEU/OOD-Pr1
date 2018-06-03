#include <iostream>
#include "p3.h"

void p3::say() {
	std::cout << "I am p3";
}

int main() {
	p3 p;
	p.say();
	return 0;
}