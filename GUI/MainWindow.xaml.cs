/////////////////////////////////////////////////////////////////////
// MainWindow.xaml.cs   - Graphical User Interface                 //
// ver 1.0                                                         //
// Language:    C#, Visual Studio 2017                             //
// Platform:    Macbook pro, Windows 10 Home                       //
// Application: Spring 2018 CSE687 Project#3                       //
// Author:      Yuan Liu, yliu219@syr.edu                          //
// Referrence:  Jim Fawcett, CSE687                                //
/////////////////////////////////////////////////////////////////////
/*
* NOTE:
* -------------------
* -This package provides all of the functions for graphical user
*  interface, although most functions of GUI still needs to be 
*  implemented. For this version, the function for View Text Button
*  has been implemented.
*  
* Package Operations:
* -------------------
* -Connect button connect the server, and set the user.
* -Disconnect button break the connection to server.
* -Check in
*  Choose a file by clicking Choose File.
*  Add dependency by double click file listed by Dependency button. You
*  can add more than one dependency. Click del would delete the last dependency.
*  Input Description, categories and Owner which are metadata.
*  Click Add Item to add the current element to Group, you can add serveral
*  more elements to the current Group in the same way. Click cancel would 
*  clear current Group.
*  Input Namespace and Group Name, then click Check in, all the elements in 
*  current Group would be checked in.
* -Modify
*  Choose a file by clicking Choose File.
*  You can modify the value of Dependency in the same way of Check in, you
*  can also edit Description and Categories, then you click Modify, the 
*  information in the database of the chosen element would be modified.
* -Close
*  Choose a file by clicking Choose File.
*  Click close to close the chosen file. If the chosen file has open/close-pending
*  child, it can't close, there would pop up a message, and the status of the
*  chosen file would transfer to close-pending. After close all the open/close-pending
*  children, the chosen file can be closed.
* -Check out
*  Choose a file by clicking Choose File.
*  If you want to check out the chosen file with dependency, you need to select
*  Dependent Files.
*  Choose a path where the output files would go by clicking Choose Path.
*  Now, click Check Out and the chosen files would be checked out to the chosen path.
* -Browse
*  Choose a file by clicking Choose File.
*  Click View, the metadata would be shown on GUI and the content of chosen file would
*  be shown in the pop up window named READER.
*  The chosen file would be download and you can find it in Project4/Client-Checked-Out.
* -Query
*  Choose a dependent file by clicking Choose Dependency.
*  For Adding Category, FileName, Version, first click the left ComboBox to specify
*  a field, then input the value, click Add at last, the value would be added to
*  the chosen field.
*  After inputing all the conditions, click the right ComboBox to choose the mode(and/or),
*  then click Query, the result would be shown in the listbox.
*  NOTE: The version should be a number.
*
* Public Interface:
* -----------------
* There is no public functions.
*
* Required Files:
* ---------------
* Translate.h
* Translate.cpp
*
* Build Process:
* --------------
* devenv Project4.sln /rebuild debug
*
* Maintenance History:
* --------------------
* ver 1.0 : 10 Apr 2018
* 
* ver 2.0 : 30 Apr 2018
* implement a bounch of functions which not implemented
* in ver 1.0. Check in, Check out, Browse, Query.
*/

using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Threading;
using System.Drawing;
using MsgPassingCommunication;
using System.Timers;

namespace GUI
{
  /// <summary>
  /// Interaction logic for MainWindow.xaml
  /// </summary>
  /// 
  public partial class MainWindow : Window
  {
    public MainWindow()
    {
      InitializeComponent();
    }

    private struct element
    {
      public string name { get; set; }
      public string dep { get; set; }
      public string des { get; set; }
      public string owner { get; set; }
      public string cate { get; set; }
      public string path { get; set; }
    }

    private Stack<string> outPathStack_ = new Stack<string>();
    private Stack<string> inPathStack_ = new Stack<string>();
    private Stack<string> browsePathStack_ = new Stack<string>();
    private string filePathToCheckin;
    private string packageName_;
    private string namespace_;
    private int serverPortNum = 8080;
    private int showTime = 1000;
    private string machineAddress = "localhost";
    private Translater translater;
    private CsEndPoint endPoint_;
    private Thread rcvThrd = null;
    private Dictionary<string, element> checkInDic
      = new Dictionary<string, element>();
    private Dictionary<string, Action<CsMessage>> dispatcher_
      = new Dictionary<string, Action<CsMessage>>();
    //----< process the received messages >---------------------------

    private void processMessages()
    {
      ThreadStart thrdProc = () =>
      {
        while (true)
        {
          CsMessage msg = translater.getMessage();
          if (msg.attributes.Count != 0)
          {
            Console.Write("\n\n  received message: " + msg.value("command"));
            string cmd_ = msg.value("command");
            Action<string> statusAndFlow = (string cmd) =>
            {
              statusBar.Text = "Received a " + cmd + " message.";
              MessageFlows.Items.Add("Received message: " + cmd);
            };
            Dispatcher.Invoke(statusAndFlow, new Object[] { cmd_ });
            string msgId = msg.value("command");
            if (dispatcher_.ContainsKey(msgId))
              dispatcher_[msgId].Invoke(msg);
          }
        }
      };
      rcvThrd = new Thread(thrdProc);
      rcvThrd.IsBackground = true;
      rcvThrd.Start();
    }
    //----< add client processing for message with key >---------------

    private void addClientProc(string key, Action<CsMessage> clientProc)
    {
      dispatcher_[key] = clientProc;
    }

    private void t_Elapsed(object sender, ElapsedEventArgs e)
    {
      this.Dispatcher.Invoke(new Action(() =>
      {
        this.Close();
      }), null);
    }
    //----< load ReceiveEnd processing into dispatcher dictionary >---

