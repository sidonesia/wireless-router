#-----------------------------------------------------#
# Sidnei's make file								  #
# used to build the router application 				  #
#													  #
# Nothing to explain really its just a very simply	  # 
# makefile											  #
#-----------------------------------------------------#

#
# change APP_PATH to where ever the graphapp header files are along
# with the libapp file
#
#
APP_PATH=EXTERNAL_LIB_AND_INCS
TARGET=router.exe
CXX=gcc
LINKER=g++
FLAGS=-Wall -g  -I$(APP_PATH) -c
BFLAGS=-Wall -g  -I$(APP_PATH) 
APP_LIB=app
X11=/usr/X11R6
X11LIB=$(X11)/lib
LIBS= -lX11 -lc -lm
DYNALINK= -Xlinker -rpath -Xlinker $(APP_PATH)
GAPPFLGS= -I$(APP_PATH)
DYNAMIC=-L$(APP_PATH) -l$(APP_LIB) -L$(X11LIB) $(LIBS) $(DYNALINK)

OBJ=main.o fileIO.o  mip_protocol.o  \
    mobile_unit.o router.o router_socket_channel.o \
    table.o RIPAlgorithm.o mobile_unit_icmp.o  \
	mobile_unit_request.o router_switch_fabric.o \
	fileIOUpdate.o common_include.o GUIRouter.o

OBJC=mip_protocol.o router_socket_channel.o fileIO.o  \
	 mobile_unit.o router.o  RIPAlgorithm.o table.o main_client.o \
	 mobile_unit_icmp.o mobile_unit_request.o  \
	 router_switch_fabric.o fileIOUpdate.o common_include.o GUIRouter.o


OBJCN=mip_protocol.o router_socket_channel.o fileIO.o  \
	  mobile_unit.o router.o  RIPAlgorithm.o table.o 			\
	  mobile_unit_icmp.o mobile_unit_request.o  \
	  router_switch_fabric.o CN_terminal.o fileIOUpdate.o \
	  common_include.o GUIRouter.o GUICn.o
#
# Will be used for solaris unix builds
#
solaris:solenv $(OBJ)
	$(LINKER) -Wall -g *.o -threads  -L/usr/lib/netlib -lsocket -o router.exe

solarisClient:solenv $(OBJC)
	$(LINKER) -Wall -g $(OBJC) -threads -L/usr/lib/netlib -lsocket -o mobile_unit.exe

solarisCN:solenv $(OBJCN)
	$(LINKER) -Wall -g $(OBJCN) -L/usr/lib/netlib -lsocket -o CNUnit.exe


#
# Will be used for linux redhat builds
#
linux:linenv $(OBJ)
	$(LINKER) -Wall -g *.o -pthread -o router.exe $(DYNAMIC)

linuxClient:linenv $(OBJC)
	$(LINKER) -Wall -g $(OBJC) -pthread -o mobile_unit.exe $(DYNAMIC)

linuxCN:linenv $(OBJCN)
	$(LINKER) -Wall -g $(OBJCN) -pthread -o CNUnit.exe $(DYNAMIC)

solenv:
	BUILDENV="solaris"
	export BUILDENV
linenv: 
	BUILDENV="linux"
	export BUILDENV

GUICn.o:GUICn.c GUICn.h
	$(CXX) $(FLAGS) GUICn.c 

GUIRouter.o:GUIRouter.c GUIRouter.h
	$(CXX) $(FLAGS) GUIRouter.c 

common_include.o: common_include.c common_include.h
	$(CXX) $(FLAGS) common_include.c

fileIOUpdate.o: fileIOUpdate.c fileIOUpdate.h
	$(CXX) $(FLAGS) fileIOUpdate.c

CN_terminal.o: CN_terminal.c CN_terminal.h
	$(CXX) $(FLAGS) CN_terminal.c

router_switch_fabric.o: router_switch_fabric.c router_switch_fabric.h
	$(CXX) $(FLAGS) router_switch_fabric.c

mobile_unit_icmp.o:mobile_unit_icmp.c mobile_unit_icmp.h
	$(CXX) $(FLAGS) mobile_unit_icmp.c
#	$(CXX) $(GAPPFLGS) mobile_unit_icmp.c $(DYNAMIC)

mobile_unit_request.o:mobile_unit_request.c mobile_unit_request.h
	$(CXX) $(FLAGS) mobile_unit_request.c

main_client.o:main_client.c
	$(CXX) $(FLAGS) main_client.c

main.o: main.c
	 $(CXX) $(FLAGS) main.c
#	$(CXX) $(GAPPFLGS) main.c $(DYNAMIC)

RIPAlgorithm.o: RIPAlgorithm.h RIPAlgorithm.c
	$(CXX) $(FLAGS) RIPAlgorithm.c

fileIO.o:fileIO.c fileIO.h
	$(CXX) $(FLAGS) fileIO.c

mip_protocol.o:mip_protocol.c mip_protocol.h
	$(CXX) $(FLAGS) mip_protocol.c

mobile_unit.o:mobile_unit.c mobile_unit.h
	$(CXX) $(FLAGS) mobile_unit.c

router.o:router.c router.h
	$(CXX) $(FLAGS) router.c
#	$(CXX) $(GAPPFLGS) router.c $(DYNAMIC)

router_socket_channel.o:router_socket_channel.c router_socket_channel.h
	$(CXX) $(FLAGS) router_socket_channel.c

table.o:table.c table.h
	 $(CXX) $(FLAGS) table.c

clean:
	rm *.o >> /dev/null

