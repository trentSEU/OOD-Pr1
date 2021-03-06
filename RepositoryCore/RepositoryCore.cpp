///////////////////////////////////////////////////////////////////////
// RepositoryCore.cpp                                                //
// ver 1.0                                                           //
// Yuan Liu, CSE687 - Object Oriented Design, Spring 2018            //
///////////////////////////////////////////////////////////////////////
#include "RepositoryCore.h"
using namespace NoSqlDb;
using namespace Repository;
using PayLoad = Repository::PayLoad;

#ifdef TEST_REPO
int main() {
	Utilities::title("Test RepositoryCore");
	std::cout << "\n  -Check in and version a file named p1.h,"
		<< "  the format of key is package_name.version\n"
		<< "  if check in success, you can find files in package1 in dir ../Checked-in";
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
	repo.elemVec().push_back(elem1);
	if (!repo.checkIn("jim", "package1")) return false;
	std::cout << "\n  -Browse the file named p1.h";
	Conditions<PayLoad> conds;
	conds.name("p1.h");
	auto isOwnerJim = [](DbElement<PayLoad>& elem) {
		return elem.payLoad().owner() == "jim";
	};
	repo.browse(conds, isOwnerJim);
	//package name with dep means containing the dependent files
	std::cout << "\n  -Check out the file named p1.h\n"
		<< "  if check out success, you can find namespace_name.version-(dep) in dir ../Checked-out";
	if (repo.checkOut("NoSqlDb_p1.h.1", true) == "") return false;
	std::cout << std::endl;
	return 0;
}
#endif