    private void DispatcherReceiveEnd()
    {
      Action<CsMessage> getFiles = (CsMessage rcvMsg) =>
      {
        string name = rcvMsg.value("file").Split('/').Last();
        string path = rcvMsg.value("receivePath") + '/' + name;
        try
        {
          String line = System.IO.File.ReadAllText(path);
          Action<string, string> show = (string path_, string line_) =>
          {
            showFile(path_, line_);
          };
          Dispatcher.Invoke(show, new Object[] { path, line });
          if (rcvMsg.value("type") == "checkOut") displayCheckOut(rcvMsg);
          if (rcvMsg.value("type") == "replyViewFile") displayMetaData(rcvMsg);
        }
        catch (Exception ex)
        {
          Console.Write(ex);
        }
      };
      addClientProc("receiveEnd", getFiles);
    }
    //----< process the message replyModify >-------------------------

    private void DispatcherReplyModify()
    {
      Action<CsMessage> replyModify = (CsMessage rcvMsg) =>
      {
        Action<CsMessage> display = (CsMessage msg) => 
        {
          AutoClosingMessageBox.Show(msg.value("reply"), "Message", showTime);
          FileToModify.Text = "";
          return;
        };
        Dispatcher.Invoke(display, new Object[] { rcvMsg });
      };
      addClientProc("replyModify", replyModify);
    }
    //----< process the message replyCheckIn >------------------------

    private void DispatcherReplyCheckIn()
    {
      Action<CsMessage> replyCheckIn = (CsMessage rcvMsg) =>
      {
        Action<string> statusAndFlow = (string reply) =>
        {
          AutoClosingMessageBox.Show(reply, "Message", showTime);
          return;
        };
        Dispatcher.Invoke(statusAndFlow, new Object[] { rcvMsg.value("type") });
      };
      addClientProc("replyCheckIn", replyCheckIn);
    }
    //----< process the message replyClose >--------------------------

    private void DispatcherReplyClose()
    {
      Action<CsMessage> ReplyClose = (CsMessage rcvMsg) =>
      {
        Action<string> statusAndFlow = (string reply) =>
        {
          AutoClosingMessageBox.Show(reply, "Message", showTime);
          return;
        };
        Dispatcher.Invoke(statusAndFlow, new Object[] { rcvMsg.value("type") });
      };
      addClientProc("replyClose", ReplyClose);
    }
    //----< process the message replyQuery >--------------------------

    private void DispatcherReplyQuery()
    {
      Action<CsMessage> replyQuery = (CsMessage rcvMsg) =>
      {
        Action<CsMessage> display = (CsMessage msg) =>
        {
          if (msg.value("reply") == "") QueryOutput.Items.Add("Found nothing.");
          else QueryOutput.Items.Add("Found: " + msg.value("reply"));
        };
        Dispatcher.Invoke(display, new Object[] { rcvMsg });
      };
      addClientProc("replyQuery", replyQuery);
    }
    //----< process the message replyWithoutPar >---------------------

    private void DispatcherReplyWithoutPar()
    {
      Action<CsMessage> ReplyWithoutPar = (CsMessage rcvMsg) =>
      {
        Action<CsMessage> display = (CsMessage msg) =>
        {
          if (msg.value("reply") == "") QueryWithout.Items.Add("Found nothing.");
          else QueryWithout.Items.Add("Found: " + msg.value("reply"));
        };
        Dispatcher.Invoke(display, new Object[] { rcvMsg });
      };
      addClientProc("replyWithoutPar", ReplyWithoutPar);
    }
    //----< display the metadata in Browse >--------------------------

    private void displayMetaData(CsMessage msg_)
    {
      Action<CsMessage> displayMeta = (CsMessage msg) =>
      {
        BrowseName.Text = msg.value("name");
        BrowseStatus.Text = msg.value("status");
        BrowseDateTime.Text = msg.value("time");
        BrowseDescription.Text = msg.value("des");
        BrowseOwner.Text = msg.value("owner");
        BrowsePath.Text = msg.value("path");
        BrowseCategories.Text = msg.value("cate");
        BrowseDep.Text = msg.value("dep");
      };
      Dispatcher.Invoke(displayMeta, new Object[] { msg_ });
    }
    //----< display the checkout in Check-Out >-----------------------

    private void displayCheckOut(CsMessage msg)
    {
      Action<CsMessage> display = (CsMessage msg_) =>
      {
        output.Items.Add("Checked out file: " + msg_.value("file"));
      };
      Dispatcher.Invoke(display, new Object[] { msg });
    }
    //----< load all dispatcher processing >---------------------------

    private void loadDispatcher()
    {
      DispatcherReplyModify();
      DispatcherReceiveEnd();
      DispatcherReplyPath();
      DispatcherReplyQuery();
      DispatcherReplyWithoutPar();
      DispatcherReplyCheckIn();
      DispatcherReplyClose();
    }
    //----< start Comm, fill window display with dirs and files >------

    private void Window_Loaded(object sender, RoutedEventArgs e)
    {
      Console.Title = "Client" ;
      Console.Write("\n  Testing Client Prototype");
      Console.Write("\n ==========================");
      Console.Write("\n");
      CheckInButton.IsEnabled = false;
      ViewTextButton.IsEnabled = false;
      ModifyButton.IsEnabled = false;
      CloseButton.IsEnabled = false;
      CheckoutButton.IsEnabled = false;
      endPoint_ = new CsEndPoint();                   // start Comm
      endPoint_.machineAddress = machineAddress;
      endPoint_.port = new Random().Next(10000);
      translater = new Translater();
      translater.listen(endPoint_);
      processMessages();                              // start processing messages
      loadDispatcher();                               // load dispatcher
      test();
    }
    //----< strip off name of first part of path >---------------------

