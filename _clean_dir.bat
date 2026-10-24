@echo off
del/S/Q "*.ncb"
del/S/Q "*.aps"
del/S/Q/AH "*.suo"
rmdir/S/Q "Debug"
rmdir/S/Q "Release"
rmdir/S/Q "DebUns"
rmdir/S/Q "RelUns"
rmdir/S/Q "X64"
rmdir/S/Q ".vs"

