//---------------------------------------------------------------------------
#include "MainForm.h"
//---------------------------------------------------------------------------
CMainForm::CMainForm() : bcl::CForm()
{
   InitDCT();
   display = false;
   SetCaption(_T("Duplicate finder"));
   onCreate.Connect(this, FormCreate);
   onResize.Connect(this, FormResize);
   onPaint.Connect(this, FormPaint);
   onMouseDoubleClick.Connect(this, FormMouseDoubleClick);

   listSelectedFiles.SetTop(10);
   listSelectedFiles.SetLeft(10);
   listSelectedFiles.onChange.Connect(this, ChangeSelectedFiles);
   AddObject(&listSelectedFiles);

   buttonAddFile.SetCaption(_T("Add Files..."));
   buttonAddFile.SetLeft(10);
   buttonAddFile.SetHeight(30);
   buttonAddFile.onClick.Connect(this, AddFileClick);
   AddObject(&buttonAddFile);

   buttonAddFolder.SetCaption(_T("Add Folder..."));
   buttonAddFolder.SetHeight(30);
   buttonAddFolder.onClick.Connect(this, AddFolderClick);
   AddObject(&buttonAddFolder);

   buttonRemove.SetCaption(_T("Remove"));
   buttonRemove.SetHeight(30);
   buttonRemove.SetEnable(false);
   buttonRemove.onClick.Connect(this, RemoveClick);
   AddObject(&buttonRemove);

   buttonModify.SetCaption(_T("Modify..."));
   buttonModify.SetHeight(30);
   buttonModify.SetEnable(false);
   buttonModify.onClick.Connect(this, ModifyClick);
   AddObject(&buttonModify);

   buttonNext.SetCaption(_T("Next >"));
   buttonNext.SetHeight(30);
   buttonNext.onClick.Connect(this, NextClick);
   AddObject(&buttonNext);

   progressScan.SetLeft(10);
   progressScan.SetHeight(30);
   progressScan.SetVisible(false);
   AddObject(&progressScan);

   progressCalculate.SetLeft(10);
   progressCalculate.SetHeight(30);
   progressCalculate.SetVisible(false);
   AddObject(&progressCalculate);

   progressCompare.SetLeft(10);
   progressCompare.SetHeight(30);
   progressCompare.SetVisible(false);
   AddObject(&progressCompare);

   imageListDescription.SetTop(10);
   imageListDescription.SetLeft(20);
   imageListDescription.SetHeight(15);
   imageListDescription.SetVisible(false);
   AddObject(&imageListDescription);

   imageDescription.SetTop(10);
   imageDescription.SetHeight(15);
   imageDescription.SetVisible(false);
   imageDescription.SetText(_T("Image preview"));
   AddObject(&imageDescription);

   deleteListDescription.SetLeft(20);
   deleteListDescription.SetHeight(15);
   deleteListDescription.SetVisible(false);
   deleteListDescription.SetText(_T("Check the item that you wish to delete"));
   AddObject(&deleteListDescription);

   imageList.SetTop(30);
   imageList.SetLeft(10);
   imageList.SetVisible(false);
   imageList.onChange.Connect(this, ImageChange);
   AddObject(&imageList);

   deleteList.SetLeft(10);
   deleteList.SetVisible(false);
   deleteList.onCheck.Connect(this, DeleteCheck);
   deleteList.onChange.Connect(this, DeleteChange);
   AddObject(&deleteList);

   buttonFinish.SetCaption(_T("Finish"));
   buttonFinish.SetHeight(30);
   buttonFinish.SetVisible(false);
   buttonFinish.onClick.Connect(this, FinishClick);
   AddObject(&buttonFinish);
}
//---------------------------------------------------------------------------

CMainForm::~CMainForm()
{
}
//---------------------------------------------------------------------------

void CMainForm::FormCreate(bcl::CObject *Sender)
{
   deleteList.AddColumn(_T("File name"));
   deleteList.AddColumn(_T("Picture size"));
   deleteList.AddColumn(_T("File size"));
   deleteList.AddColumn(_T("File path"));
}
//---------------------------------------------------------------------------