    private string removeFirstDir(string path)
    {
      string modifiedPath = path;
      int pos = path.IndexOf("/");
      modifiedPath = path.Substring(pos + 1, path.Length - pos - 1);
      return modifiedPath;
    }
    //----< clear TextBox when it get focus >-------------------------

    private void TextBox_GotFocus(object sender, RoutedEventArgs e)
    {
      return;
    }
    //----< respond to mouse click on ViewTextButton >----------------

    private void ViewTextButton_Click(object sender, RoutedEventArgs e)
    {
      CsEndPoint serverEndPoint = new CsEndPoint();
      serverEndPoint.machineAddress = machineAddress;
      serverEndPoint.port = serverPortNum;
      CsMessage msg = new CsMessage();
      msg.add("to", CsEndPoint.toString(serverEndPoint));
      msg.add("from", CsEndPoint.toString(endPoint_));
      msg.add("command", "viewFile");
      msg.add("filename", ChooseFileToBrowse.Text);
      msg.add("receivePath", System.IO.Path.GetFullPath("../../../ViewTextStorage"));
      translater.postMessage(msg);
      ViewTextButton.IsEnabled = false;
    }
    //----< respond to mouse click on ConnectButton >-----------------

    private void ConnectButton_Click(object sender, RoutedEventArgs e)
    {
      ConnectButton.IsEnabled = false;
      machineAddress = Address.Text;
      serverPortNum = Convert.ToInt32(portNumber.Text);
      CsEndPoint serverEndPoint = new CsEndPoint();
      serverEndPoint.machineAddress = machineAddress;
      serverEndPoint.port = serverPortNum;
      CsMessage msg = new CsMessage();
      msg.add("to", CsEndPoint.toString(serverEndPoint));
      msg.add("from", CsEndPoint.toString(endPoint_));
      msg.add("command", "Connect");
      msg.add("owner", ConnectUser.Text);
      translater.postMessage(msg);
    }
    //----< respond to mouse click on DisconnectButton >--------------

    private void DisconnectButton_Click(object sender, RoutedEventArgs e)
    {
      CsEndPoint serverEndPoint = new CsEndPoint();
      serverEndPoint.machineAddress = machineAddress;
      serverEndPoint.port = serverPortNum;
      CsMessage msg = new CsMessage();
      msg.add("to", CsEndPoint.toString(serverEndPoint));
      msg.add("from", CsEndPoint.toString(endPoint_));
      msg.add("command", "serverQuit");
      translater.postMessage(msg);
    }
    //----< respond to mouse click on CheckinButton >-----------------

    private void CheckinButton_Click(object sender, RoutedEventArgs e)
    {
      CheckInButton.IsEnabled = false;
      DepFileName.Text = "";
      InFileName.Text = "";
      if (GroupName.Text == "")
      {
        AutoClosingMessageBox.Show("Please fill in the Group Name", "Error", showTime);
        return;
      }
      packageName_ = GroupName.Text;
      namespace_ = Namespace.Text;
      CsEndPoint serverEndPoint = new CsEndPoint();
      serverEndPoint.machineAddress = machineAddress;
      serverEndPoint.port = serverPortNum;
      CsMessage msg_ = new CsMessage();
      msg_.add("to", CsEndPoint.toString(serverEndPoint));
      msg_.add("from", CsEndPoint.toString(endPoint_));
      msg_.add("command", "getPath");
      msg_.add("pacName", packageName_);
      msg_.add("namespace", namespace_);
      msg_.add("pacCount", checkInDic.Count.ToString());
      translater.postMessage(msg_);

      GroupBox.Items.Clear();
    }
    //----< respond to the message replyPath >------------------------

    private void DispatcherReplyPath()
    {
      Action<CsMessage> replyPath = (CsMessage rcvMsg) =>
      {
        CsEndPoint serverEndPoint = new CsEndPoint();
        serverEndPoint.machineAddress = machineAddress;
        serverEndPoint.port = serverPortNum;

        foreach (var item in checkInDic)
        {
          element ele = item.Value;
          CsMessage msg = new CsMessage();
          msg.add("to", CsEndPoint.toString(serverEndPoint));
          msg.add("from", CsEndPoint.toString(endPoint_));
          msg.add("command", "CheckInFile");
          msg.add("file", ele.name);
          msg.add("description", ele.des);
          msg.add("owner", ele.owner);
          msg.add("dependency", ele.dep);
          msg.add("categories", ele.cate);
          msg.add("sendPath", ele.path);
          msg.add("receivePath", rcvMsg.value("receivePath"));
          translater.postMessage(msg);
        }
        checkInDic.Clear();
      };
      addClientProc("replyPath", replyPath);
    }
    //----< respond to mouse click on CloseButton >-------------------

    private void CloseButton_Click(object sender, RoutedEventArgs e)
    {
      CloseButton.IsEnabled = false;
      if (FileToClose.Text == "") return;
      CsEndPoint serverEndPoint = new CsEndPoint();
      serverEndPoint.machineAddress = machineAddress;
      serverEndPoint.port = serverPortNum;
      CsMessage msg = new CsMessage();
      msg.add("to", CsEndPoint.toString(serverEndPoint));
      msg.add("from", CsEndPoint.toString(endPoint_));
      msg.add("command", "closeElem");
      msg.add("name", FileToClose.Text);
      translater.postMessage(msg);
    }
    //----< respond to mouse click on CheckoutButton >----------------

