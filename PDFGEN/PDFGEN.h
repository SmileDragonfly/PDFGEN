#pragma once
#define _AFXDLL
#include <iostream>
#include <fstream>
#include <vector>
#include <afx.h>

struct MEDIA_BOX
{
	int PageWidth;
	int PageHeight;
};
struct OFFSET
{
	int XOff;
	int YOff;
};
struct FONT
{
	int FontObjectIndex;
	int FontSize;
	FONT()
	{
		FontSize = 12;
	}
};
struct PAGES_OBJ
{
	int Index;			//Object index in file
	int* Kid;			//Page_obj array index
	int KidCount;		//Page_obj count
	PAGES_OBJ()
	{
		Kid = NULL;
		KidCount = 0;
	}
};
struct PAGE_OBJ
{
	int Index;			//Object index in file
	int Parent;			//Pages_obj index
	MEDIA_BOX MediaBox;	//Width and height of page
	int ResoureFont;	//Font index obj
	int** Content;		//Stream_obj array, contain all object address
	int ContentCount;	//Stream_obj count
	PAGE_OBJ()
	{
		Content = NULL;
		ContentCount = 0;
	}
};
struct STREAM_OBJ
{
	int Index;			//Object index in file
	OFFSET Offset;		//Xoff and Yoff of this object in page
	FONT Font;			//Index of font object and size to display
	CString Data;		//Text data to display
};

// PDF File structure
// I.HEADER
// II.OBJECT
// 1.Infor
// 3.Catalog(indirect reference to pages)
// 4.Font
// {
// 5.Page(contain contents which indirect reference to stream object)
// 6.Stream Object
// 7.Stream Object
// ....
// }loop;
// 2.Pages(indirect reference to page)
// III. REFERENCE AND TRAILER
// Xref table (object offset in file)
// Trailer
// StartXref

class CPDFGEN
{
public:
	CPDFGEN();
	PAGE_OBJ * PDF_CreatePageObject();
	BOOL PDF_AddStreamObjectToPage(int xOff, int yOff, CString data);
	void PDF_WriteBasicInfor(FILE* pFile);
	void PDF_WritePageObject(FILE* pFile);
	void PDF_WritePagesObject(FILE* pFile);
	void PDF_WriteXrefAndTrailer(FILE* pFile);
protected:
	STREAM_OBJ * PDF_CreateStreamObject(int xOff, int yOff, CString data);
	BOOL PDF_AddPageIndexToPages(int pageIndex, PAGES_OBJ * pagesObj);
	void PDF_DeleteCurrentPage();
	void PDF_DeletePagesObject();
	int m_xref;
	int m_startXrefAddress;
	std::vector<int> m_xrefOffset;
	PAGES_OBJ m_pagesObj;
	PAGE_OBJ* m_currentPageObj;
};