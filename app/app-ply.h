
#ifndef APP_PLY_H
#define APP_PLY_H

#include "../PcrLib/pcrlib.h"
namespace pcrapp
{
	struct   LibCallback;
	int readPlyFile(const char *pPath, pcrlib::IPcrLib *pLib, pcrlib::LibCallback *pCb);
}

#endif