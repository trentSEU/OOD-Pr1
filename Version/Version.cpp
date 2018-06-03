/////////////////////////////////////////////////////////////////////
// Version.cpp    -test version                                    //
// ver 1.0                                                         //
// Language:      C++, Visual Studio 2017                          //
// Platform:      Macbook pro, Windows 10 Home                     //
// Application:   Spring 2018 CSE687 Project#2                     //
// Author:        Yuan Liu, yliu219@syr.edu                        //
// Referrence:    Jim Fawcett, CSE687                              //
/////////////////////////////////////////////////////////////////////
#include "Version.h"
using namespace Repository;

#ifdef TEST_VERSION
int main() {
	DbCore<Repository::PayLoad> db;
	DbElement<Repository::PayLoad> elem = makeElement<Repository::PayLoad>("p5.h.1", "a description");
	Repository::PayLoad pl;
	pl.path("a payload value");
	pl.owner("jim");
	pl.status() = false;
	pl.categories().push_back("some category");
	elem.payLoad(pl);
	db["first"] = elem;
	DbElement<Repository::PayLoad> elem1 = makeElement<Repository::PayLoad>("p5.h.2", "a description");
	pl.path("a payload value");
	pl.owner("jim");
	pl.status() = true;
	pl.categories().push_back("some category");
	elem1.payLoad(pl);
	db["second"] = elem1;
	DbElement<Repository::PayLoad> elem2 = makeElement<Repository::PayLoad>("p5.h.3", "a description");
	pl.path("a payload value");
	pl.owner("jim");
	pl.status() = false;
	pl.categories().push_back("some category");
	elem2.payLoad(pl);
	db["third"] = elem2;

	Version *ver = new Version(db, "namespace");
	std::cout << "  -remove version number for p5.h.3\n";
	std::cout << "   " << ver->removeVersion("p5.h.3") << std::endl;
	std::cout << "  -add version number to p5.h, p5.h.1, p5.h.2, p5.h.3 already exists\n";
	std::cout << "   " << ver->getVersion("p5.h") << std::endl;
}
#endif