#pragma once
///////////////////////////////////////////////////////////////////////
// ServerPrototype.h - Console App that processes incoming messages  // 
// ver 2.0                                                           //
// Language:       C++, Visual Studio 2017                           //
// Platform:       Macbook pro, Windows 10 Home                      //
// Application:    Spring 2018 CSE687 Project#2                      //
// Author:         Yuan Liu, yliu219@syr.edu                         //
// Referrence:     Jim Fawcett, CSE687                               //
///////////////////////////////////////////////////////////////////////
/*
*  Package Operations:
* ---------------------
*  Package contains one class, Server, that contains a Message-Passing Communication
*  facility. It processes each message by invoking an installed callable object
*  defined by the message's command key.
*
*  Message handling runs on a child thread, so the Server main thread is free to do
*  any necessary background processing (none, so far).
*
*  Public Interface:
*  -----------------
*  Server(MsgPassingCommunication::EndPoint ep, const std::string& name, DbCore<PayLoad>& db);  // constructor
*  void start();																																								// start the Comm
*  void stop();																																									// stop Comm
*  void addMsgProc(Key key, ServerProc proc);																										// add a callable object
*  void processMessages();																																			// start message process thread
*  void postMessage(MsgPassingCommunication::Message msg);																			// post the given message
*  void receiveItem(MsgPassingCommunication::Message msg);																			// receive a file block
*  void showPayload();																																					// show PayLoad on server console
*  void serverViewFile(MsgPassingCommunication::Message msg);																		// respond to viewFile request
*  bool serverCheckIn();																																				// check in the given elements
*  void modify(MsgPassingCommunication::Message msg);																						// modify the specific element
*  void query(MsgPassingCommunication::Message msg);																						// query with given conditions
*  void withoutPar(MsgPassingCommunication::Message msg);																				// send back the keys of elements without parents
*  bool serverCheckOut(MsgPassingCommunication::Message msg);																		// respond to checkOut request
*  bool serverClose(MsgPassingCommunication::Message msg);																			// response to close request
*  bool clearDirs();																																						// clear the file storage
*  void checkDir();																																							// check in the dir exists
*  int removeDir(const char* dirPath);																													// remove the given dir
*  std::vector<std::string> getFileNames(std::string dir);																			// get all the file names in the given dir
*  MsgPassingCommunication::Message getMessage();																								// get a message
*  static Dirs getDirs(const SearchPath& path = storageRoot);																		// get dir in storage root
*  static Files getFiles(const SearchPath& path = storageRoot);																	// get files in storage root
*  std::vector<std::string> split(const std::string& str, const std::string& sep);							// split the given string with the given seperater
*  void processHelper(MsgPassingCommunication::Message msg);                                    // a helper function used to reduce line counts
*
*
*  Required Files:
* -----------------
*  ServerPrototype.h, ServerPrototype.cpp
*  Comm.h, Comm.cpp, IComm.h
*  Message.h, Message.cpp
*  FileSystem.h, FileSystem.cpp
*  Utilities.h
*
*  Maintenance History:
* ----------------------
*  ver 3.0 : 4/30/2018
*  - implement complete functionality for the server
*  ver 2.0 : 4/10/2018
*  - add getDirs() and getFiles() functions to server
*  ver 1.0 : 3/27/2018
*  - first release
*/
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <thread>
#include "../CppCommWithFileXfer/Message/Message.h"
#include "../CppCommWithFileXfer/MsgPassingComm/Comm.h"
#include "../RepositoryCore/RepositoryCore.h"
#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <direct.h>
#include <vector>

namespace Repository
{
  using File = std::string;
  using Files = std::vector<File>;
  using Dir = std::string;
  using Dirs = std::vector<Dir>;
  using SearchPath = std::string;
  using Key = std::string;
  using Msg = MsgPassingCommunication::Message;
  using ServerProc = std::function<Msg(Msg)>;
  using MsgDispatcher = std::unordered_map<Key,ServerProc>;
  
