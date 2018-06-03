#include <iostream>
#include "p2.h"

void p1::say() {
	std::cout << "I am p2";
}

int main() {
	p2 p;
	p.say();
	return 0;
}