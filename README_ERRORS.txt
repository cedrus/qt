
when using the cedrus-build.py script, there are some 'known errors' that happen
that you can IGNORE.  All of the 'ignore-able' errors happen during the INSTALL
phase (during 'make install', not the first basic 'make').

any errors OTHER than what you see listed below should be investigated!

these errors below, however, may be ignored. no need to retry the build if
these are your only errors:

make[2]: *** [sub-script-install_subtargets-ordered] Error 2
make[1]: *** [sub-src-install_subtargets] Error 2
make: *** [module-qtscript-install_subtargets] Error 2
Project ERROR: Unknown module(s) in QT: network
make[4]: *** [sub-weatherinfo-install_subtargets] Error 3
Project ERROR: Unknown module(s) in QT: network
make[4]: *** [sub-bearercloud-install_subtargets] Error 3
make[3]: *** [sub-network-install_subtargets] Error 2
make[3]: *** [sub-embedded-install_subtargets] Error 2
make[2]: *** [sub-svg-install_subtargets] Error 2
make[1]: *** [sub-examples-install_subtargets] Error 2
make: *** [module-qtsvg-install_subtargets] Error 2