  const SearchPath storageRoot = "../Storage";  // root for all server file storage
	const std::string serverStorage = "../ServerStorage";  // storage for receiving check in files 
  const MsgPassingCommunication::EndPoint serverEndPoint("localhost", 8080);  // listening endpoint
	const std::vector<std::string> simpleMsgs{"Checkin", "Close"};
	const std::vector<std::string> dirs_files{ "getOutDirs", "getOutFiles", "getBrowseDirs", "getBrowseFiles", "getPopDirs", "getPopFiles" };
	const std::vector<std::string> commands{ "Connect", "receiveEnd", "modify", "query", "closeElem", "Checkout", "viewFile" };

  class Server
  {
  public:
    Server(MsgPassingCommunication::EndPoint ep, const std::string& name, DbCore<PayLoad>& db);
    void start();
    void stop();
    void addMsgProc(Key key, ServerProc proc);
    void processMessages();
    void postMessage(MsgPassingCommunication::Message msg);
		void receiveItem(MsgPassingCommunication::Message msg);
		void showPayload();
		void serverViewFile(MsgPassingCommunication::Message msg);
		bool serverCheckIn();
		void modify(MsgPassingCommunication::Message msg);
		void query(MsgPassingCommunication::Message msg);
		void withoutPar(MsgPassingCommunication::Message msg);
		bool serverCheckOut(MsgPassingCommunication::Message msg);
		bool serverClose(MsgPassingCommunication::Message msg);
		bool clearDirs();
		void checkDir();
		int removeDir(const char* dirPath);
		std::vector<std::string> getFileNames(std::string dir);
    MsgPassingCommunication::Message getMessage();
    static Dirs getDirs(const SearchPath& path = storageRoot);
    static Files getFiles(const SearchPath& path = storageRoot);
		std::vector<std::string> split(const std::string& str, const std::string& sep);
		void processHelper(MsgPassingCommunication::Message msg);
  private:
    MsgPassingCommunication::Comm comm_;
    MsgDispatcher dispatcher_;
    std::thread msgProcThrd_;
		RepositoryCore repo_;
		//std::vector<DbElement<Repository::PayLoad> > elemVec_;
		int packageCount_;
		std::string packageName_;
		std::string namespace_;
		// set when connect!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		std::string user_ = "Jim";
		std::vector<std::string> checkOutPaths;
  };
  //----< initialize server endpoint and give server a name >----------

  inline Server::Server(MsgPassingCommunication::EndPoint ep, const std::string& name, DbCore<PayLoad>& db)
    : comm_(ep, name), repo_(db), packageCount_(0) {}

  //----< start server's instance of Comm >----------------------------

  inline void Server::start()
  {
    comm_.start();
  }
  //----< stop Comm instance >-----------------------------------------

  inline void Server::stop()
  {
    if(msgProcThrd_.joinable())
      msgProcThrd_.join();
    comm_.stop();
  }
  //----< pass message to Comm for sending >---------------------------

  inline void Server::postMessage(MsgPassingCommunication::Message msg)
  {
    comm_.postMessage(msg);
  }
  //----< get message from Comm >--------------------------------------

  inline MsgPassingCommunication::Message Server::getMessage()
  {
    Msg msg = comm_.getMessage();
    return msg;
  }
  //----< add ServerProc callable object to server's dispatcher >------

  inline void Server::addMsgProc(Key key, ServerProc proc)
  {
    dispatcher_[key] = proc;
  }
  //----< start processing messages on child thread >------------------

  inline void Server::processMessages()
  {
    auto proc = [&]()
    {
      if (dispatcher_.size() == 0) {
        std::cout << "\n  no server procs to call";
        return;
      }
      while (true) {
        Msg msg = getMessage();
        std::cout << "\n  received message: " << msg.command() << " from " << msg.from().toString();
        if (msg.containsKey("verbose")) {
          std::cout << "\n";
          msg.show();
        }
        if (msg.command() == "serverQuit") break;
				if (std::find(commands.begin(), commands.end(), msg.command()) != commands.end()) {
					processHelper(msg);
					continue;
				}
				if (msg.command() == "getPath") {
					packageCount_ = stoi(msg.value("pacCount"));
					packageName_ = msg.value("pacName");
					namespace_ = msg.value("namespace");
				}
				Msg reply;
				if (std::find(dirs_files.begin(), dirs_files.end(), msg.command()) != dirs_files.end()) {
					std::string cmd = msg.command();
					if (msg.command().find("Files") != std::string::npos)
						reply = dispatcher_["getFiles"](msg);
					if (msg.command().find("Dirs") != std::string::npos)
						reply = dispatcher_["getDirs"](msg);
					reply.command(cmd);
				}
        else if (msg.attributes().size() != 0) reply = dispatcher_[msg.command()](msg);
        if (msg.to().port != msg.from().port)  // avoid infinite message loop
        {
          postMessage(reply);
          msg.show();
        }
        else
          std::cout << "\n  server attempting to post to self";
      }
      std::cout << "\n  server message processing thread is shutting down";
    };
    std::thread t(proc);
    std::cout << "\n  starting server thread to process messages";
    msgProcThrd_ = std::move(t);
		msgProcThrd_.join();
  }
	//----< helper function >--------------------------------------------