    private void CheckoutButton_Click(object sender, RoutedEventArgs e)
    {
      CheckoutButton.IsEnabled = false;
      output.Items.Clear();
      string dep = Dependency.IsChecked.ToString();
      CsEndPoint serverEndPoint = new CsEndPoint();
      serverEndPoint.machineAddress = machineAddress;
      serverEndPoint.port = serverPortNum;
      CsMessage msg = new CsMessage();
      msg.add("to", CsEndPoint.toString(serverEndPoint));
      msg.add("from", CsEndPoint.toString(endPoint_));
      msg.add("command", "Checkout");
      msg.add("dep",dep);
      msg.add("name", ChooseOutFile.Text);
      msg.add("path", CheckOutPath.Text);
      translater.postMessage(msg);
    }
    //----< respond to mouse click on ExitItem >----------------------

    private void ExitItem_Click(object sender, RoutedEventArgs e)
    {
      this.Close();
    }
    //----< respond to mouse click on ReadMeItem >--------------------

    private void ReadMeItem_Click(object sender, RoutedEventArgs e)
    {
      string path = "../../../ReadMe.txt";
      try
      {
        String line = System.IO.File.ReadAllText(path);
        Action<string, string> show = (string path_, string line_) =>
        {
          showFile(path_, line_);
        };
        Dispatcher.Invoke(show, new Object[] { path, line });
      }
      catch (Exception ex)
      {
        Console.Write(ex);
      }
    }
    //----< respond to mouse click on ContactItem >-------------------

    private void ContactItem_Click(object sender, RoutedEventArgs e)
    {
      string path = "../../../Contact.txt";
      try
      {
        String line = System.IO.File.ReadAllText(path);
        Action<string, string> show = (string path_, string line_) =>
        {
          showFile(path_, line_);
        };
        Dispatcher.Invoke(show, new Object[] { path, line });
      }
      catch (Exception ex)
      {
        Console.Write(ex);
      }
    }
    //----< respond to mouse click on Choose File >-------------------

    private void ChooseFileButton_Click(object sender, RoutedEventArgs e)
    {
      Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
      dlg.InitialDirectory = System.IO.Path.GetFullPath("../../../Storage");
      Nullable<bool> result = dlg.ShowDialog();
      string filename = "", path = "";
      if (result == true)
      {
        path = dlg.FileName;
        filename = path.Split('\\').Last();
      }
      InFileName.Text = filename;
      filePathToCheckin = path;
    }
    //----< respond to mouse click on Dependency >--------------------

    private void DepChooseButton_Click(object sender, RoutedEventArgs e)
    {
      LocalBrowser lb = new LocalBrowser("DepFileName", -1);
      lb.Owner = this;
      lb.Show();
    }
    //----< respond to mouse click on del >---------------------------

    private void DelDepButton_Click(object sender, RoutedEventArgs e)
    {
      string dep = DepFileName.Text;
      if (!dep.Contains(";"))
        DepFileName.Text = "";
      else
      {
        DepFileName.Text = dep.Substring(0, dep.LastIndexOf(";"));
      }
    }
    //----< respond to mouse click on Add Item >----------------------

    private void AddItemButton_Click(object sender, RoutedEventArgs e)
    {
      if (InFileName.Text == "")
      {
        AutoClosingMessageBox.Show("Please choose a file", "Message", showTime);
        return;
      }
      if (InOwner.Text == "")
      {
        AutoClosingMessageBox.Show("Please fill in owner", "Message", showTime);
        return;
      }
      if (checkInDic.ContainsKey(InFileName.Text))
      {
        AutoClosingMessageBox.Show("Item already exists in current group", "Message", showTime);
        return;
      }
      element ele = new element();
      GroupBox.Items.Add("Name: " + InFileName.Text);
      ele.name = InFileName.Text;
      GroupBox.Items.Add("Dependency: " + DepFileName.Text);
      ele.dep = DepFileName.Text;
      GroupBox.Items.Add("Description: " + InDescription.Text);
      ele.des = InDescription.Text;
      GroupBox.Items.Add("Owner: " + InOwner.Text);
      ele.owner = InOwner.Text;
      GroupBox.Items.Add("Categories: " + InCategories.Text);
      ele.cate = InCategories.Text;
      GroupBox.Items.Add("Path: " + filePathToCheckin);
      ele.path = filePathToCheckin;
      GroupBox.Items.Add("---------------------------------------------");
      checkInDic[ele.name] = ele;

      CheckInButton.IsEnabled = true;
    }
    //----< respond to mouse click on Cancel >------------------------

    private void CancelButton_Click(object sender, RoutedEventArgs e)
    {
      InFileName.Text = "";
      DepFileName.Text = "";
      InDescription.Text = "";
      InOwner.Text = "";
      InCategories.Text = "";
      GroupBox.Items.Clear();
      checkInDic.Clear();
    }
    //----< respond to mouse click on Choose File >-------------------

    private void ChooseFileCloseButton_Click(object sender, RoutedEventArgs e)
    {
      LocalBrowser lb = new LocalBrowser("FileToClose", -1);
      lb.Owner = this;
      lb.Show();
    }
    //----< respond to mouse click on Choose File >-------------------

    private void ChooseOutFileButton_Click(object sender, RoutedEventArgs e)
    {
      LocalBrowser lb = new LocalBrowser("ChooseOutFile", -1);
      lb.Owner = this;
      lb.Show();
    }
    //----< respond to mouse click on Choose Path >-------------------

    private void CheckOutPathButton_Click(object sender, RoutedEventArgs e)
    {
      System.Windows.Forms.FolderBrowserDialog fd = new System.Windows.Forms.FolderBrowserDialog();
      System.Windows.Forms.DialogResult result = fd.ShowDialog();
      if (result == System.Windows.Forms.DialogResult.OK && !string.IsNullOrWhiteSpace(fd.SelectedPath))
      {
        CheckOutPath.Text = fd.SelectedPath;
      }
    }
    //----< respond to mouse click on Choose File >-------------------

