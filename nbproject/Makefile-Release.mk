#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/become_daemon.o \
	${OBJECTDIR}/config.o \
	${OBJECTDIR}/get_num.o \
	${OBJECTDIR}/libcmd.o \
	${OBJECTDIR}/libcmd_acl.o \
	${OBJECTDIR}/libcmd_async.o \
	${OBJECTDIR}/libcmd_async_procs.o \
	${OBJECTDIR}/libcmd_dir.o \
	${OBJECTDIR}/libcmd_exchange.o \
	${OBJECTDIR}/libcmd_print.o \
	${OBJECTDIR}/libcmd_quota.o \
	${OBJECTDIR}/libcmd_shell.o \
	${OBJECTDIR}/libcmd_sys.o \
	${OBJECTDIR}/libcmd_utils.o \
	${OBJECTDIR}/libcmd_xattr.o \
	${OBJECTDIR}/logging.o \
	${OBJECTDIR}/m_crypt.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/pipe_bind.o \
	${OBJECTDIR}/sig_handlers.o \
	${OBJECTDIR}/ugid_functions.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/dmsd

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/dmsd: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/dmsd ${OBJECTFILES} ${LDLIBSOPTIONS} -lconfig -lsmbclient -lpthread -ljansson -lacl -lattr -lmcrypt -lssl

${OBJECTDIR}/become_daemon.o: become_daemon.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/become_daemon.o become_daemon.c

${OBJECTDIR}/config.o: config.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/config.o config.c

${OBJECTDIR}/get_num.o: get_num.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/get_num.o get_num.c

${OBJECTDIR}/libcmd.o: libcmd.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libcmd.o libcmd.c

${OBJECTDIR}/libcmd_acl.o: libcmd_acl.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libcmd_acl.o libcmd_acl.c

${OBJECTDIR}/libcmd_async.o: libcmd_async.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libcmd_async.o libcmd_async.c

${OBJECTDIR}/libcmd_async_procs.o: libcmd_async_procs.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libcmd_async_procs.o libcmd_async_procs.c

${OBJECTDIR}/libcmd_dir.o: libcmd_dir.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libcmd_dir.o libcmd_dir.c

${OBJECTDIR}/libcmd_exchange.o: libcmd_exchange.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libcmd_exchange.o libcmd_exchange.c

${OBJECTDIR}/libcmd_print.o: libcmd_print.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libcmd_print.o libcmd_print.c

${OBJECTDIR}/libcmd_quota.o: libcmd_quota.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libcmd_quota.o libcmd_quota.c

${OBJECTDIR}/libcmd_shell.o: libcmd_shell.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libcmd_shell.o libcmd_shell.c

${OBJECTDIR}/libcmd_sys.o: libcmd_sys.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libcmd_sys.o libcmd_sys.c

${OBJECTDIR}/libcmd_utils.o: libcmd_utils.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libcmd_utils.o libcmd_utils.c

${OBJECTDIR}/libcmd_xattr.o: libcmd_xattr.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libcmd_xattr.o libcmd_xattr.c

${OBJECTDIR}/logging.o: logging.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/logging.o logging.c

${OBJECTDIR}/m_crypt.o: m_crypt.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/m_crypt.o m_crypt.c

${OBJECTDIR}/main.o: main.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.c

${OBJECTDIR}/pipe_bind.o: pipe_bind.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/pipe_bind.o pipe_bind.c

${OBJECTDIR}/sig_handlers.o: sig_handlers.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/sig_handlers.o sig_handlers.c

${OBJECTDIR}/ugid_functions.o: ugid_functions.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ugid_functions.o ugid_functions.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
