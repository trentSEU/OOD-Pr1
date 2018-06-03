/////////////////////////////////////////////////////////////////////
// CodePopupWindow.xaml.cs   - pop up window                       //
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
* -Show the file in pop up window.
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
* ver 1.0 : 10 Apr 2018
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

namespace GUI
{
  /// <summary>
  /// Interaction logic for CodePopupWindow.xaml
  /// </summary>
  public partial class CodePopupWindow : Window
  {
    public CodePopupWindow()
    {
      InitializeComponent();
    }
  }
}