    private void ChooseFileToBrowseButton_Click(object sender, RoutedEventArgs e)
    {
      LocalBrowser lb = new LocalBrowser("ChooseFileToBrowse", -1);
      lb.Owner = this;
      lb.Show();
      BrowseName.Text = "";
      BrowseStatus.Text = "";
      BrowseDateTime.Text = "";
      BrowseDescription.Text = "";
      BrowseOwner.Text = "";
      BrowsePath.Text = "";
      BrowseCategories.Text = "";
      BrowseDep.Text = "";
    }
    //----< respond to mouse click on Choose Dependency >-------------

    private void ChooseDepButton_Click(object sender, RoutedEventArgs e)
    {
      LocalBrowser lb = new LocalBrowser("QueryDep", -1);
      lb.Owner = this;
      lb.Show();
    }
    //----< respond to mouse click on Add >---------------------------

    private void AddCondsButton_Click(object sender, RoutedEventArgs e)
    {
      if (QueryCombo.SelectedIndex == 0)
      {
        QueryCate.Text = QueryConds.Text;
      }
      if (QueryCombo.SelectedIndex == 1)
      {
        QueryName.Text = QueryConds.Text;
      }
      if (QueryCombo.SelectedIndex == 2)
      {
        string ver = QueryConds.Text;
        foreach (char v in ver)
        {
          if ((int)v < 48 || (int)v > 57)
          {
            AutoClosingMessageBox.Show("Version should be a number!", "Message", showTime);
            QueryConds.Text = "";
            return;
          }
        }
        QueryVersion.Text = QueryConds.Text;
      }
    }
    //----< respond to mouse click on Clear >-------------------------

    private void ClearCondsButton_Click(object sender, RoutedEventArgs e)
    {
      QueryCate.Text = "";
      QueryName.Text = "";
      QueryVersion.Text = "";
      QueryDep.Text = "";
    }
    //----< respond to mouse click on Choose File >-------------------

    private void ChooseModifyButton_Click(object sender, RoutedEventArgs e)
    {
      LocalBrowser lb = new LocalBrowser("FileToModify", -1);
      lb.Owner = this;
      lb.Show();
    }
    //----< respond to mouse click on Modify >------------------------

    private void ModifyButton_Click(object sender, RoutedEventArgs e)
    {
      CsEndPoint serverEndPoint = new CsEndPoint();
      serverEndPoint.machineAddress = machineAddress;
      serverEndPoint.port = serverPortNum;
      CsMessage msg = new CsMessage();
      msg.add("to", CsEndPoint.toString(serverEndPoint));
      msg.add("from", CsEndPoint.toString(endPoint_));
      msg.add("command", "modify");
      msg.add("filename", FileToModify.Text);
      msg.add("dep", DepFileName.Text);
      msg.add("des", InDescription.Text);
      msg.add("cate", InCategories.Text);
      translater.postMessage(msg);
      ModifyButton.IsEnabled = false;
    }
    //----< respond to mouse click on Query >-------------------------

    private void QueryButton_Click(object sender, RoutedEventArgs e)
    {
      string mode = AndOrCombo.SelectedIndex == 1 ? "or" : "and";
      CsEndPoint serverEndPoint = new CsEndPoint();
      serverEndPoint.machineAddress = machineAddress;
      serverEndPoint.port = serverPortNum;
      CsMessage msg = new CsMessage();
      msg.add("to", CsEndPoint.toString(serverEndPoint));
      msg.add("from", CsEndPoint.toString(endPoint_));
      msg.add("command", "query");
      msg.add("filename", QueryName.Text);
      msg.add("dep", QueryDep.Text);
      msg.add("version", QueryVersion.Text);
      msg.add("cate", QueryCate.Text);
      msg.add("mode", mode);
      translater.postMessage(msg);
    }
    //----< show file text >-------------------------------------------

    private void showFile(string fileName, string fileContents)
    {
      Paragraph paragraph = new Paragraph();
      paragraph.Inlines.Add(new Run(fileContents));
      CodePopupWindow popUp = new CodePopupWindow();
      popUp.Show();
      popUp.codeView.Blocks.Clear();
      popUp.codeView.Blocks.Add(paragraph);
    }
    //----< test function testR1 >-------------------------------------

    private void testR1()
    {
      ThreadStart testR1 = () =>
      {
        System.Threading.Thread.Sleep(1000);
        Console.Write("\n\n  Demonstrating Requirement #1");
        Console.Write("\n  ********************************");
        Console.Write("\n  -This project uses C++11, C#, WPF and C++/CLI.");
      };
      Thread r1Thrd = new Thread(testR1);
      r1Thrd.IsBackground = true;
      r1Thrd.Start();
    }
    //----< test function testR2a >------------------------------------

    private void testR2a()
    {
      ThreadStart testR2a = () =>
      {
        System.Threading.Thread.Sleep(2000);
        Console.Write("\n\n  Demonstrating Requirement #2a");
        Console.Write("\n  ********************************");
        Console.Write("\n  -Connect to server.");
        Action clickTab = () =>
        {
          this.Focus();
          ConnectTab.IsSelected = true;
        };
        Dispatcher.Invoke(clickTab, new Object[] { });

        Action clickButton = () =>
        {
          ConnectButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
        };
        Dispatcher.Invoke(clickButton, new Object[] { });
      };
      Thread r1Thrd = new Thread(testR2a);
      r1Thrd.IsBackground = true;
      r1Thrd.Start();
    }
    //----< test function testR2b >------------------------------------

