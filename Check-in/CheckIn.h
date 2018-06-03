#pragma once
/////////////////////////////////////////////////////////////////////
// CheckIn.h   - Implements check in operation                     //
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
* -This package provides all of the functions needed to check in a
*  file or a package with multiple files. If found closed former
*  version, the version number would add by 1. If found open former
*  version, the former version would be replaced and the version number
*  would not change. If found close-pending former version, the user
*  would be asked to close the close-pending one before check in.
*
* Public Interface:
* -----------------
* CheckIn()																																//constructor
* CheckIn(DbCore<PayLoad>& db)																						//constructor with parameter
* std::string& nspace()																										//get/set function for nspace_
* std::string nspace() const
* void elem(const std::string& nspace)
* std::vector<DbElement<PayLoad> >& elemVec()															//get/set function for elemVec_
* std::vector<DbElement<PayLoad> > elemVec() const
* void elemVec(std::vector<DbElement<PayLoad> >& elemVec)
* Key addElement(owner own, DbElement<PayLoad>& elem);										//add element to db_
* bool checkInPackage(owner own, pName);																	//check in package with files
* bool checkInFile(owner own, DbElement<PayLoad>& elem, path topath);			//check in a single file
* bool checkDepd(DbElement<PayLoad>& elem);																//find open/missing children
* bool closeElement(DbElement<PayLoad>& elem);														//try to close element
* bool copyFile(path from, path to);																			//copy single file
* bool checkClosePend(DbElement<PayLoad>& elem);													//find close-pending file
*
* Required Files:
* ---------------
* NoSqlDb.h
* Version.h
*
* Build Process:
* --------------
* devenv SoftwareRepository.sln /rebuild debug
*
* Maintenance History:
* --------------------
* ver 1.0 : 05 Mar 2018
*/

#include <iostream>
#include <stdio.h>
#include <io.h>
#include <direct.h>
#include "../NoSqlDb/Executive/NoSqlDb.h"
#include "../Version/Version.h"
#define BUF_SIZE 256

namespace Repository {
	using namespace NoSqlDb;
	using PayLoad = Repository::PayLoad;

	class CheckIn{
	public:
		using owner = std::string;
		using path = std::string;
		using pName = std::string;

		CheckIn() = default;
		CheckIn(DbCore<PayLoad>& db) : db_(db) {}

		std::string& nspace() { return nspace_; }
		std::string nspace() const { return nspace_; }
		void elem(const std::string& nspace) { nspace_ = nspace; }

		std::vector<DbElement<PayLoad> >& elemVec() { return elemVec_; }
		std::vector<DbElement<PayLoad> > elemVec() const { return elemVec_; }
		void elemVec(std::vector<DbElement<PayLoad> >& elemVec) { elemVec_ = elemVec; }

		Key addElement(owner own, DbElement<PayLoad>& elem);			//return NULL if own != elem.payLoad().owner()
		bool checkInPackage(owner own, pName);
		bool checkInFile(owner own, DbElement<PayLoad>& elem, path topath);
		bool checkDepd(DbElement<PayLoad>& elem);								//check dependencies
		bool closeElement(DbElement<PayLoad>& elem);
		bool copyFile(path from, path to);
		bool checkClosePend(DbElement<PayLoad>& elem);

	private:
		bool status_;																						//true->open, false->close
		const path toPath_ = "../Checked-in";
		std::string nspace_;
		std::vector<DbElement<PayLoad> > elemVec_;
		DbCore<PayLoad>& db_;
	};
	//----< add new element to the db >-----------------------------------------------
	//if there are elements with same key and open status, they would be replaced

	inline Key CheckIn::addElement(owner own, DbElement<PayLoad>& elem) {
		elem.payLoad().status() = true;													//element open
		elem.payLoad().closePend() = true;											//element not close-pending
		if (own != elem.payLoad().owner()) {
			std::cout << "  only the owenr can check in the package!" << std::endl;
			return "";
		}
		Version *ver = new Version(db_, nspace_);
		Key key = ver->getVersion(elem.name());
		db_[key] = elem;
		return key;
	}
	//----< make sure all the dependent package in db and close >----------------------

