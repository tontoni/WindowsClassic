
#include "windowsclassicjni_cpp.h"
#include <windowsclassic.h>

void WindowsClassic_MessageBox()
{
	MessageBoxClassic(NULL, NULL, "It works omfg!", "JNI Test", MB_OK | MB_ICONINFORMATION);
}