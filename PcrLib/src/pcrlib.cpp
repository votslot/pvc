

#include <stdio.h>
#include "..\PcrLib.h"
#include <string>
#include "cshader.h"
#include "OpenGL\wave-test.cs.glsl"

namespace pcrlib 
{

	class ThePcrLib :public IPcrLib
	{
		void runTest() 
		{
			ICShader *pCsh = ICShader::GetNew();
			pCsh->initFromSource(cs_wave_test.c_str());
		}
	};


	
	IPcrLib* IPcrLib::Init()
	{
		return  new ThePcrLib();
	}
	
}

