#ifndef PAYLOAD_H
#define PAYLOAD_H
/////////////////////////////////////////////////////////////////////
// PayLoad.h   - Implements PayLoad                                //
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
* -This package provides all of the functions needed for a payload,
*  the status is true for open file, the closePend is true for not
*  close-pending file.
*
* Public Interface:
* -----------------
* PayLoad()																																															//constructor
* PayLoad(const std::string& val)																																				//constructor with parameter
* static void identify(std::ostream& out = std::cout)																										//show file name																	
* PayLoad& operator=(const std::string& val)																														//overload operator =
* operator std::string()																																								//overload operator ()
* std::string path() const																																							//get/set function for path_
*	std::string& path()
*	void path(const std::string& value)
*	bool status() const																																										//get/set function for status_
*	bool& status()
*	void status(const bool& status)
*	bool closePend() const																																								//get/set function for closePend
*	bool& closePend()
*	void closePend(const bool& closePend)
*	std::string owner() const																																							//get/set function for owner_
*	std::string& owner()
*	void owner(const std::string& owner)
*	std::vector<std::string>& categories()																																//get/set function for categories_
*	std::vector<std::string> categories() const
* bool hasCategory(const std::string& cat)																															//find targged category
* Sptr toXmlElement()																																										//read payload from Xml
* static PayLoad fromXmlElement																																					//load payload in Xml
* static void showPayLoadHeaders(std::ostream& out = std::cout)																					//display header											
* static void showElementPayLoad(NoSqlDb::DbElement<PayLoad>& elem, std::ostream& out = std::cout);			//display single element payload
* static void showDb(NoSqlDb::DbCore<PayLoad>& db, std::ostream& out = std::cout);											//display complete payload										
*
* Required Files:
* ---------------
* XmlDocument.h
* XmlElement.h
* DbCore.h
* Definitions.h
* IPayLoad.h
*
* Build Process:
* --------------
* devenv SoftwareRepository.sln /rebuild debug
*
* Maintenance History:
* --------------------
* ver 1.0 : 05 Mar 2018
*/

#include <string>
#include <vector>
#include <iostream>
#include "../NoSqlDb/XmlDocument/XmlDocument/XmlDocument.h"
#include "../NoSqlDb/XmlDocument/XmlElement/XmlElement.h"
#include "../NoSqlDb/DbCore/DbCore.h"
#include "../NoSqlDb/DbCore/Definitions.h"
#include "../NoSqlDb/PayLoad/IPayLoad.h"

namespace Repository {
	using Sptr = NoSqlDb::Sptr;

	class PayLoad : public NoSqlDb::IPayLoad<PayLoad>
	{
	public:
		PayLoad() = default;
		PayLoad(const std::string& val) : path_(val) {}
		static void identify(std::ostream& out = std::cout);
		PayLoad& operator=(const std::string& val)
		{
			path_ = val;
			return *this;
		}
		operator std::string() { return path_; }

		std::string path() const { return path_; }
		std::string& path() { return path_; }
		void path(const std::string& value) { path_ = value; }

		bool status() const { return status_; }
		bool& status() { return status_; }
		void status(const bool& status) { status_ = status; }

		bool closePend() const { return closePend_; }
		bool& closePend() { return closePend_; }
		void closePend(const bool& closePend) { closePend_ = closePend; }

		std::string owner() const { return owner_; }
		std::string& owner() { return owner_; }
		void owner(const std::string& owner) { owner_ = owner; }

		std::vector<std::string>& categories() { return categories_; }
		std::vector<std::string> categories() const { return categories_; }

		bool hasCategory(const std::string& cat)
		{
			return std::find(categories().begin(), categories().end(), cat) != categories().end();
		}

		Sptr toXmlElement();
		static PayLoad fromXmlElement(Sptr elem);

		static void showPayLoadHeaders(std::ostream& out = std::cout);
		static void showElementPayLoad(NoSqlDb::DbElement<PayLoad>& elem, std::ostream& out = std::cout);
		static void showDb(NoSqlDb::DbCore<PayLoad>& db, std::ostream& out = std::cout);
	private:
		bool status_;
		bool closePend_;
		std::string path_;
		std::string owner_;
		std::vector<std::string> categories_;
	};