    private void testR2b()
    {
      ThreadStart testR2b = () =>
      {
        System.Threading.Thread.Sleep(3000);
        Console.Write("\n\n  Demonstrating Requirement #2b");
        Console.Write("\n  ********************************");
        Console.Write("\n  -Test check in.");
        Console.Write("\n   Check in 3 files, add dependency and metadata.");
        Console.Write("\n   You can check dependency in the server console,");
        Console.Write("\n   also you can use browse to check. The file checked");
        Console.Write("\n   in could be found in the dir Project4/Checked-in/package1.");
        String commh = "../../../Storage/Comm.h";
        Action clickTab = () => 
        {
          this.Focus();
          CheckinTab.IsSelected = true;
        };
        Dispatcher.Invoke(clickTab, new Object[] { });
        System.Threading.Thread.Sleep(1000);
        Action clickAddItem1 = () =>
        {
          filePathToCheckin = commh;
          InFileName.Text = "Comm.h";
          AddItemButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
        };
        Dispatcher.Invoke(clickAddItem1, new Object[] { });
        Action clickCheckIn = () => { CheckInButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent)); };
        Dispatcher.Invoke(clickCheckIn, new Object[] { });
        R2bHelper();
      };
      Thread r1Thrd = new Thread(testR2b);
      r1Thrd.IsBackground = true;
      r1Thrd.Start();
    }
    //----< helper function for function testR2b >--------------------

    private void R2bHelper()
    {
      String commcpp = "../../../Storage/Comm.cpp", filecpp = "../../../Storage/FileSystem.cpp";
      Action clickCheckIn = () => { CheckInButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent)); };
      System.Threading.Thread.Sleep(1500);
      Action clickAddItem2 = () =>
      {
        LocalBrowser lb = new LocalBrowser("DepFileName", 0);
        lb.Owner = this;
        lb.Show();
        filePathToCheckin = commcpp;
        InFileName.Text = "Comm.cpp";
        DepFileName.Text = "NoSqlDb_Comm.h.1";
        AddItemButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
      };
      Dispatcher.Invoke(clickAddItem2, new Object[] { });
      Dispatcher.Invoke(clickCheckIn, new Object[] { });
      System.Threading.Thread.Sleep(1500);
      Action clickAddItem3 = () =>
      {
        LocalBrowser lb = new LocalBrowser("DepFileName", 0);
        lb.Owner = this;
        lb.Show();
        filePathToCheckin = filecpp;
        InFileName.Text = "FileSystem.cpp";
        DepFileName.Text = "NoSqlDb_Comm.h.1;NoSqlDb_Comm.cpp.1";
        AddItemButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
      };
      Dispatcher.Invoke(clickAddItem3, new Object[] { });
      Dispatcher.Invoke(clickCheckIn, new Object[] { });
    }
    //----< test function testR2c >------------------------------------

    private void testR2c()
    {
      ThreadStart testR2c = () =>
      {
        System.Threading.Thread.Sleep(8500);
        Console.Write("\n\n  Demonstrating Requirement #2c");
        Console.Write("\n  ********************************");
        Console.Write("\n  -Test modify.");
        Console.Write("\n   Modify metadata for element NoSqlDb_FileSystem.cpp.1");
        Console.Write("\n   and add dependency to it. ");
        Console.Write("\n   You can check dependency and metadata in the");
        Console.Write("\n   server console, also you can use browse to check.");
        Action clickTab = () => 
        {
          this.Focus();
          CheckinTab.IsSelected = true;
        };
        Dispatcher.Invoke(clickTab, new Object[] { });
        System.Threading.Thread.Sleep(1000);
        Action clickModify = () =>
        {
          LocalBrowser lb = new LocalBrowser("FileToModify", 0);
          lb.Owner = this;
          lb.Show();
          FileToModify.Text = "NoSqlDb_FileSystem.cpp.1";
          DepFileName.Text = "NoSqlDb_Comm.h.1;NoSqlDb_Comm.cpp.1";
          InDescription.Text = "Changed";
          InCategories.Text = "Changed";
          ModifyButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
        };
        Dispatcher.Invoke(clickModify, new Object[] { });
      };
      Thread r1Thrd = new Thread(testR2c);
      r1Thrd.IsBackground = true;
      r1Thrd.Start();
    }
    //----< test function testR2d >------------------------------------

    private void testR2d()
    {
      ThreadStart testR2d = () =>
      {
        System.Threading.Thread.Sleep(11000);
        Console.Write("\n\n  Demonstrating Requirement #2d");
        Console.Write("\n  ********************************");
        Console.Write("\n  -Test close.");
        Console.Write("\n   Close the 3 files checked in, then check in another");
        Console.Write("\n   file, you can find the increasement of version.");
        Console.Write("\n   Close a file with open child would fail and change");
        Console.Write("\n   the file status to close-pending. After the open child");
        Console.Write("\n   closed, it can now change status to close.");
        R2dHelper();
        System.Threading.Thread.Sleep(1500);
        Action clickClose4 = () =>
        {
          LocalBrowser lb = new LocalBrowser("FileToClose", 0);
          lb.Owner = this;
          lb.Show();
          FileToClose.Text = "NoSqlDb_FileSystem.cpp.1";
          CloseButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
        };
        Dispatcher.Invoke(clickClose4, new Object[] { });
        System.Threading.Thread.Sleep(1500);
        Action clickAddItem4 = () =>
        {
          filePathToCheckin = "../../../Storage/Comm.h";
          InFileName.Text = "Comm.h";
          AddItemButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
        };
        Dispatcher.Invoke(clickAddItem4, new Object[] { });
        Action clickCheckIn = () => { CheckInButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent)); };
        Dispatcher.Invoke(clickCheckIn, new Object[] { });
      };
      Thread r1Thrd = new Thread(testR2d);
      r1Thrd.IsBackground = true;
      r1Thrd.Start();
    }
    //----< helper function for function testR2d >--------------------

    private void R2dHelper()
    {
      Action clickTab = () =>
      {
        this.Focus();
        CheckinTab.IsSelected = true;
      };
      Dispatcher.Invoke(clickTab, new Object[] { });
      System.Threading.Thread.Sleep(1000);
      Action clickClose1 = () =>
      {
        LocalBrowser lb = new LocalBrowser("FileToClose", 0);
        lb.Owner = this;
        lb.Show();
        FileToClose.Text = "NoSqlDb_Comm.h.1";
        CloseButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
      };
      Dispatcher.Invoke(clickClose1, new Object[] { });
      System.Threading.Thread.Sleep(1500);
      Action clickClose2 = () =>
      {
        LocalBrowser lb = new LocalBrowser("FileToClose", 0);
        lb.Owner = this;
        lb.Show();
        FileToClose.Text = "NoSqlDb_FileSystem.cpp.1";
        CloseButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
      };
      Dispatcher.Invoke(clickClose2, new Object[] { });
      System.Threading.Thread.Sleep(1500);
      Action clickClose3 = () =>
      {
        LocalBrowser lb = new LocalBrowser("FileToClose", 0);
        lb.Owner = this;
        lb.Show();
        FileToClose.Text = "NoSqlDb_Comm.cpp.1";
        CloseButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
      };
      Dispatcher.Invoke(clickClose3, new Object[] { });
    }
    //----< test function testR2e >------------------------------------

    private void testR2e()
    {
      ThreadStart testR2e = () =>
      {
        System.Threading.Thread.Sleep(19500);
        Console.Write("\n\n  Demonstrating Requirement #2e");
        Console.Write("\n  ********************************");
        Console.Write("\n  -Test check out.");
        Console.Write("\n   Check out the file NoSqlDb_FileSystem.cpp.1 with");
        Console.Write("\n   dependency, you can find the files checked out in");
        Console.Write("\n   the dir Project4/Client-Checked-Out.");
        Action clickTab = () => 
        {
          this.Focus();
          CheckoutTab.IsSelected = true;
        };
        Dispatcher.Invoke(clickTab, new Object[] { });
        System.Threading.Thread.Sleep(1000);
        Action clickCheckOut = () =>
        {
          LocalBrowser lb = new LocalBrowser("ChooseOutFile", 0);
          lb.Owner = this;
          lb.Show();
          ChooseOutFile.Text = "NoSqlDb_FileSystem.cpp.1";
          CheckOutPath.Text = System.IO.Path.GetFullPath("../../../Client-Checked-Out");
          Dependency.IsChecked = true;
          CheckoutButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
        };
        Dispatcher.Invoke(clickCheckOut, new Object[] { });
      };
      Thread r1Thrd = new Thread(testR2e);
      r1Thrd.IsBackground = true;
      r1Thrd.Start();
    }
    //----< test function testR2f >------------------------------------

    private void testR2f()
    {
      ThreadStart testR2f = () =>
      {
        System.Threading.Thread.Sleep(22000);
        Console.Write("\n\n  Demonstrating Requirement #2f");
        Console.Write("\n  ********************************");
        Console.Write("\n  -Test browse.");
        Console.Write("\n   The metadata is shown on the browse tab of GUI,");
        Console.Write("\n   the content is shown in the READER window.");
        Action clickTab = () => 
        {
          this.Focus();
          BrowseTab.IsSelected = true;
        };
        Dispatcher.Invoke(clickTab, new Object[] { });
        System.Threading.Thread.Sleep(1000);
        Action clickView = () =>
        {
          LocalBrowser lb = new LocalBrowser("ChooseFileToBrowse", 0);
          lb.Owner = this;
          lb.Show();
          ChooseFileToBrowse.Text = "NoSqlDb_FileSystem.cpp.1";
          ViewTextButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
        };
        Dispatcher.Invoke(clickView, new Object[] { });
      };
      Thread r1Thrd = new Thread(testR2f);
      r1Thrd.IsBackground = true;
      r1Thrd.Start();
    }
    //----< test function testR2g >------------------------------------

    private void testR2g()
    {
      ThreadStart testR2g = () =>
      {
        System.Threading.Thread.Sleep(23000);
        Console.Write("\n\n  Demonstrating Requirement #2g");
        Console.Write("\n  ********************************");
        Console.Write("\n  -Test query.");
        Console.Write("\n   For adding conditions, you need to first choose an");
        Console.Write("\n   attribute in the ComboBox, then input the value and");
        Console.Write("\n   click Add. For query, the empty value would return true.");
        Action clickTab = () => 
        {
          this.Focus();
          QueryTab.IsSelected = true;
        };
        Dispatcher.Invoke(clickTab, new Object[] { });
        System.Threading.Thread.Sleep(1000);
        Action clickQueryAnd = () =>
        {
          this.Focus();
          AndOrCombo.SelectedIndex = 0;
          QueryDep.Text = "NoSqlDb_Comm.h.1";
          QueryCate.Text = "Hello";
          QueryName.Text = "cpp";
          QueryVersion.Text = "1";
          QueryButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
        };
        Dispatcher.Invoke(clickQueryAnd, new Object[] { });
        System.Threading.Thread.Sleep(1500);
        Action clickQueryOr = () =>
        {
          AndOrCombo.SelectedIndex = 1;
          QueryDep.Text = "NoSqlDb_Comm.h.1";
          QueryCate.Text = "A not exist category";
          QueryName.Text = "cpp";
          QueryVersion.Text = "1";
          QueryButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
        };
        Dispatcher.Invoke(clickQueryOr, new Object[] { });
      };
      Thread r1Thrd = new Thread(testR2g);
      r1Thrd.IsBackground = true;
      r1Thrd.Start();
    }
    //----< test function testR3 >------------------------------------

    private void testR3()
    {
      ThreadStart testR3 = () =>
      {
        System.Threading.Thread.Sleep(26000);
        Console.Write("\n\n  Demonstrating Requirement #3");
        Console.Write("\n  ********************************");
        Console.Write("\n  -About requirement #3");
        Console.Write("\n   For uploading files, it's part of check in,");
        Console.Write("\n   which was already demonstrated in #2b.");
        Console.Write("\n   For downloading files and viewing the repo contents,");
        Console.Write("\n   it's both part of check out and browse, which was");
        Console.Write("\n   already demonstrated in #2e and #2f.");
      };
      Thread r3Thrd = new Thread(testR3);
      r3Thrd.IsBackground = true;
      r3Thrd.Start();
    }
    //----< test function testR4 >------------------------------------

    private void testR4()
    {
      ThreadStart testR4 = () =>
      {
        System.Threading.Thread.Sleep(26500);
        Console.Write("\n\n  Demonstrating Requirement #4");
        Console.Write("\n  ********************************");
        Console.Write("\n  -About requirement #4");
        Console.Write("\n   For communication system, it's implemented in the project");
        Console.Write("\n   named MsgPassingComm, which based on sockets. All of the");
        Console.Write("\n   message passing and file transmission used in this project");
        Console.Write("\n   are based on the function provided by MsgPassingComm.");
      };
      Thread r4Thrd = new Thread(testR4);
      r4Thrd.IsBackground = true;
      r4Thrd.Start();
    }
    //----< test function testR5 >------------------------------------

    private void testR5()
    {
      ThreadStart testR5 = () =>
      {
        System.Threading.Thread.Sleep(27000);
        Console.Write("\n\n  Demonstrating Requirement #5");
        Console.Write("\n  ********************************");
        Console.Write("\n  -About requirement #5");
        Console.Write("\n   For HTTP style communication system, the check in procedure");
        Console.Write("\n   in this project could be a good demonstration(#2b). The client");
        Console.Write("\n   would post a getPath message to server, On receiving that,");
        Console.Write("\n   server construct a dir and send back the path in a message called");
        Console.Write("\n   replyPath. After received the replyPath message, client would send");
        Console.Write("\n   files to be checked in to the path provided by server.");
      };
      Thread r5Thrd = new Thread(testR5);
      r5Thrd.IsBackground = true;
      r5Thrd.Start();
    }
    //----< test function testR6 >------------------------------------

    private void testR6()
    {
      ThreadStart testR6 = () =>
      {
        System.Threading.Thread.Sleep(27500);
        Console.Write("\n\n  Demonstrating Requirement #6");
        Console.Write("\n  ********************************");
        Console.Write("\n  -About requirement #6");
        Console.Write("\n   For file transmission, it happens in check in(#2b), check out(#2e),");
        Console.Write("\n   and browse(#2f). The file to be sent would be seperated into blocks,");
        Console.Write("\n   in this project, after receiving all of the blocks, a receiveEnd");
        Console.Write("\n   message would receive which is the end of file receiving.");
      };
      Thread r6Thrd = new Thread(testR6);
      r6Thrd.IsBackground = true;
      r6Thrd.Start();
    }
    //----< test function testR7 >------------------------------------

    private void testR7()
    {
      ThreadStart testR7 = () =>
      {
        System.Threading.Thread.Sleep(28000);
        Console.Write("\n\n  Demonstrating Requirement #7");
        Console.Write("\n  ********************************");
        Console.Write("\n  -About requirement #7");
        Console.Write("\n   You are reading the result of automated unit test.");
        showTime = 3000;
      };
      Thread r7Thrd = new Thread(testR7);
      r7Thrd.IsBackground = true;
      r7Thrd.Start();
    }
    //----< run all the tests >----------------------------------------

    private void test()
    {
      // call test function
      testR1();
      testR2a();
      testR2b();
      testR2c();
      testR2d();
      testR2e();
      testR2f();
      testR2g();
      testR3();
      testR4();
      testR5();
      testR6();
      testR7();
    }
  }

  public class AutoClosingMessageBox
  {
    System.Threading.Timer _timeoutTimer;
    string _caption;
    AutoClosingMessageBox(string text, string caption, int timeout)
    {
      _caption = caption;
      _timeoutTimer = new System.Threading.Timer(OnTimerElapsed,
          null, timeout, System.Threading.Timeout.Infinite);
      using (_timeoutTimer)
        MessageBox.Show(text, caption);
    }
    public static void Show(string text, string caption, int timeout)
    {
      new AutoClosingMessageBox(text, caption, timeout);
    }
    void OnTimerElapsed(object state)
    {
      IntPtr mbWnd = FindWindow("#32770", _caption); // lpClassName is #32770 for MessageBox
      if (mbWnd != IntPtr.Zero)
        SendMessage(mbWnd, WM_CLOSE, IntPtr.Zero, IntPtr.Zero);
      _timeoutTimer.Dispose();
    }
    const int WM_CLOSE = 0x0010;
    [System.Runtime.InteropServices.DllImport("user32.dll", SetLastError = true)]
    static extern IntPtr FindWindow(string lpClassName, string lpWindowName);
    [System.Runtime.InteropServices.DllImport("user32.dll", CharSet = System.Runtime.InteropServices.CharSet.Auto)]
    static extern IntPtr SendMessage(IntPtr hWnd, UInt32 Msg, IntPtr wParam, IntPtr lParam);
  }
}
