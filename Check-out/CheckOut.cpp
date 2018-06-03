/////////////////////////////////////////////////////////////////////
// CheckOut.cpp     - Test Check out                               //
// ver 1.0                                                         //
// Language:      C++, Visual Studio 2017                          //
// Platform:      Macbook pro, Windows 10 Home                     //
// Application:   Spring 2018 CSE687 Project#2                     //
// Author:        Yuan Liu, yliu219@syr.edu                        //
// Referrence:    Jim Fawcett, CSE687                              //
/////////////////////////////////////////////////////////////////////
#include "CheckOut.h"
using namespace Repository;

#ifdef TEST_CHECKOUT
int main() {
	DbCore<Repository::PayLoad> db;
	CheckIn checkin(db);
	checkin.nspace() = "NoSqlDb";
	DbElement<Repository::PayLoad> elem1 = makeElement<Repository::PayLoad>("p1.h", "a description");
	Repository::PayLoad pl;
	pl.path("../Archive/package1");
	pl.owner("jim");
	pl.categories().push_back("some category");
	elem1.payLoad(pl);
	elem1.addChildKey("NoSqlDb_p1.cpp.1");
	elem1.addChildKey("NoSqlDb_p2.h.1");
	elem1.addChildKey("NoSqlDb_p3.h.1");
	DbElement<Repository::PayLoad> elem2 = makeElement<Repository::PayLoad>("p1.cpp", "a description");
	elem2.payLoad(pl);
	elem2.addChildKey("NoSqlDb_p1.h.1");
	checkin.elemVec().push_back(elem1);
	checkin.elemVec().push_back(elem2);
	if (checkin.checkInPackage("jim", "package1")) std::cout << "success" << std::endl;

	DbElement<Repository::PayLoad> elem3 = makeElement<Repository::PayLoad>("p2.h", "a description");
	pl.path("../Archive/package2");
	elem3.payLoad(pl);
	elem3.addChildKey("NoSqlDb_p2.cpp.1");
	elem3.addChildKey("NoSqlDb_p3.h.1");
	DbElement<Repository::PayLoad> elem4 = makeElement<Repository::PayLoad>("p2.cpp", "a description");
	elem4.payLoad(pl);
	elem4.addChildKey("NoSqlDb_p2.h.1");
	checkin.elemVec().push_back(elem3);
	checkin.elemVec().push_back(elem4);
	if (checkin.checkInPackage("jim", "package2")) std::cout << "success" << std::endl;

	DbElement<Repository::PayLoad> elem5 = makeElement<Repository::PayLoad>("p3.h", "a description");
	pl.path("../Archive/package3");
	elem5.payLoad(pl);
	elem5.addChildKey("NoSqlDb_p3.cpp.1");
	DbElement<Repository::PayLoad> elem6 = makeElement<Repository::PayLoad>("p3.cpp", "a description");
	elem6.payLoad(pl);
	elem6.addChildKey("NoSqlDb_p3.h.1");
	checkin.elemVec().push_back(elem5);
	checkin.elemVec().push_back(elem6);
	if (checkin.checkInPackage("jim", "package3")) std::cout << "success" << std::endl;

	showDb(db);
	std::cout << std::endl;

	CheckOut checkout(db);
	std::vector<std::string> filePaths;
	checkout.checkFileOut("NoSqlDb_p1.h.1", true, filePaths);
}
#endif