void CMainForm::FormResize(bcl::CObject *Sender)
{
   const int clientWidth = GetClientWidth();
   const int clientHeight = GetClientHeight();

   listSelectedFiles.SetWidth(clientWidth - 20);
   listSelectedFiles.SetHeight(clientHeight - 60);

   buttonAddFile.SetTop(clientHeight - 40);
   buttonAddFile.SetWidth(clientWidth / 8);

   buttonAddFolder.SetLeft(clientWidth / 8 + 20);
   buttonAddFolder.SetTop(clientHeight - 40);
   buttonAddFolder.SetWidth(clientWidth / 8);

   buttonRemove.SetLeft(clientWidth / 4 + 30);
   buttonRemove.SetTop(clientHeight - 40);
   buttonRemove.SetWidth(clientWidth / 8);

   buttonModify.SetLeft(clientWidth / 8 * 3 + 40);
   buttonModify.SetTop(clientHeight - 40);
   buttonModify.SetWidth(clientWidth / 8);

   buttonNext.SetLeft(clientWidth - clientWidth / 8 - 10);
   buttonNext.SetTop(clientHeight - 40);
   buttonNext.SetWidth(clientWidth / 8);

   progressScan.SetTop(clientHeight / 2 - 55);
   progressScan.SetWidth(clientWidth - 20);

   progressCalculate.SetTop(clientHeight / 2 - 15);
   progressCalculate.SetWidth(clientWidth - 20);

   progressCompare.SetTop(clientHeight / 2 + 35);
   progressCompare.SetWidth(clientWidth - 20);

   imageListDescription.SetWidth(clientWidth / 2 - 25);

   imageDescription.SetLeft(clientWidth / 2 + 25);
   imageDescription.SetWidth(clientWidth / 2 - 25);

   deleteListDescription.SetTop((clientHeight - 70) / 2 + 20);
   deleteListDescription.SetWidth(clientWidth / 2 - 25);

   imageList.SetWidth(clientWidth / 2 - 15);
   imageList.SetHeight((clientHeight - 70) / 2 - 20);

   deleteList.SetTop((clientHeight - 70) / 2 + 40);
   deleteList.SetWidth(clientWidth  - 20);
   deleteList.SetHeight(clientHeight / 2 - 55);

   buttonFinish.SetLeft(clientWidth - clientWidth / 8 - 10);
   buttonFinish.SetTop(clientHeight - 40);
   buttonFinish.SetWidth(clientWidth / 8);
}
//---------------------------------------------------------------------------

void CMainForm::FormPaint(bcl::CObject *Sender)
{
   if (display)
   {
      const int clientWidth = GetClientWidth();
      const int clientHeight = GetClientHeight();
      Canvas.ProportionalStretchDraw(clientWidth / 2 + 5, 25, clientWidth / 2 - 15, clientHeight / 2 - 35, &displayImage);
   }
}
//---------------------------------------------------------------------------

void CMainForm::AddFileClick(bcl::CObject *Sender)
{
   bcl::COpenFileDialog dialog;
   dialog.SetOwner(this);
   dialog.SetFilter(_T("\0*\0"));
   dialog.SetMultiselect(true);
   if (dialog.Execute())
   {
      for (size_t i = 0; i < dialog.GetFileCount(); i++)
      {
         listSelectedFiles.AddString(dialog.GetFileName(i));
      }
   }
}
//---------------------------------------------------------------------------

void CMainForm::AddFolderClick(bcl::CObject *Sender)
{
   std::tstring str;
   if (bcl::SelectDirectory(hwnd, _T("Select folder to add"), str))
   {
      str.push_back('*');
      listSelectedFiles.AddString(str.c_str());
   }
}
//---------------------------------------------------------------------------

void CMainForm::ChangeSelectedFiles(bcl::CObject *Sender)
{
   bool value = (listSelectedFiles.GetSelectItem() >= 0);
   buttonRemove.SetEnable(value);
   buttonModify.SetEnable(value);
}
//---------------------------------------------------------------------------

void CMainForm::RemoveClick(bcl::CObject *Sender)
{
   int selectItem = listSelectedFiles.GetSelectItem();
   if (selectItem >= 0)
   {
      listSelectedFiles.DeleteItem(selectItem);
   }
}
//---------------------------------------------------------------------------

