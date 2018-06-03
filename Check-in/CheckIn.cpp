/////////////////////////////////////////////////////////////////////
// CheckIn.cpp   - Test check in                                   //
// ver 1.0                                                         //
// Language:       C++, Visual Studio 2017                         //
// Platform:       Macbook pro, Windows 10 Home                    //
// Application:    Spring 2018 CSE687 Project#2                    //
// Author:         Yuan Liu, yliu219@syr.edu                       //
// Referrence:     Jim Fawcett, CSE687                             //
/////////////////////////////////////////////////////////////////////
#include "CheckIn.h"
using namespace Repository;

#ifdef TEST_CHECKIN
int main() {
	std::cout << "  -If check in success, you can find the files in package5 in ../package5";
	DbCore<Repository::PayLoad> db;
	DbElement<Repository::PayLoad> elem1 = makeElement<Repository::PayLoad>("p5.h", "a description");
	Repository::PayLoad pl;
	pl.path("../Archive/package5");
	pl.owner("jim");
	pl.categories().push_back("some category");
	elem1.payLoad(pl);
	DbElement<Repository::PayLoad> elem2 = makeElement<Repository::PayLoad>("p5.h", "a description");
	elem2.payLoad(pl);
	elem2.addChildKey("NoSqlDb_p5.h.1");
	elem2.addChildKey("NoSqlDb_p1.h.1");
	CheckIn checkin(db);
	checkin.nspace() = "NoSqlDb";
	checkin.elemVec().push_back(elem1);
	checkin.elemVec().push_back(elem2);
	if(checkin.checkInPackage("jim", "package5")) std::cout << "success" << std::endl;
	showDb(db);
	std::cout << std::endl;
}
#endif