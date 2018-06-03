/////////////////////////////////////////////////////////////////////
// Browse.cpp     - Test browse                                    //
// ver 1.0                                                         //
// Language:      C++, Visual Studio 2017                          //
// Platform:      Macbook pro, Windows 10 Home                     //
// Application:   Spring 2018 CSE687 Project#2                     //
// Author:        Yuan Liu, yliu219@syr.edu                        //
// Referrence:    Jim Fawcett, CSE687                              //
/////////////////////////////////////////////////////////////////////
#include "Browse.h"
#include "../Check-in/CheckIn.h"
using namespace NoSqlDb;
using CheckIn = Repository::CheckIn;

//----< check in elements for test >--------------------------------------------

void testHelper(DbCore<Repository::PayLoad>& db) {
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
}

#ifdef TEST_BROWSE
int main() {
	DbCore<Repository::PayLoad> db;
	testHelper(db);
	showDb(db);
	Conditions<Repository::PayLoad> conds;
	conds.name("h");
	std::string owner = "jim";
	auto hasOwner = [&owner](DbElement<Repository::PayLoad>& elem) {
		return (elem.payLoad()).owner() == owner;
	};
	std::string path = "2";
	auto hasPath = [&path](DbElement<Repository::PayLoad>& elem) {
		if (elem.payLoad().path().find(path) == std::string::npos) return false;
		return true;
	};
	Repository::Browse browse(db);
	std::cout << "\n\n  -Browse the file whose name contains h and path contains 2";
	browse.browseFiles(conds, hasPath);
	std::cout << std::endl;
}
#endif