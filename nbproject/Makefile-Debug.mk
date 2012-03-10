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
CCC=g++-4.1
CXX=g++-4.1
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include kaffeine-Makefile.mk

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/server/kaffeine.o \
	${OBJECTDIR}/src/server/vcp.o


# C Compiler Flags
CFLAGS=-D_GNU_SOURCE -std=c99 -pthread -Wall

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
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/kaffeine

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/kaffeine: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/kaffeine ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/src/server/kaffeine.o: src/server/kaffeine.c 
	${MKDIR} -p ${OBJECTDIR}/src/server
	${RM} $@.d
	$(COMPILE.c) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/server/kaffeine.o src/server/kaffeine.c

${OBJECTDIR}/src/server/vcp.o: src/server/vcp.c 
	${MKDIR} -p ${OBJECTDIR}/src/server
	${RM} $@.d
	$(COMPILE.c) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/server/vcp.o src/server/vcp.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/kaffeine

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
