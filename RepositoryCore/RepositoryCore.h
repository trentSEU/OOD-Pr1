#pragma once
/////////////////////////////////////////////////////////////////////
// RepositoryCore.h   - Check in, check out, browse, version       //
// ver 1.0                                                         //
// Language:    C++, Visual Studio 2017                            //
// Platform:    Macbook pro, Windows 10 Home                       //
// Application: Spring 2018 CSE687 Project#2                       //
// Author:      Yuan Liu, yliu219@syr.edu                          //
// Referrence:  Jim Fawcett, CSE687                                //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* -This package provides all of the functions needed to check in,
*  check out, version and browse.
*
* Public Interface:
* -----------------
* CRepositoryCore(DbCore<PayLoad>& db)																		//constructor with parameter
* bool checkIn(Owner own, pName pname)																		//check in
* bool checkOut(Key key, bool dep)																				//check out
* bool close(DbElement<PayLoad>& elem)																		//close targged element
* template<typename CallObj>
* bool browse(Conditions<PayLoad> conds, CallObj callobj)									//browse
* std::string getVersion(std::string name, std::string nspace)						//return namespace_name.version
* std::string removeVersion(std::string name, std::string nspace)					//remove version number
* std::string& nspace()																										//get/set function for nspace_
* std::string nspace() const
* void elem(const std::string& nspace)
* std::vector<DbElement<PayLoad> >& elemVec()															//get/set function for elemVec_
* std::vector<DbElement<PayLoad> > elemVec() const
* void elemVec(std::vector<DbElement<PayLoad> >& elemVec)
*
*
* Required Files:
* ---------------
* NoSqlDb.h
* Version.h
* PayLoad.h
* CheckIn.h
* Browse.h
* CheckOut.h
*
* Build Process:
* --------------
* devenv SoftwareRepository.sln /rebuild debug
*
* Maintenance History:
* --------------------
* ver 1.0 : 05 Mar 2018
*/

#include "../NoSqlDb/Executive/NoSqlDb.h"
#include "../Version/Version.h"
#include "../PayLoad/PayLoad.h"
#include "../Check-in/CheckIn.h"
#include "../Browse/Browse.h" 
#include "../Check-out/CheckOut.h"

namespace Repository {
	using namespace NoSqlDb;
	using PayLoad = Repository::PayLoad;
	using Owner = std::string;
	using pName = std::string;

	class RepositoryCore {
	public:
		RepositoryCore(DbCore<PayLoad>& db) : db_(db) {}
		bool checkIn(Owner own, pName pname);
		bool checkOut(Key key, bool dep, std::vector<std::string>& filePaths);
		bool close(DbElement<PayLoad>& elem);
		template<typename CallObj>
		bool browse(Conditions<PayLoad> conds, CallObj callobj);
		std::string getVersion(std::string name, std::string nspace);
		std::string removeVersion(std::string name, std::string nspace);

		std::string& nspace() { return nspace_; }
		std::string nspace() const { return nspace_; }
		void elem(const std::string& nspace) { nspace_ = nspace; }

		std::vector<DbElement<PayLoad> >& elemVec() { return elemVec_; }
		std::vector<DbElement<PayLoad> > elemVec() const { return elemVec_; }
		void elemVec(std::vector<DbElement<PayLoad> >& elemVec) { elemVec_ = elemVec; }

		void showRepo() { showDb(db_); }

		DbCore<PayLoad>& db() { return db_; };
		DbCore<PayLoad> db() const { return db_; };
	private:
		DbCore<PayLoad>& db_;
		std::string nspace_;
		std::vector<DbElement<PayLoad> > elemVec_;
	};
	//----< check in package >-----------------------------------------------

	inline bool RepositoryCore::checkIn(Owner own, pName pname) {
		CheckIn checkin(db_);
		checkin.nspace() = nspace_;
		for (auto elem : elemVec_) checkin.elemVec().push_back(elem);
		if (checkin.checkInPackage(own, pname)) {
			std::cout << "  check-in success!\n";
			return true;
		}
		std::cout << "  check-in fail!\n";
		elemVec_.clear();
		return false;
	}
	//----< check out file >-------------------------------------------------

	inline bool RepositoryCore::checkOut(Key key, bool dep, std::vector<std::string>& filePaths) {
		CheckOut checkout(db_);
		bool suc = checkout.checkFileOut(key, dep, filePaths);
		if (suc) std::cout << "  check-out success!\n";
		else std::cout << "  check-out fail!\n";
		return suc;
	}
	//----< close element >--------------------------------------------------

	inline bool RepositoryCore::close(DbElement<PayLoad>& elem) {
		CheckIn checkin(db_);
		return checkin.closeElement(elem);
	}
	//----< browse files >---------------------------------------------------

	template<typename CallObj>
	inline bool RepositoryCore::browse(Conditions<PayLoad> conds, CallObj callobj) {
		Repository::Browse browse(db_);
		browse.browseFiles(conds, callobj);
		return true;
	}
	//----< remove version number >------------------------------------------

	inline std::string RepositoryCore::removeVersion(std::string name, std::string nspace) {
		Version *ver = new Version(db_, nspace);
		return ver->removeVersion(name);
	}
	//----< return namespace_name.version >----------------------------------

	inline std::string RepositoryCore::getVersion(std::string name, std::string nspace) {
		Version *ver = new Version(db_, nspace);
		return ver->getVersion(name);
	}
}