void CMainForm::ModifyClick(bcl::CObject *Sender)
{
   int selectItem = listSelectedFiles.GetSelectItem();
   if (selectItem >= 0)
   {
      std::tstring line = listSelectedFiles.GetString(selectItem);
      if (line.c_str()[line.size() - 1] != '*')
      {
         bcl::COpenFileDialog dialog;
         dialog.SetOwner(this);
         dialog.SetFilter(_T("\0*\0"));

         dialog.SetMultiselect(true);
         if (dialog.Execute())
         {
            listSelectedFiles.DeleteItem(selectItem);
            for (size_t i = 0; i < dialog.GetFileCount(); i++)
            {
               listSelectedFiles.InsertString(dialog.GetFileName(i), selectItem + i);
            }
         }
      }
      else
      {
         std::tstring str;
         if (bcl::SelectDirectory(hwnd, _T("Select folder to add"), str))
         {
            str.push_back('*');
            listSelectedFiles.DeleteItem(selectItem);
            listSelectedFiles.InsertString(str.c_str(), selectItem);
         }
      }
   }
}
//---------------------------------------------------------------------------

DWORD WINAPI StartCompare(CMainForm *Form)
{
   Form->SearchDuplicate();
   return 0;
}
//---------------------------------------------------------------------------

void CMainForm::NextClick(bcl::CObject *Sender)
{
   listSelectedFiles.SetVisible(false);
   buttonAddFile.SetVisible(false);
   buttonAddFolder.SetVisible(false);
   buttonRemove.SetVisible(false);
   buttonModify.SetVisible(false);
   buttonNext.SetVisible(false);
   progressScan.SetVisible(true);
   progressCalculate.SetVisible(true);
   progressCompare.SetVisible(true);
   CreateThread(nullptr, 0, (DWORD WINAPI (*)(LPVOID))StartCompare, this, 0, nullptr);
}
//---------------------------------------------------------------------------

DWORD CaculateDist(const BYTE Value1[HASH_LEN], const BYTE Value2[HASH_LEN])
{
   DWORD result = 0;
   for (int i = 0; i < HASH_LEN; i++)
   {
      BYTE temp = Value1[i] ^ Value2[i];
      temp -= (temp >> 1) & 0x55;
      temp = (temp & 0x33) + ((temp >> 2) & 0x33);
      temp = (temp + (temp >> 4)) & 0x0F;
      result += temp;
   }
   return result;
}
//---------------------------------------------------------------------------

