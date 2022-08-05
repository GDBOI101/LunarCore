#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <vector>
#include <iostream>

#define CURLOPT(na,t,nu) na = t + nu

#define CURLOPTTYPE_LONG          0
#define CURLOPTTYPE_OBJECTPOINT   10000
#define CURLOPTTYPE_FUNCTIONPOINT 20000
#define CURLOPTTYPE_OFF_T         30000
#define CURLOPTTYPE_BLOB          40000
#define CURLOPTTYPE_STRINGPOINT CURLOPTTYPE_OBJECTPOINT

#define CHost "http://127.0.0.1:4459"