	inline void Server::processHelper(MsgPassingCommunication::Message msg) {
		if (msg.command() == "Connect") {
			user_ = msg.value("owner");
		}
		if (msg.command() == "receiveEnd") {
			msg.show();
			receiveItem(msg);
		}
		if (msg.command() == "modify") {
			msg.show();
			modify(msg);
		}
		if (msg.command() == "query") {
			msg.show();
			query(msg);
			withoutPar(msg);
		}
		if (msg.command() == "closeElem") {
			serverClose(msg);
			showPayload();
		}
		if (msg.command() == "Checkout") {
			msg.show();
			serverCheckOut(msg);
		}
		if (msg.command() == "viewFile") {
			msg.show();
			serverViewFile(msg);
		}
	}

	//----< response to check in >---------------------------------------

	inline bool Server::serverCheckIn() {
		repo_.nspace() = namespace_;
		bool suc = repo_.checkIn(user_, packageName_);
		if (!suc)
			std::cout << "check in failed!" << std::endl;
		repo_.showRepo();
		repo_.elemVec().clear();
		return suc;
	}
	//----< check out the file in the given message >--------------------

	inline bool Server::serverCheckOut(MsgPassingCommunication::Message msg) {
		std::string name = msg.value("name");
		std::string clientPath = msg.value("path"); // send path = msg.value("sendPath"), receive path = msg.value("receivePath") + "/" + msg.file()
		bool dep = msg.value("dep") == "True";
		if (dep) std::cout << "with dependency" << std::endl;
		else std::cout << "without dependency" << std::endl;
		checkOutPaths.clear();
		bool suc = repo_.checkOut(name, dep, checkOutPaths);
		for (auto file : checkOutPaths) {
			Msg reply;
			reply.from(msg.to());
			reply.to(msg.from());
			reply.file(file.substr(file.find_last_of('/') + 1, file.size()));
			reply.attribute("receivePath", clientPath);
			reply.attribute("sendPath", file);
			reply.attribute("type", "checkOut");
			postMessage(reply);
		}
		return true;
	}
	//----< close the given file >---------------------------------------

	inline bool Server::serverClose(MsgPassingCommunication::Message msg) {
		bool suc = repo_.close(repo_.db()[msg.value("name")]);
		Msg reply;
		reply.from(msg.to());
		reply.to(msg.from());
		reply.command("replyClose");
		if (suc) reply.attribute("type", "Close succeed!");
		else reply.attribute("type", "Close failed! Change it to be close pending!");
		postMessage(reply);
		return suc;
	}
	//----< receive a file block >---------------------------------------

	inline void Server::receiveItem(MsgPassingCommunication::Message msg) {
		DbElement<Repository::PayLoad> elem = makeElement<Repository::PayLoad>(msg.file(), msg.value("description"));
		Repository::PayLoad pl;
		pl.path(msg.value("receivePath"));
		pl.owner(msg.value("owner"));
		for (auto cate : split(msg.value("categories"), ";"))
			pl.categories().push_back(cate);
		elem.payLoad(pl);
		for (auto dep : split(msg.value("dependency"), ";")) 
			elem.addChildKey(dep);
		repo_.elemVec().push_back(elem);
		Msg reply;
		reply.from(msg.to());
		reply.to(msg.from());
		reply.command("replyCheckIn");
		if (--packageCount_ == 0) {
			if (serverCheckIn()) reply.attribute("type", "Check in succeed!");
			else reply.attribute("type", "Check in failed! Please check server console log!");
			postMessage(reply);
		}
	}
	//----< split the given string >-------------------------------------

