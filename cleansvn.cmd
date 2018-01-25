echo Removing SVN Folders in %1 
COLOR 9A 
rem FOR /r \"%1\" %%f IN (.svn) DO RD /s /q \"%%f\" \"
FOR /r "%1" %%f IN (.svn) DO RD /s /q "%%f"
