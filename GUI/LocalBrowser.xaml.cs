/////////////////////////////////////////////////////////////////////
// LocalBrowse.xaml.cs                                             //
// ver 1.0                                                         //
// Language:    C#, Visual Studio 2017                             //
// Platform:    Macbook pro, Windows 10 Home                       //
// Application: Spring 2018 CSE687 Project#3                       //
// Author:      Yuan Liu, yliu219@syr.edu                          //
// Referrence:  Jim Fawcett, CSE687                                //
/////////////////////////////////////////////////////////////////////
/*
*  
* Package Operations:
* -------------------
* -Show the file list get from server.
*
* Public Interface:
* -----------------
* There is no public functions.
*
* Required Files:
* ---------------
* MainWindow.xaml.cs
*
* Build Process:
* --------------
* devenv Project4.sln /rebuild debug
*
* Maintenance History:
* --------------------
* ver 1.0 : 20 Apr 2018
*/
using System;
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
using System.Windows.Shapes;
using System.Threading;
using MsgPassingCommunication;

namespace GUI
{
  /// <summary>
  /// Interaction logic for LocalBrowser.xaml
  /// </summary>
  public partial class LocalBrowser : Window
  {
    public LocalBrowser(string textname, int index)
    {
      InitializeComponent();
      textname_ = textname;
      index_ = index;
    }

    private Stack<string> popPathStack_ = new Stack<string>();
    private Translater translater;
    private CsEndPoint endPoint_;
    private int serverPortNum = 8080;
    private Thread rcvThrd = null;
    private string textname_;
    private int index_;
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
    //----< load getPopDirs processing into dispatcher dictionary >-----

    private void DispatcherLoadGetPopDirs()
    {
      Action<CsMessage> getDirs = (CsMessage rcvMsg) =>
      {
        Action clrDirs = () =>
        {
          PopDirs.Items.Clear();
        };
        Dispatcher.Invoke(clrDirs, new Object[] { });
        var enumer = rcvMsg.attributes.GetEnumerator();
        while (enumer.MoveNext())
        {
          string key = enumer.Current.Key;
          if (key.Contains("dir"))
          {
            Action<string> doDir = (string dir) =>
            {
              PopDirs.Items.Add(dir);
            };
            Dispatcher.Invoke(doDir, new Object[] { enumer.Current.Value });
          }
        }
        Action insertUp = () =>
        {
          PopDirs.Items.Insert(0, "..");
        };
        Dispatcher.Invoke(insertUp, new Object[] { });
      };
      addClientProc("getPopDirs", getDirs);
    }
    //----< load getPopFiles processing into dispatcher dictionary >----

    private void DispatcherLoadGetPopFiles()
    {
      Action<CsMessage> getFiles = (CsMessage rcvMsg) =>
      {
        Action clrFiles = () =>
        {
          PopFiles.Items.Clear();
        };
        Dispatcher.Invoke(clrFiles, new Object[] { });
        var enumer = rcvMsg.attributes.GetEnumerator();
        while (enumer.MoveNext())
        {
          string key = enumer.Current.Key;
          if (key.Contains("file"))
          {
            Action<string> doFile = (string file) =>
            {
              PopFiles.Items.Add(file);
            };
            Dispatcher.Invoke(doFile, new Object[] { enumer.Current.Value });
          }
        }
      };
      addClientProc("getPopFiles", getFiles);
    }
    //----< respond to mouse double-click on PopDirs >---------

    private void PopDirs_MouseDoubleClick(object sender, MouseButtonEventArgs e)
    {
      // build path for selected dir
      string selectedDir = (string)PopDirs.SelectedItem;
      if (selectedDir == null) return;
      string path;
      if (selectedDir == "..")
      {
        if (popPathStack_.Count > 1)  // don't pop off "Storage"
          popPathStack_.Pop();
        else
          return;
      }
      else
      {
        path = popPathStack_.Peek() + "/" + selectedDir;
        popPathStack_.Push(path);
      }

      CsEndPoint serverEndPoint = new CsEndPoint();
      serverEndPoint.machineAddress = "localhost";
      serverEndPoint.port = serverPortNum;
      CsMessage msg = new CsMessage();
      msg.add("to", CsEndPoint.toString(serverEndPoint));
      msg.add("from", CsEndPoint.toString(endPoint_));
      msg.add("command", "getPopDirs");
      msg.add("path", popPathStack_.Peek());
      translater.postMessage(msg);

      // build message to get files and post it
      msg.remove("command");
      msg.add("command", "getPopFiles");
      translater.postMessage(msg);
    }
    //----< respond to mouse double-click on PopFiles >--------

