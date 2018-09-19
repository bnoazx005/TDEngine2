#include <iostream>
#include <TDEngine2.h>


#pragma comment(lib, "TDEngine2.lib")


using namespace TDEngine2;


int main(int argc, char** argv)
{
	E_RESULT_CODE result = RC_OK;

	IWindowSystem* pWindow = TDEngine2::CreateWin32WindowSystem("Sandbox Game", 800, 600, P_RESIZEABLE, result);

	pWindow->Run([&pWindow]() 
	{
		/// render's code here

		if (GetKeyState(VK_ESCAPE) & 0x8000)
		{
			pWindow->Quit();
		}
	});

	pWindow->Free();

	return 0;
}