	//----< show file name >---------------------------------------------

	inline void PayLoad::identify(std::ostream& out)
	{
		out << "\n  \"" << __FILE__ << "\"";
	}
	//----< create XmlElement that represents PayLoad instance >---------
	/*
	* - Required by Persist<PayLoad>
	*/
	inline Sptr PayLoad::toXmlElement()
	{
		Sptr sPtr = XmlProcessing::makeTaggedElement("payload");
		XmlProcessing::XmlDocument doc(makeDocElement(sPtr));
		Sptr sPtrSta = XmlProcessing::makeTaggedElement("status", status_?"true":"false");
		sPtr->addChild(sPtrSta);
		Sptr sPtrVal = XmlProcessing::makeTaggedElement("path", path_);
		sPtr->addChild(sPtrVal);
		Sptr sPtrOwn = XmlProcessing::makeTaggedElement("owner", owner_);
		sPtr->addChild(sPtrOwn);
		Sptr sPtrCats = XmlProcessing::makeTaggedElement("categories");
		sPtr->addChild(sPtrCats);
		for (auto cat : categories_)
		{
			Sptr sPtrCat = XmlProcessing::makeTaggedElement("category", cat);
			sPtrCats->addChild(sPtrCat);
		}
		return sPtr;
	}
	//----< create PayLoad instance from XmlElement >--------------------
	/*
	* - Required by Persist<PayLoad>
	*/
	inline PayLoad PayLoad::fromXmlElement(Sptr pElem)
	{
		PayLoad pl;
		for (auto pChild : pElem->children())
		{
			std::string tag = pChild->tag();
			std::string val = pChild->children()[0]->value();
			if (tag == "status")
			{
				pl.status(val == "true"?true:false);
			}
			if (tag == "path")
			{
				pl.path(val);
			}
			if (tag == "owner")
			{
				pl.owner(val);
			}
			if (tag == "categories")
			{
				std::vector<Sptr> pCategories = pChild->children();
				for (auto pCat : pCategories)
				{
					pl.categories().push_back(pCat->children()[0]->value());
				}
			}
		}
		return pl;
	}
	/////////////////////////////////////////////////////////////////////
	// PayLoad display functions

	inline void PayLoad::showPayLoadHeaders(std::ostream& out)
	{
		out << "\n  ";
		out << std::setw(10) << std::left << "Name";
		out << std::setw(10) << std::left << "Status";
		out << std::setw(10) << std::left << "Owner";
		out << std::setw(45) << std::left << "Payload Path";
		out << std::setw(25) << std::left << "Categories";
		out << "\n  ";
		out << std::setw(10) << std::left << "--------";
		out << std::setw(10) << std::left << "--------";
		out << std::setw(10) << std::left << "--------";
		out << std::setw(45) << std::left << "---------------------------------------";
		out << std::setw(25) << std::left << "-----------------------";
	}
	/////////////////////////////////////////////////////////////////////
	// PayLoad display functions

	inline void PayLoad::showElementPayLoad(NoSqlDb::DbElement<Repository::PayLoad>& elem, std::ostream& out)
	{
		out << "\n  ";
		out << std::setw(10) << std::left << elem.name().substr(0, 8);
		out << std::setw(10) << std::left << (elem.payLoad().status()?"true":"false");
		out << std::setw(10) << std::left << elem.payLoad().owner().substr(0, 8);
		out << std::setw(45) << std::left << elem.payLoad().path().substr(0, 43);
		for (auto cat : elem.payLoad().categories())
		{
			out << cat << " ";
		}
	}
	/////////////////////////////////////////////////////////////////////
	// PayLoad display functions

	inline void PayLoad::showDb(NoSqlDb::DbCore<PayLoad>& db, std::ostream& out)
	{
		showPayLoadHeaders(out);
		for (auto item : db)
		{
			NoSqlDb::DbElement<PayLoad> temp = item.second;
			PayLoad::showElementPayLoad(temp, out);
		}
	}
}
#endif
