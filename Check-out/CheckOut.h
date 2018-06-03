#pragma once
/////////////////////////////////////////////////////////////////////
// CheckOut.h   - Implements check out operation                   //
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
* -This package provides all of the functions needed to check out a file,
*  by set dep as true, the dependent files of targged file would alse be
*  checked out.
*
* Public Interface:
* -----------------
* CheckOut(DbCore<PayLoad>& db)																						//constructor with parameter
* bool checkFileOut(Key key, bool dep)																		//check out file
*																																						check out dependent files if dep true 
* bool copyFile(path from, path to)																				//copy a single file
* void BFS(std::queue<Key>& que, Keys& keys, DbCore<PayLoad> db)					//BFS to find all dependent files
* bool makeDir(path toDir)                                                //make a dir
*
* Required Files:
* ---------------
* NoSqlDb.h
* Version.h
* CheckIn.h
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
#include "../Check-in/CheckIn.h"
#include <direct.h>
#include <iostream>
#include <queue>
#include <io.h>
#define BUF_SIZE 256

namespace Repository {
	class CheckOut {
	public:
		using path = std::string;

		CheckOut(DbCore<PayLoad>& db) : db_(db) {}
	  bool checkFileOut(Key key, bool dep, std::vector<std::string>& filePaths);
		bool copyFile(path from, path to);
		void BFS(std::queue<Key>& que, Keys& keys, DbCore<PayLoad> db);
		bool makeDir(path toDir);
	private:
		DbCore<PayLoad>& db_;
		const path toPath_ = "../Checked-out";
	};
	//----< make a dir >-----------------------------------------------------------

	inline bool CheckOut::makeDir(path toDir) {
		if (_access(toDir.c_str(), 0) == -1) {
			std::cout << "  " << toDir << " is not existing, making it now" << std::endl;
			int flag = _mkdir(toDir.c_str());
			if (flag != 0) {
				std::cout << "  fail to make" << std::endl;
				std::cout << "  -------------------------" << std::endl;
				return false;
			}
		}
		return true;
	}
	//----< check out dependent files if dep true >--------------------------------

	inline bool CheckOut::checkFileOut(Key key, bool dep, std::vector<std::string>& filePaths) {
		std::cout << "\n  checking out file " << key << std::endl;
		std::cout << "  -------------------------" << std::endl;
		if (!db_.contains(key)) {
			std::cout << "  the element " << key << " not exists in database!" << std::endl;
			std::cout << "  -------------------------" << std::endl;
			return false;
		}
		path toDir = toPath_ + "/" + key + (dep?"-dep":"");
		if (!makeDir(toDir)) return false;
		Version *ver = new Version(db_);
		if (!dep) {
			std::string topath = toDir + "/" + ver->removeVersion(key);
			filePaths.push_back(topath);
			if (!copyFile(db_[key].payLoad().path(), topath)) {
				std::cout << "  -------------------------" << std::endl;
				return false;
			}
		}
		else {
			Keys keys;
			std::queue<Key> que;
			que.push(key);
			BFS(que, keys, db_);
			for (auto k : keys) {
				if (!db_.contains(k)) {
					std::cout << "  the element " << k << " not exists in database!" << std::endl;
				}
				else {
					std::string topath = toDir + "/" + ver->removeVersion(k);
					filePaths.push_back(topath);
					if (!copyFile(db_[k].payLoad().path(), topath)) {
						std::cout << "  -------------------------" << std::endl;
						return false;
					}
				}
			}
		}
		std::cout << "  -------------------------" << std::endl;
		return true;
	}
	//----< BFS to find all dependent files >--------------------------------------

	inline void CheckOut::BFS(std::queue<Key>& que, Keys& keys, DbCore<PayLoad> db) {
		while (!que.empty()) {
			Key key = que.front();
			que.pop();
			if (find(keys.begin(), keys.end(), key) == keys.end()) keys.push_back(key);
			for (auto k : db[key].children()) {
				if(find(keys.begin(), keys.end(), k) == keys.end())
					que.push(k);
			}
		}
	}
	//----< copy a single file >---------------------------------------------------

	inline bool CheckOut::copyFile(path from, path to) {
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
		printf("  copying file...%d bytes copied\n", len);
		fclose(in_file);
		fclose(out_file);
		return true;
	}
}