#!/bin/sh


./router.exe rtr1.conf 1&
./router.exe rtr2.conf 2&
./router.exe rtr3.conf 3&
./router.exe rtr4.conf 4&
./router.exe rtr5.conf 5&


../CNUnit.exe 2 7 7000&
