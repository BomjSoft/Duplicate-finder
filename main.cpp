//---------------------------------------------------------------------------
#include "BCL.h"
//---------------------------------------------------------------------------
#include "MainForm.h"
//---------------------------------------------------------------------------

int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow)
{
   try
   {
      bcl::CApplication::Instance()->Initialize();
      bcl::CApplication::Instance()->CreateForm(new CMainForm);
      bcl::CApplication::Instance()->Run();
   }
   catch (std::exception &except)
   {
      printf("%s\n", except.what());
   }
   catch(...)
   {
      printf("Fatal error!\n");
   }
   return 0;
}
//---------------------------------------------------------------------------
