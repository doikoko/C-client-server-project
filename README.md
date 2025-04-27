It's a client-server server writed by C programming language for LINUX

To use it:
    make (for compilation)
    
    SERVER:
        cd ./se
        ./server IP PORT MAX_CONNECTIONS
    CLIENT:
        cd ./cl
        ./client IP PORT

What it can:
    when you run client file, you can see bottom panel, it's contains some functions:
    
    1)QUIT (stop connection)
    2)VIEW FILE (by default see the files in "se" directory, but you can change it by whriting like this(my_directory/file.txt)
                (print to screen what file on server side are contain)
    3)VIEW DIR (like previos, by for a directory)
    4)DOWNLOAD (by default download to "cl" directory file or directory from "se" directory)
    5)CH DIRECTORY (change directory, look at previos function (DOWNLOAD) after call CH DIRECTORY with any folder name, for example
        "downloads/" DOWNLOAD function mutate like this:
        "download to "cl/downloads" directory file or directory from "se" directory)

FOR EXAMPLE:
    if you want download file se/serverFiles/log.txt to cl/downloads/ you can do like this:
   
    1)CH DIRECTORY -> downloads
    2)DOWNLOAD -> serverFiles/log.txt 
    
also this server support multiuse by generate new processes    
