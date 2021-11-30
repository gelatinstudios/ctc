@echo off 
cl ctc.c /Gm- /GR- /EHa- /GS- /Gs99999999 /O2 /Oi /Zi /link kernel32.lib user32.lib shell32.lib /incremental:no /NODEFAULTLIB /subsystem:console /opt:ref 
