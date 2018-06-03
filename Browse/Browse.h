#pragma once
/////////////////////////////////////////////////////////////////////
// Browse.h   - Implements browse operation                        //
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
* -This package provides all of the functions needed to browse a file.
*  User provides Conditions and lanmda to search keys. Files found would
*  display on console, notepad would display the content of files.
*
* Public Interface:
* -----------------
* Browse()(DbCore<PayLoad>& db)																						//constructor
* template<typename CallObj>																						
*	void browseFiles(Conditions<PayLoad> conds, CallObj callobj);						//find and display files
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

#include "../Process/Process/Process.h"
#include "../NoSqlDb/Executive/NoSqlDb.h"
#include "../PayLoad/PayLoad.h"

namespace Repository {
	using namespace NoSqlDb;
	using PayLoad = Repository::PayLoad;

	class Browse {
	public:
		Browse(DbCore<PayLoad>& db) : db_(db) {}
		template<typename CallObj>
		void browseFiles(Conditions<PayLoad> conds, CallObj callobj);
	private:
		DbCore<PayLoad>& db_;
	};
	//----< browse files by conds >-----------------------------------------------

	template<typename CallObj>
	inline void Browse::browseFiles(Conditions<PayLoad> conds, CallObj callobj) {
		Query<PayLoad> que(db_);
		Keys keys;
		Keys keys1 = que.select(conds).keys();
		Keys keys2 = que.select(callobj).keys();
		for (auto k : keys2) {
			if (find(keys1.begin(), keys1.end(), k) != keys1.end()) keys.push_back(k);
		}

		std::cout << "\n  description of elements found, if any" << std::endl;
		std::cout << std::setw(15) << std::left << "  Name";
		std::cout << std::setw(20) << std::left << " Description" << std::endl;
		std::cout << std::setw(15) << std::left << "  ------------";
		std::cout << std::setw(20) << std::left << " ------------------" << std::endl;
		for (auto k : keys) {
			std::cout << "  " << std::setw(13) << std::left << db_[k].name();
			std::cout << " " << std::setw(20) << std::left << db_[k].descrip() << std::endl;

			Process p;
			p.title("browsing reader");
			std::string appPath = "c:/windows/system32/notepad.exe";
			p.application(appPath);
			std::string cmdLine = "/A " + db_[k].payLoad().path();												// /A -> ANSI/GBK coding
			p.commandLine(cmdLine);
			p.create();
		}
	}
}