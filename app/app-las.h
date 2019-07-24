
#ifndef APP_LAS_H
#define APP_LAS_H

#include "../PcrLib/pcrlib.h"
namespace pcrapp
{
	struct   LibCallback;
	void readLasFile(const char *pPath, pcrlib::IPcrLib *pLib, pcrlib::LibCallback *pCb);
}

#endif