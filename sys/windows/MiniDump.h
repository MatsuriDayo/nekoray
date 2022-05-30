#pragma once

#ifdef __MINGW32__

void Windows_SetCrashHandler(){}

#else

void Windows_SetCrashHandler();

#endif
