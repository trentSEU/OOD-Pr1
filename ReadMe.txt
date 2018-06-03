----------------READ ME-----------------------

 -Connect 
  Connect button connect the server, and set the user.
 -Disconnect
  Disconnect button break the connection to server.
 -Check in
  Choose a file by clicking Choose File.
  Add dependency by double click file listed by Dependency button. You
  can add more than one dependency. Click del would delete the last dependency.
  Input Description, categories and Owner which are metadata.
  Click Add Item to add the current element to Group, you can add serveral
  more elements to the current Group in the same way. Click cancel would 
  clear current Group.
  Input Namespace and Group Name, then click Check in, all the elements in 
  current Group would be checked in.
 -Modify
  Choose a file by clicking Choose File.
  You can modify the value of Dependency in the same way of Check in, you
  can also edit Description and Categories, then you click Modify, the 
  information in the database of the chosen element would be modified.
 -Close
  Choose a file by clicking Choose File.
  Click close to close the chosen file. If the chosen file has open/close-pending
  child, it can't close, there would pop up a message, and the status of the
  chosen file would transfer to close-pending. After close all the open/close-pending
  children, the chosen file can be closed.
 -Check out
  Choose a file by clicking Choose File.
  If you want to check out the chosen file with dependency, you need to select
  Dependent Files.
  Choose a path where the output files would go by clicking Choose Path.
  Now, click Check Out and the chosen files would be checked out to the chosen path.
 -Browse
  Choose a file by clicking Choose File.
  Click View, the metadata would be shown on GUI and the content of chosen file would
  be shown in the pop up window named READER.
  The chosen file would be download and you can find it in Project4/Client-Checked-Out.
 -Query
  Choose a dependent file by clicking Choose Dependency.
  For Adding Category, FileName, Version, first click the left ComboBox to specify
  a field, then input the value, click Add at last, the value would be added to
  the chosen field.
  After inputing all the conditions, click the right ComboBox to choose the mode(and/or),
  then click Query, the result would be shown in the listbox.
  ------------------------------------
  NOTE: The version should be a number.