	inline bool CheckIn::checkDepd(DbElement<PayLoad>& elem) {
		bool depd = true;
		if (elem.children().size() > 0) {
			for (auto child : elem.children()) {
				if (!db_.contains(child)) {
					std::cout << "\n  the child " << child << " not exists in the database!";
					depd = false;
				}
				//open status and not close-pending
				if (depd) {
					if (db_[child].payLoad().status() && db_[child].payLoad().closePend()) {
						std::cout << "\n  the child " << child << "'s status is open!";
						depd = false;
					}
				}
			}
		}
		return depd;
	}
	//----< modify the status to false means close >-----------------------------------

	inline bool CheckIn::closeElement(DbElement<PayLoad>& elem) {
		if (checkDepd(elem)) {
			elem.payLoad().status() = false;
			elem.payLoad().closePend() = true;
		}
		else {
			std::cout << "\n  fail to close the element " << elem.name()
								<< ", change it to be close-pending";
			elem.payLoad().closePend() = false;
			return false;
		}
		return true;
	}
	//----< copy single file from fromPath_ to toPath_ >-------------------------------

	inline bool CheckIn::copyFile(path from, path to) {
		const char *pSrc, *pDes;
		FILE *in_file, *out_file;
		pSrc = from.c_str();
		pDes = to.c_str();
		char data[BUF_SIZE];
		size_t bytes_in, bytes_out;
		long len = 0;
		fopen_s(&in_file, pSrc, "rb");
		if (in_file == NULL) {
			perror(pSrc);
			return false;
		}
		fopen_s(&out_file, pDes, "wb");
		if (out_file == NULL) {
			perror(pDes);
			return false;
		}
		while ((bytes_in = fread(data, 1, BUF_SIZE, in_file)) > 0) {
			bytes_out = fwrite(data, 1, bytes_in, out_file);
			if (bytes_in != bytes_out) {
				perror("  Fatal write error.\n");
				return false;
			}
			len += bytes_out;
		}
		printf("\n  copying file...%d bytes copied\n", len);
		fclose(in_file);
		fclose(out_file);
		return true;
	}
	//----< check closePend of elements in database >----------------------------------

	inline bool CheckIn::checkClosePend(DbElement<PayLoad>& elem) {
		Query<PayLoad> que(db_);
		Conditions<PayLoad> conds;
		conds.name(elem.name());
		Keys keys = que.select(conds).keys();
		//return false if find closePending child
		for (auto k : keys) {
			if (!db_[k].payLoad().closePend()) {
				std::cout << "\n  element " << k << " is close-pending!" << std::endl;
				return false;
			}
		}
		return true;
	}
	//----< check in a single element >------------------------------------------------

	inline bool CheckIn::checkInFile(owner own, DbElement<PayLoad>& elem, path topath) {
		std::cout << "  checking in file " << elem.name() << std::endl;
		std::cout << "  --------------------------------";
		//checkin and warning if child not exists in db, checkin and warning if find open child(to avoid dead lock)
		checkDepd(elem);
		//fail to checkin if find same file with close-pending
		if (!checkClosePend(elem)) {
			std::cout << "  you have to close the close-pending file before check-in!" << std::endl;
			std::cout << "  --------------------------------" << std::endl;
			return false;
		}
		Key key = addElement(own, elem);
		if (key == "") {
			std::cout << "  --------------------------------" << std::endl;
			return false;
		}
		//path in message already has full path of client
		path from = elem.payLoad().path() +"/" + elem.name();
		path to = topath + "/" + key;
		if (!copyFile(from, to)) {
			std::cout << "  --------------------------------" << std::endl;
			return false;
		}
		db_[key].payLoad().path() = to;
		std::cout << "  --------------------------------" << std::endl;
		return true;
	}
	//----< check in a whole package >--------------------------------------------------

	inline bool CheckIn::checkInPackage(owner own, pName name) {
		std::cout << "\n  checking in package " << name << std::endl;
		std::cout << "  #-----------------------------" << std::endl;
		path to = toPath_ + "/" + name;
		if (_access(to.c_str(), 0) == -1) {
			std::cout << "  " << to << " is not existing, making it now" << std::endl;
			int flag = _mkdir(to.c_str());
			if (flag != 0) {
				std::cout << "  fail to make" << std::endl;
				return false;
			}
		}

		for (auto elem : elemVec_) {
			elem.payLoad().status() = true;
			// to: ../Checked-in/packageName
			if (!checkInFile(own, elem, to)) {
				return false;
			}
		}
		elemVec_.clear();
		return true;
	}
}