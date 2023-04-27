## Yet to be named OpenVMS IRC Client

Currently can only receive from a single channel and server.

Can't send messages yet. 

Whoops. 

Only used on x86_64 so far, should compile on VAX, Alpha, and Itanium 
given only DEC C / VMS C runtime provided libraries and functions used.


## What to do
cc irc.c

link irc.obj

mcr DISK$X86SYS:[sys0.sysmgr]irc.exe irc.servername.com


Modify path as required to where you placed irc.c

## Congrats, you now have read-only IRC.

It will prompt you for a nick, username, and channel to join. 
Username populates both ident and whois username field.


