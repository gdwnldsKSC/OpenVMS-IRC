## Yet to be named OpenVMS IRC Client

Currently can only receive from a single channel and server.

Can't send messages yet. 

Whoops. 

Only used on x86_64 so far, should compile on VAX, Alpha, and Itanium   
given only DEC C / VMS C runtime provided libraries and functions used.


## What to do
cc irc.c,snprintf_vms.c  
link irc.obj,snprintf_vms.obj  
mcr DISK$X86SYS:[sys0.sysmgr]irc.exe irc.servername.com  

Modify path as required to where you placed the source/output EXE file  
snprintf_vms.obj is not required 

## Congrats, you now have read-only IRC.

It will prompt you for a nick, username, and channel to join.  
Username populates both ident and whois username field.  


## MMS BUILD FILE

Works... ish. Autogenerated using VMS-IDE plugin for VSCode - in folder VMSIRC  
on remote host. Primary development work is done via terminal directly however   
but successfully builds on remote x86_64 system using VMS-IDE. Autogenerated MMS  
file uses CXXLINK by default, so that is modified to just LINK since CXXLINK is  
almost useless on Itanium - it just brings along the deobfuscator, and doesn't   
exist at all on x86_64.   

Shouldn't hurt other platforms, as no C++ code is in use