void CMainForm::SearchDuplicate()
{
   progressScan.SetStep(1);
   progressCalculate.SetStep(1);
   progressCompare.SetStep(1);
   progressScan.SetRange(0, listSelectedFiles.GetItemCount());
   hashList.clear();
   groupList.clear();
   std::set<std::tstring> tempList;
   for (int i = 0; i < listSelectedFiles.GetItemCount(); i++)
   {
      std::tstring line = listSelectedFiles.GetString(i);
      if (line.c_str()[line.size() - 1] != '*')
      {
         tempList.insert(line);
      }
      else
      {
         SearchDuplicateFolder(line, tempList);
      }
      progressScan.Update();
   }

   progressCalculate.SetRange(0, tempList.size());
   hashList.reserve(tempList.size());
   for (auto imageName = tempList.begin(); imageName != tempList.end(); imageName++)
   {
      SImageInfo temp;
      temp.name = *imageName;
      HANDLE handle = CreateFile(temp.name.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      if (handle != INVALID_HANDLE_VALUE)
      {
         GetFileSizeEx(handle, &temp.size);
         CloseHandle(handle);
         if (CalculateHash(temp.name.c_str(), temp.hash, temp.SizeX, temp.SizeY))
         {
            temp.group = 0;
            temp.deleted = false;
            hashList.push_back(temp);
         }
      }
      progressCalculate.Update();
   }
   tempList.clear();
   progressCompare.SetRange(0, hashList.size());

   for (size_t i = 1; i < hashList.size(); i++)
   {
      for (size_t o = 0; o < i; o++)
      {
         if (CaculateDist(hashList[i].hash, hashList[o].hash) <= MAX_DUPLICATE_DELTA)
         {
            if (hashList[i].group == 0)
            {
               if (hashList[o].group == 0)
               {
                  std::vector<SImageInfo*> tempVector;
                  groupList.push_back(tempVector);
                  hashList[i].group = hashList[o].group = groupList.size();
                  groupList[hashList[o].group - 1].push_back(&hashList[o]);
                  groupList[hashList[o].group - 1].push_back(&hashList[i]);
               }
               else
               {
                  hashList[i].group = hashList[o].group;
                  groupList[hashList[o].group - 1].push_back(&hashList[i]);
               }
            }
            else
            {
               if (hashList[o].group == 0)
               {
                  hashList[o].group = hashList[i].group;
                  groupList[hashList[i].group - 1].push_back(&hashList[o]);
               }
               else
               {
                  DWORD deleteGroup = hashList[i].group;
                  DWORD targetGroup = hashList[o].group;
                  if (deleteGroup != targetGroup)
                  {
                     for (size_t p = 0; p < groupList[deleteGroup - 1].size(); p++)
                     {
                        groupList[deleteGroup - 1][p]->group = targetGroup;
                        groupList[targetGroup - 1].push_back(groupList[deleteGroup - 1][p]);
                     }
                     for (size_t p = deleteGroup; p < groupList.size(); p++)
                     {
                        groupList[p - 1] = groupList[p];
                     }
                     groupList.pop_back();
                     for (size_t p = 0; p < hashList.size(); p++)
                     {
                        if (hashList[p].group > deleteGroup)
                        {
                           hashList[p].group--;
                        }
                     }
                  }
               }
            }
         }
      }
      progressCompare.Update();
   }

   progressScan.SetVisible(false);
   progressCalculate.SetVisible(false);
   progressCompare.SetVisible(false);

   imageListDescription.SetText((_T("Sets of duplicates: ") + std::to_tstring(groupList.size())).c_str());
   imageListDescription.SetVisible(true);
   imageDescription.SetVisible(true);
   deleteListDescription.SetVisible(true);
   imageList.SetVisible(true);
   deleteList.SetVisible(true);
   buttonFinish.SetVisible(true);

   for (size_t i = 0; i < groupList.size(); i++)
   {
      const TCHAR *temp = groupList[i][0]->name.c_str();
      int len = _tcslen(temp);
      while (len > 0)
      {
         if ((temp[len] == '\\')||(temp[len] == '/'))
         {
            temp = &temp[len + 1];
            break;
         }
         len--;
      }
      std::tstring str = _T("(") + std::to_tstring(groupList[i].size()) + _T(" founds) ") + temp;
      imageList.AddString(str.c_str());
   }
   display = true;
}
//---------------------------------------------------------------------------

void CMainForm::SearchDuplicateFolder(std::tstring Folder, std::set<std::tstring> &List)
{
   std::vector<std::tstring> folderList;
   std::tstring path = Folder;
   path.pop_back();
   WIN32_FIND_DATA data;
   HANDLE handle = FindFirstFile(Folder.c_str(), &data);
   if (handle != INVALID_HANDLE_VALUE)
   {
      do
      {
         if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
         {
            if (data.cFileName[0] != '.')
            {
               std::tstring temp = path;
               temp.append(data.cFileName);
               temp.append(_T("\\*"));
               folderList.push_back(temp);
            }
         }
         else
         {
            std::tstring line = path;
            line.append(data.cFileName);
            List.insert(line);
         }
      } while (FindNextFile(handle, &data));
      FindClose(handle);
   }
   for (size_t i = 0; i < folderList.size(); i++)
   {
      SearchDuplicateFolder(folderList[i], List);
   }
}
//---------------------------------------------------------------------------

void CMainForm::ImageChange(bcl::CObject *Sender)
{
   deleteList.Clear();
   int group = imageList.GetSelectItem();
   if (group >= 0)
   {
      for (size_t i = 0; i < groupList[group].size(); i++)
      {
         const TCHAR *temp = groupList[group][i]->name.c_str();
         int len = _tcslen(temp);
         while (len > 0)
         {
            if ((temp[len] == '\\')||(temp[len] == '/'))
            {
               temp = &temp[len + 1];
               break;
            }
            len--;
         }
         int index = deleteList.AddString(temp, groupList[group][i]->deleted);
         deleteList.SetString(index, 1, (std::to_tstring(groupList[group][i]->SizeX) + _T("x") + std::to_tstring(groupList[group][i]->SizeY)).c_str());
         deleteList.SetString(index, 2, (std::to_tstring(groupList[group][i]->size.QuadPart) + _T(" bytes")).c_str());
         deleteList.SetString(index, 3, groupList[group][i]->name.c_str());
      }
   }
}
//---------------------------------------------------------------------------

void CMainForm::DeleteCheck(bcl::CObject *Sender, int Item, bool Value)
{
   int group = imageList.GetSelectItem();
   groupList[group][Item]->deleted = Value;
}
//---------------------------------------------------------------------------

void CMainForm::DeleteChange(bcl::CObject *Sender)
{
   const int imageBufferSize = 10;
   struct STempImage
   {
      std::tstring *name;
      STexture *image;

      STempImage()
      {
         name = nullptr;
         image = nullptr;
      }
   };
   static STempImage imageBuffer[imageBufferSize];

   int group = imageList.GetSelectItem();
   int item = deleteList.GetSelectItem();
   if ((unsigned(group) < groupList.size())&&(unsigned(item) < groupList[group].size()))
   {
      STexture *image = nullptr;
      for (int i = 0; i < imageBufferSize; i++)
      {
         if ((imageBuffer[i].name != nullptr)&&(*imageBuffer[i].name == groupList[group][item]->name))
         {
            STempImage temp = imageBuffer[i];
            for (int o = i; o > 0; o--)
            {
               imageBuffer[o] = imageBuffer[o - 1];
            }
            imageBuffer[0] = temp;
            image = imageBuffer[0].image;
            break;
         }
      }
      if (image == nullptr)
      {
         image = CTexture::LoadData(groupList[group][item]->name.c_str());
         CTexture::Delete(imageBuffer[imageBufferSize - 1].image);
         for (int i = imageBufferSize - 1; i > 0; i--)
         {
            imageBuffer[i] = imageBuffer[i - 1];
         }
         imageBuffer[0].name = &groupList[group][item]->name;
         imageBuffer[0].image = image;
      }
      if (image != nullptr)
      {
         displayImage.Create(image->sizeX, image->sizeY, image->channels, image->bytesPerChannel, image->data);
      }
   }
   else
   {
      displayImage.Clear();
   }
   Redraw();
}
//---------------------------------------------------------------------------

void CMainForm::FormMouseDoubleClick(bcl::CObject *Sender, EMouseButton MouseButton, int X, int Y)
{
   if ((display)&&(X > GetClientWidth() / 2)&&(Y < GetClientHeight() / 2))
   {
      int group = imageList.GetSelectItem();
      int item = deleteList.GetSelectItem();
      if ((unsigned(group) < groupList.size())&&(unsigned(item) < groupList[group].size()))
      {
         bcl::RemoteOpenFile(groupList[group][item]->name.c_str());
      }
   }
}
//---------------------------------------------------------------------------

DWORD WINAPI StartDelete(CMainForm *Form)
{
   Form->DeleteDuplicate();
   return 0;
}
//---------------------------------------------------------------------------

void CMainForm::FinishClick(bcl::CObject *Sender)
{
   imageListDescription.SetVisible(false);
   imageDescription.SetVisible(false);
   deleteListDescription.SetVisible(false);
   imageList.SetVisible(false);
   deleteList.SetVisible(false);
   buttonFinish.SetVisible(false);
   display = false;
   Redraw();

   progressCalculate.SetPosition(0);
   progressCalculate.SetRange(0, hashList.size());
   progressCalculate.SetVisible(true);
   CreateThread(nullptr, 0, (DWORD WINAPI (*)(LPVOID))StartDelete, this, 0, nullptr);
}
//---------------------------------------------------------------------------

void CMainForm::DeleteDuplicate()
{
   for (size_t i = 0; i < hashList.size(); i++)
   {
      if (hashList[i].deleted)
      {
         bcl::MoveToTrash(hashList[i].name.c_str());
      }
      progressCalculate.Update();
   }
   ExitProcess(0);
}
//---------------------------------------------------------------------------
