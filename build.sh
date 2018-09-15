#!/bin/sh

#+ + + + + + + + + + + + + + + + + #
# Sidnei B's Startup and build     #
# Shell script for NRS Assignment  #
#+ + + + + + + + + + + + + + + + + #

error_check()
{
        if [ $1 -ne 0 ];then
                echo "Error in compilation step"
        fi
}




if [ "$1" = "clean" ];then
        make clean
        exit 0
fi

if [ "`echo $OSTYPE | grep solaris`" != "" ];then
        make clean
        make solaris
        error_check $?

        make clean
        make solarisClient

        error_check $?
        make clean
        make solarisCN
else [ "`echo $OSTYPE | grep linux`" != "" ]
        error_check $?

        make clean
        make linux
        error_check $?

        make clean
        make linuxClient
        error_check $?

        make clean
        make linuxCN
        error_check $?
fi

cp -p router.exe configuration/
cp -p mobile_unit.exe configuration/