	inline std::vector<std::string> Server::split(const std::string& str, const std::string& sep) {
		std::vector<std::string> v;
		if (str.size() == 0) return v;
		std::string::size_type pos1, pos2;
		pos2 = str.find(sep);
		pos1 = 0;
		while (std::string::npos != pos2) {
			v.push_back(str.substr(pos1, pos2 - pos1));
			pos1 = pos2 + sep.size();
			pos2 = str.find(sep, pos1);
		}
		if (pos1 != str.length())
			v.push_back(str.substr(pos1));
		return v;
	}
	//----< show the PayLoad in server console >-------------------------

	inline void Server::showPayload() {
		DbElement<PayLoad> elem;
		elem.payLoad().showDb(repo_.db());
	}
	//----< get file names in target dir >-------------------------------

	inline std::vector<std::string> Server::getFileNames(std::string dir) {
		std::vector<std::string> files;  
		_finddata_t file;
		long lf; 
		if ((lf = _findfirst(dir.c_str(), &file)) == -1) {
			std::cout << dir << " Can't find dir!" << std::endl;
		}
		else {
			while (_findnext(lf, &file) == 0) {
				if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
					continue;
				files.push_back(file.name);
			}
		}
		_findclose(lf);
		return files;
	}
	//----< clear the dirs >---------------------------------------------

	bool Server::clearDirs() {
		std::string serStorage = "../ServerStorage";
		std::string serCheckedIn = "../Checked-in";
		std::string serCheckOut = "../Checked-out";
		std::string cltCheckedOut = "../Client-Checked-Out";
		int i = 0, j = 0;
		std::vector<std::string> dirVec = {serStorage, serCheckedIn, serCheckOut, cltCheckedOut };
		for (auto path : dirVec) {
			if (_access(path.c_str(), 0) == 0) {
				std::cout << " removing dir." << std::endl;
				i = removeDir(path.c_str()); // 0 -> success, -1 -> fail
				if (i == -1) std::cout << " fail to remove" <<std::endl;
			}
			else
				std::cout << std::endl << " dir not exists!" << std::endl;
			j = _mkdir(path.c_str());
		}
		return (i == 0) && (j == 0);
	}
	//----< remove dir, inner dir, inner files recursively >-------------

	int Server::removeDir(const char* dirPath) {
		struct _finddata_t fb;
		char  path[250];
		long    handle;
		int  resultone;
		int   noFile;

		noFile = 0;
		handle = 0;

		strcpy_s(path, dirPath);
		strcat_s(path, "/*");

		handle = _findfirst(path, &fb);
		if (handle != 0)
		{
			while (0 == _findnext(handle, &fb))
			{
				noFile = strcmp(fb.name, "..");

				if (0 != noFile)
				{ 
					memset(path, 0, sizeof(path));
					strcpy_s(path, dirPath);
					strcat_s(path, "/");
					strcat_s(path, fb.name);
					if (fb.attrib == 16)
					{
						removeDir(path);
					}  
					else
					{
						remove(path);
					}
				}
			} 
			_findclose(handle);
		}
		resultone = _rmdir(dirPath);
		return  resultone;
	}
	//----< check the dirs exist or not >--------------------------------

	inline void Server::checkDir() {
		std::string serStorage = "../ServerStorage";
		std::string serCheckedIn = "../Checked-in";
		std::string serCheckOut = "../Checked-out";
		std::string cltCheckedOut = "../Client-Checked-Out";
		std::vector<std::string> dirVec = { serStorage, serCheckedIn, serCheckOut, cltCheckedOut };
		for (auto path : dirVec) {
			if (_access(path.c_str(), 0) == -1) {
				int i = _mkdir(path.c_str());
			}
		}
	}
	//----< response to the viewFile request >---------------------------

