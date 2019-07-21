
#ifndef APP_LAS_H
#define APP_LAS_H

#include "../PcrLib/pcrlib.h"
namespace pcrapp
{
	void readLasFile(const char *pPath, pcrlib::IPcrLib *pLib);
}

#endif