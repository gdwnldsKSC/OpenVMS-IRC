## Yet to be named OpenVMS IRC Client

Currently can only join a single server and channel.

use /quit to exit the client or it'll jack up your terminal because  
otherwise the curses interface isn't cleaned up. /quit takes a REALLY  
long time to process right now and is kind of broken-ish. consider it  
POC.  

In fact, don't use this at all right now really - it works, you connect  
send/receive/join channel/etc, but exiting is just a complete mess.  

Only used on x86_64 so far, should compile on VAX, Alpha, and Itanium    
given only DEC C / VMS C runtime provided libraries and functions used.  

## What's new

Handles connecting to IRCDs that require a PING/PONG response session  

Beginnings of Curses interface! Output window scrolls properly, wraparound  
enabled, Nick and Channel displayed in the soon-to-be message sending/input  
area. 

CTCP handler added. Responds to VERSION query. Officially version 0.00.1 !

0.01.0: We can SEND messages to the channel we're in now! And we're threaded.  
More cleanup must be done. 

## What to do

cc cc irc.c,snprintf_vms.c,[.modules]privmsg.c,[.modules]ctcp.c    
link irc.obj,snprintf_vms.obj,privmsg.obj,ctcp.obj  
mcr DISK$X86SYS:[sys0.sysmgr]irc.exe irc.servername.com  

Currently, on vax, you will need to CC/DEFINE="__VAX" instead of just cc  

Modify path as required to where you placed the source/output EXE file  
snprintf_vms.obj is not required 

## Congrats, you now have single-channel IRC.

It will prompt you for a nick, username, and channel to join.  
Username populates both ident and whois username field.  


## MMS BUILD FILE

Works... ish. Autogenerated using VMS-IDE plugin for VSCode - in folder VMSIRC  
on remote host. Primary development work is done via terminal directly however   
but successfully builds on remote x86_64 system using VMS-IDE. Autogenerated MMS  
file uses CXXLINK by default, so that is modified to just LINK since CXXLINK is  
almost useless on Itanium - it just brings along the deobfuscator, and doesn't   
exist at all on x86_64.   

VSCode VMS-IDE is running this to build:
MMS/EXTENDED_SYNTAX/DESCR=VMSIRC.mms/MACRO=("DEBUG=0","OUTDIR=out","NAME=VMSIRC","UNIXNAME=VMSIRC","CONFIG=RELEASE")  

Shouldn't hurt other platforms, as no C++ code is in use