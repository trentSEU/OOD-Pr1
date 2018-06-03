#include <iostream>
#include "p1.h"

void p1::say() {
	std::cout << "I am p1";
}

int main() {
	p1 p;
	p.say();
	return 0;
}