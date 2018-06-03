/////////////////////////////////////////////////////////////////////
// PayLoad.cpp   - Test PayLoad                                    //
// ver 1.0                                                         //
// Language:       C++, Visual Studio 2017                         //
// Platform:       Macbook pro, Windows 10 Home                    //
// Application:    Spring 2018 CSE687 Project#2                    //
// Author:         Yuan Liu, yliu219@syr.edu                       //
// Referrence:     Jim Fawcett, CSE687                             //
/////////////////////////////////////////////////////////////////////
#include "../RepositoryCore/RepositoryCore.h"
#include "../NoSqlDb/Executive/NoSqlDb.h"
#include "PayLoad.h"
using namespace NoSqlDb;
using namespace Repository;
using PayLoad = Repository::PayLoad;

#ifdef TEST_PAYLOAD
int main() {
	DbCore<PayLoad> db;
	RepositoryCore repo(db);
	repo.nspace() = "NoSqlDb";
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
	repo.elemVec().push_back(elem1);
	repo.elemVec().push_back(elem2);
	repo.checkIn("jim", "package1");
	DbElement<Repository::PayLoad> elem3 = makeElement<Repository::PayLoad>("p2.h", "a description");
	pl.path("../Archive/package2");
	elem3.payLoad(pl);
	elem3.addChildKey("NoSqlDb_p2.cpp.1");
	elem3.addChildKey("NoSqlDb_p3.h.1");
	DbElement<Repository::PayLoad> elem4 = makeElement<Repository::PayLoad>("p2.cpp", "a description");
	elem4.payLoad(pl);
	repo.elemVec().clear();
	repo.elemVec().push_back(elem3);
	repo.elemVec().push_back(elem4);
	repo.checkIn("jim", "package2");
	DbElement<Repository::PayLoad> elem5 = makeElement<Repository::PayLoad>("p3.h", "a description");
	pl.path("../Archive/package3");
	elem5.payLoad(pl);
	elem5.addChildKey("NoSqlDb_p3.cpp.1");
	DbElement<Repository::PayLoad> elem6 = makeElement<Repository::PayLoad>("p3.cpp", "a description");
	elem6.payLoad(pl);
	repo.elemVec().clear();
	repo.elemVec().push_back(elem5);
	repo.elemVec().push_back(elem6);
	repo.checkIn("jim", "package3");
	std::cout << "\n  -The payload is shown as below:";
	repo.close(db["NoSqlDb_p3.cpp.1"]);
	repo.close(db["NoSqlDb_p3.h.1"]);
	repo.close(db["NoSqlDb_p2.cpp.1"]);
	repo.close(db["NoSqlDb_p2.h.1"]);
	repo.close(db["NoSqlDb_p1.cpp.1"]);
	repo.close(db["NoSqlDb_p1.h.1"]);
	elem6.payLoad().showDb(db);
}
#endif