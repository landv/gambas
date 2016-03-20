[[ error
Message
--
Description
==
[/error/access]
--
The  requested access to the file is not allowed,
or search permission is denied for one of the
directories in the path prefix of pathname, or the
file does not exist yet and write access to the parent directory is not allowed.
==
[/error/dir]
--
~FileName~ refers to a directory. Use the function [/lang/dir] instead!
==
[/error/nexist]
--
~FileName~ does not exist, or a directory component in pathname does not exist or is a dangling symbolic link.
==
[/error/memory]
--
The system ran out of memory.
==
[/error/full]
--
~FileName~ was to be created but the  device  containing  ~FileName~ has no room for the new file.
==
[/error/ndir]
--
A  component  used as a directory in ~FileName~ is not, in fact, a directory.
==
[/error/system]
--
Other possible system errors:
* Too many symbolic links were encountered in resolving ~FileName~.
* The process already has the maximum number of files open.
* The  system limit  on  the  total number of open files has been reached.
* ~FileName~ refers to a device special file  and  no  corresponding device  exists.
* The named file is a named pipe and no process has the file open for reading.
* ~FileName~ refers to a file on a read-only  filesystem  and  write access was requested.
* ~FileName~ refers to an executable image which is currently being executed and write access was requested.
]]