	void Server::serverViewFile(MsgPassingCommunication::Message msg) {
		DbCore<Repository::PayLoad> db = repo_.db();
		DbElement<Repository::PayLoad> elem = db[msg.value("filename")];
		std::string cate = "", dep = "";
		std::string status = elem.payLoad().status() ? (elem.payLoad().closePend() ? "open" : "close pending") : "close";
		Msg reply;
		reply.from(msg.to());
		reply.to(msg.from());
		reply.command("replyViewFile");
		reply.name(elem.name());
		reply.file(elem.name());
		reply.attribute("type", "replyViewFile");
		reply.attribute("status", status);
		reply.attribute("time", elem.dateTime().time());
		reply.attribute("des", elem.descrip());
		reply.attribute("owner", elem.payLoad().owner());
		reply.attribute("path", elem.payLoad().path());
		for (auto c : elem.payLoad().categories()) cate += ";" + c;
		reply.attribute("cate", cate.substr(1, cate.size()));
		for (auto d : elem.children()) dep += ";" + d;
		reply.attribute("dep", dep.size() > 0 ? dep.substr(1, dep.size()) : "");
		reply.attribute("receivePath", msg.value("receivePath"));
		reply.attribute("sendPath", elem.payLoad().path());
		postMessage(reply);
	}
	//----< response to the Modify request >-----------------------------

	void Server::modify(MsgPassingCommunication::Message msg) {
		MsgPassingCommunication::Message reply;
		reply.from(msg.to());
		reply.to(msg.from());
		reply.command("replyModify");
		if (!repo_.db()[msg.value("filename")].payLoad().status()) {
			reply.attribute("reply", "Can't modify a closed file!");
			postMessage(reply);
			return;
		}
		repo_.db()[msg.value("filename")].descrip() = msg.value("des");
		repo_.db()[msg.value("filename")].children() = split(msg.value("dep"), ";");
		repo_.db()[msg.value("filename")].payLoad().categories() = split(msg.value("cate"), ";");
		repo_.showRepo();
		repo_.db()[msg.value("filename")].payLoad().showDb(repo_.db());
		reply.attribute("reply", "Modify succeed!");
		postMessage(reply);
	}
	//----< query with given conditions >--------------------------------

	void Server::query(MsgPassingCommunication::Message msg) {
		std::vector<std::string> dep = split(msg.value("dep"), ";");
		std::string ans = "";
		std::string ver = msg.value("version");
		std::string name = msg.value("filename");
		std::string cate = msg.value("cate");
		std::string mode = msg.value("mode");
		auto hasCategory = [&cate](DbElement<PayLoad>& elem) {
			if (cate == "") return true;
			return (elem.payLoad()).hasCategory(cate);
		};
		Conditions<Repository::PayLoad> conds, conds1, conds2, all;
		conds.children(dep);
		conds.name(name);
		Query<Repository::PayLoad> q(repo_.db());
		if (mode == "and") {
			for (auto key : q.select(hasCategory).query_and(conds).query_ver(ver).keys()) {
				if (ans == "") ans = key;
				else ans += "; " + key;
			}
		}
		else {
			Query<Repository::PayLoad> q1(repo_.db());
			Query<Repository::PayLoad> q2(repo_.db());
			Query<Repository::PayLoad> q3(repo_.db());
			Query<Repository::PayLoad> q4(repo_.db());
			conds1.children(dep);
			conds2.name(name);
			q1.select(hasCategory);
			q2.select(conds2);
			q3.select(conds1);
			q4.select(all).query_ver(ver);
			for (auto key : q1.query_or(q2).query_or(q3).query_or(q4).keys()) {
				if (ans == "") ans = key;
				else ans += "; " + key;
			}
		}
		Msg reply;
		reply.from(msg.to());
		reply.to(msg.from());
		reply.command("replyQuery");
		reply.attribute("reply", ans);
		postMessage(reply);
	}
	//----< send back the keys of files without parents >----------------

	void Server::withoutPar(MsgPassingCommunication::Message msg) {
		DbCore<Repository::PayLoad> db = repo_.db();
		std::vector<std::string> keys = db.keys();
		std::string ans = "";
		for (auto key : db.keys()) {
			for (auto child : db[key].children())
				keys.erase(std::remove(keys.begin(), keys.end(), child), keys.end());
				continue;
		}
		for (auto key : keys) {
			if (ans == "") ans = key;
			else ans += "; " + key;
		}
		Msg reply;
		reply.from(msg.to());
		reply.to(msg.from());
		reply.command("replyWithoutPar");
		reply.attribute("reply", ans);
		postMessage(reply);
	}
}