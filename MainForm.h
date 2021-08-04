//---------------------------------------------------------------------------
#pragma once
//---------------------------------------------------------------------------
#include <set>
#include "BCL.h"
//---------------------------------------------------------------------------
#include "PerceptiveHash.h"
#include "Texture.h"
//---------------------------------------------------------------------------
#define MAX_DUPLICATE_DELTA   4
//---------------------------------------------------------------------------
struct SImageInfo
{
public:
   std::tstring name;
   LARGE_INTEGER size;
   BYTE hash[HASH_LEN];
   DWORD SizeX;
   DWORD SizeY;
   DWORD group;
   bool deleted;

   bool operator<(const SImageInfo &Operand) const
   {
      return name < Operand.name;
   }
};
//---------------------------------------------------------------------------
DWORD CaculateDist(const BYTE Value1[HASH_LEN], const BYTE Value2[HASH_LEN]);
//---------------------------------------------------------------------------
class CMainForm : public bcl::CForm
{
private:
   bcl::CListBox listSelectedFiles;
   bcl::CButton buttonAddFile;
   bcl::CButton buttonAddFolder;
   bcl::CButton buttonRemove;
   bcl::CButton buttonModify;
   bcl::CButton buttonNext;

   bcl::CProgressBar progressScan;
   bcl::CProgressBar progressCalculate;
   bcl::CProgressBar progressCompare;

   bcl::CLabel imageListDescription;
   bcl::CLabel imageDescription;
   bcl::CLabel deleteListDescription;
   bcl::CListBox imageList;
   bcl::CCheckListBox deleteList;
   bcl::CButton buttonFinish;

   std::vector<SImageInfo> hashList;
   std::vector<std::vector<SImageInfo*> > groupList;

   bool display;
   bcl::CBitmap displayImage;

   void FormCreate(bcl::CObject *Sender);
   void FormResize(bcl::CObject *Sender);
   void FormPaint(bcl::CObject *Sender);
   void FormMouseDoubleClick(bcl::CObject *Sender, EMouseButton MouseButton, int X, int Y);

   void ChangeSelectedFiles(bcl::CObject *Sender);

   void AddFileClick(bcl::CObject *Sender);
   void AddFolderClick(bcl::CObject *Sender);
   void RemoveClick(bcl::CObject *Sender);
   void ModifyClick(bcl::CObject *Sender);

   void NextClick(bcl::CObject *Sender);

   void ImageChange(bcl::CObject *Sender);
   void DeleteCheck(bcl::CObject *Sender, int Item, bool Value);
   void DeleteChange(bcl::CObject *Sender);
   void FinishClick(bcl::CObject *Sender);

   static void SearchDuplicateFolder(std::tstring Folder, std::set<std::tstring> &List);

public:
   CMainForm();
   virtual ~CMainForm();

   void SearchDuplicate();
   void DeleteDuplicate();
};
//---------------------------------------------------------------------------
