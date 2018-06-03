#pragma once
/////////////////////////////////////////////////////////////////////
// Version.h   - Implements version operation                      //
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
* -This package provides all of the functions needed to add or remove
*  version number.
*
* Public Interface:
* -----------------
* Version(DbCore<PayLoad> db)																							//constructor with patameter
* Version(DbCore<PayLoad> db, std::string nspace) 										    //constructor with parameter
* std::string getVersion(std::string name)																//return package_name.version
* std::string removeVersion(std::string name)															//remove version number
*
* Required Files:
* ---------------
* NoSqlDb.h
* PayLoad.h
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
#include "../PayLoad/PayLoad.h"
#include <string>

namespace Repository {
	using namespace NoSqlDb;
	using PayLoad = Repository::PayLoad;

	class Version {
	public:
		Version(DbCore<PayLoad> db, std::string nspace) : db_(db), nspace_(nspace) {}
		Version(DbCore<PayLoad> db) : db_(db) {}
		std::string getVersion(std::string name);
		std::string removeVersion(std::string name);
	private:
		DbCore<PayLoad> db_;
		std::string nspace_;
	};
	//----< remove the version number from given name >----------------------

	inline std::string Version::removeVersion(std::string name) {
		char *p, *temp, *cstr;
		const char* sep = ".";
		std::string res = "";
		cstr = const_cast<char*>(name.c_str());
		temp = strtok_s(cstr, sep, &p);
		res += temp;
		res += ".";
		while (true) {
			temp = strtok_s(NULL, sep, &p);
			if ((int)*temp < 48 || (int)*temp > 57) res += temp;
			else break;
		}
		return res;
	}
	//----< add a version number to the  given name >-------------------------

	inline std::string Version::getVersion(std::string name) {
		Query<PayLoad> que(db_);
		Conditions<PayLoad> conds;
		conds.name(name);
		Keys keys = que.select(conds).keys();
		//if the status of found elemnt is false, add version numer by 1
		int count = 1;
		for (auto key : keys) {
			if (!db_[key].payLoad().status() || !db_[key].payLoad().closePend()) count++;
		}
		return nspace_ + "_" + name + "." + std::to_string(count);
	}
}