    private void PopFiles_MouseDoubleClick(object sender, RoutedEventArgs e)
    {
      string selectedFile = (string)PopFiles.SelectedItem;
      if (selectedFile == null) return;
      MainWindow mainwindow = Owner as MainWindow;
      if (textname_ == "FileToClose")
      {
        mainwindow.CloseButton.IsEnabled = true;
        mainwindow.FileToClose.Text = selectedFile;
      }
      if (textname_ == "ChooseOutFile")
      {
        mainwindow.CheckoutButton.IsEnabled = true;
        mainwindow.ChooseOutFile.Text = selectedFile;
      }
      if (textname_ == "DepFileName")
      {
        if (mainwindow.DepFileName.Text == "")
          mainwindow.DepFileName.Text = selectedFile;
        else
          mainwindow.DepFileName.Text += ";" + selectedFile;
      }
      if (textname_ == "ChooseFileToBrowse")
      {
        mainwindow.ChooseFileToBrowse.Text = selectedFile;
        mainwindow.ViewTextButton.IsEnabled = true;
      }
      if (textname_ == "QueryDep")
      {
        if (mainwindow.QueryDep.Text == "")
          mainwindow.QueryDep.Text = selectedFile;
        else
          mainwindow.QueryDep.Text += ";" + selectedFile;
      }
      if (textname_ == "FileToModify")
      {
        mainwindow.FileToModify.Text = selectedFile;
        mainwindow.ModifyButton.IsEnabled = true;
      }
    }
    //----< load dispatcher >-----------------------------------------

    private void loadDispatcher()
    {
      DispatcherLoadGetPopDirs();
      DispatcherLoadGetPopFiles();
    }
    //----< window load function >------------------------------------

    private void Window_Loaded(object sender, RoutedEventArgs e)
    {
      endPoint_ = new CsEndPoint();                   // start Comm
      endPoint_.machineAddress = "localhost";
      endPoint_.port = new Random().Next(10000);
      translater = new Translater();
      translater.listen(endPoint_);
      processMessages();                              // start processing messages
      loadDispatcher();

      popPathStack_.Push("../Checked-in");
      CsEndPoint serverEndPoint = new CsEndPoint();
      serverEndPoint.machineAddress = "localhost";
      serverEndPoint.port = serverPortNum;
      CsMessage msg = new CsMessage();
      msg.add("to", CsEndPoint.toString(serverEndPoint));
      msg.add("from", CsEndPoint.toString(endPoint_));
      msg.add("command", "getPopDirs");
      msg.add("path", popPathStack_.Peek());
      translater.postMessage(msg);
      msg.remove("command");
      msg.add("command", "getPopFiles");
      translater.postMessage(msg);

      if (index_ != -1)
      {
        autoclick();
      }
    }
    //----< click the file with given index automatically >-----------

    private void autoclick()
    {
      Action clickDir = () =>
      {
        MouseButtonEventArgs e = new MouseButtonEventArgs(Mouse.PrimaryDevice, 0, MouseButton.Left);
        e.RoutedEvent = Control.MouseDoubleClickEvent;
        e.Source = PopDirs;
        PopDirs.SelectedIndex = 1;
        PopDirs.RaiseEvent(e);
      };
      Dispatcher.Invoke(clickDir, new Object[] { });

      Action clickFile = () =>
      {
        MouseButtonEventArgs e = new MouseButtonEventArgs(Mouse.PrimaryDevice, 0, MouseButton.Left);
        e.RoutedEvent = Control.MouseDoubleClickEvent;
        e.Source = PopFiles;
        PopFiles.Items.Add("Comm.h");
        PopFiles.SelectedIndex = index_;
        PopFiles.RaiseEvent(e);
        this.Close();
      };
      Dispatcher.Invoke(clickFile, new Object[] { });
    }
  }
}
