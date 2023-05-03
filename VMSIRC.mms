! header                                                                      !auto
! Do not modify this file. It may be overwritten automatically.               !auto
! includes                                                                    !auto
INCLUDES= -                                                                   !auto
irc.h -                                                                       !auto
[.modules]constants.h -                                                       !auto
[.modules]modules.h                                                           !auto
! sources                                                                     !auto
SOURCES= -                                                                    !auto
irc.c -                                                                       !auto
snprintf_vms.c -                                                              !auto
[.modules]ctcp.c -                                                            !auto
[.modules]inputbox.c -                                                        !auto
[.modules]privmsg.c                                                           !auto
! dependencies                                                                !auto
.FIRST                                                                        !auto
                                                                              !auto
.LAST                                                                         !auto
                                                                              !auto
! compiler/linker options                                                     !auto
.IF DEBUG .AND $(DEBUG) .EQ 1                                                 !auto
COMPILEFLAGS = /DEBUG/NOOP/LIST=$(MMS$TARGET_NAME)/OBJECT=$(MMS$TARGET)       !auto
LINKFLAGS = /DEBUG/MAP=$(MMS$TARGET_NAME)/EXECUTABLE=$(MMS$TARGET)            !auto
.ELSE                                                                         !auto
COMPILEFLAGS = /OBJECT=$(MMS$TARGET)                                          !auto
LINKFLAGS = /EXECUTABLE=$(MMS$TARGET)                                         !auto
.ENDIF                                                                        !auto
! directives                                                                  !auto
.SILENT                                                                       !auto
OUT_DIR = .$(OUTDIR).$(CONFIG)                                                !auto
OBJ_DIR = $(OUT_DIR).obj                                                      !auto
.SUFFIXES                                                                     !auto
.SUFFIXES .OBJ .CPP .C .CLD .MSG .BLI .COB .PAS .BAS .F77 .F90 .FOR .B32 .CBL !auto
.CPP.OBJ                                                                      !auto
    pipe create/dir $(DIR $(MMS$TARGET)) | copy SYS$INPUT nl:                 !auto
    $(CXX) $(COMPILEFLAGS) $(MMS$SOURCE)                                      !auto
                                                                              !auto
.C.OBJ                                                                        !auto
    pipe create/dir $(DIR $(MMS$TARGET)) | copy SYS$INPUT nl:                 !auto
    $(CC) $(COMPILEFLAGS) $(MMS$SOURCE)                                       !auto
                                                                              !auto
.CLD.OBJ                                                                      !auto
    pipe create/dir $(DIR $(MMS$TARGET)) | copy SYS$INPUT nl:                 !auto
    SET COMMAND/OBJECT=$(MMS$TARGET) $(MMS$SOURCE)                            !auto
                                                                              !auto
.MSG.OBJ                                                                      !auto
    pipe create/dir $(DIR $(MMS$TARGET)) | copy SYS$INPUT nl:                 !auto
    MESSAGE /OBJECT=$(MMS$TARGET) $(MMS$SOURCE)                               !auto
                                                                              !auto
.BLI.OBJ                                                                      !auto
    pipe create/dir $(DIR $(MMS$TARGET)) | copy SYS$INPUT nl:                 !auto
    BLISS $(COMPILEFLAGS) $(MMS$SOURCE)                                       !auto
                                                                              !auto
.B32.OBJ                                                                      !auto
    pipe create/dir $(DIR $(MMS$TARGET)) | copy SYS$INPUT nl:                 !auto
    BLISS $(COMPILEFLAGS) $(MMS$SOURCE)                                       !auto
                                                                              !auto
.COB.OBJ                                                                      !auto
    pipe create/dir $(DIR $(MMS$TARGET)) | copy SYS$INPUT nl:                 !auto
    COBOL $(COMPILEFLAGS) $(MMS$SOURCE)                                       !auto
                                                                              !auto
.CBL.OBJ                                                                      !auto
    pipe create/dir $(DIR $(MMS$TARGET)) | copy SYS$INPUT nl:                 !auto
    COBOL $(COMPILEFLAGS) $(MMS$SOURCE)                                       !auto
                                                                              !auto
.PAS.OBJ                                                                      !auto
    pipe create/dir $(DIR $(MMS$TARGET)) | copy SYS$INPUT nl:                 !auto
    PASCAL $(COMPILEFLAGS) $(MMS$SOURCE)                                      !auto
                                                                              !auto
.BAS.OBJ                                                                      !auto
    pipe create/dir $(DIR $(MMS$TARGET)) | copy SYS$INPUT nl:                 !auto
    BASIC $(COMPILEFLAGS) $(MMS$SOURCE)                                       !auto
                                                                              !auto
.F77.OBJ                                                                      !auto
    pipe create/dir $(DIR $(MMS$TARGET)) | copy SYS$INPUT nl:                 !auto
    FORTRAN $(COMPILEFLAGS) $(MMS$SOURCE)                                     !auto
                                                                              !auto
.F90.OBJ                                                                      !auto
    pipe create/dir $(DIR $(MMS$TARGET)) | copy SYS$INPUT nl:                 !auto
    FORTRAN $(COMPILEFLAGS) $(MMS$SOURCE)                                     !auto
                                                                              !auto
.FOR.OBJ                                                                      !auto
    pipe create/dir $(DIR $(MMS$TARGET)) | copy SYS$INPUT nl:                 !auto
    FORTRAN $(COMPILEFLAGS) $(MMS$SOURCE)                                     !auto
                                                                              !auto
.DEFAULT                                                                      !auto
    ! Source $(MMS$TARGET) not yet added                                      !auto
                                                                              !auto
! main target                                                                 !auto
[$(OUT_DIR)]$(NAME).EXE :  -                                                  !auto
[$(OBJ_DIR)]irc.obj -                                                         !auto
[$(OBJ_DIR)]snprintf_vms.obj -                                                !auto
[$(OBJ_DIR).modules]ctcp.obj -                                                !auto
[$(OBJ_DIR).modules]inputbox.obj -                                            !auto
[$(OBJ_DIR).modules]privmsg.obj                                               !auto
    LINK $(LINKFLAGS) $(MMS$SOURCE_LIST)                                   !auto
                                                                              !auto
! objects                                                                     !auto
[$(OBJ_DIR)]irc.obj : irc.c $(INCLUDES)                                       !auto
[$(OBJ_DIR)]snprintf_vms.obj : snprintf_vms.c $(INCLUDES)                     !auto
[$(OBJ_DIR).modules]ctcp.obj : [.modules]ctcp.c $(INCLUDES)                   !auto
[$(OBJ_DIR).modules]inputbox.obj : [.modules]inputbox.c $(INCLUDES)           !auto
[$(OBJ_DIR).modules]privmsg.obj : [.modules]privmsg.c $(INCLUDES)             !auto
                                                                             