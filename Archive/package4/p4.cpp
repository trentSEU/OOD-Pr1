#include <iostream>
#include "p4.h"

void p1::say() {
	std::cout << "I am p4";
}

int main() {
	p4 p;
	p.say();
	return 0;
}