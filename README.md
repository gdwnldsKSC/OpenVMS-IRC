## Yet to be named OpenVMS IRC Client

Currently can only receive from a single hard-coded channel and server IP.

Can't send messages yet. 

Whoops. 

Only used on x86_64 so far, should compile on VAX, Alpha, and Itanium 
given only DEC C / VMS C runtime provided libraries and functions used.

## Usage

Assumptions:

Using SYSTEM account

Working in SYSMGR directory (default login directory for VMS SYSTEM user)

Disk label is X86SYS (default for V9.2 and E9.2-1 on x86_64)


cc irc.c

link irc.obj

mcr DISK$X86SYS:[sys0.sysmgr]irc.exe irc.servername.com



It will prompt you for a nick, username, and channel to join. 
Username populates both ident and whois username field.


## Congrats, you now have read-only IRC.