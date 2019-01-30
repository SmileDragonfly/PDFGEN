#include "PDFGEN.h"

STREAM_OBJ * PDF_CreateStreamObject(int streamIndex, int xOff, int yOff, CString data)
{
	STREAM_OBJ* streamObject = new STREAM_OBJ;
	streamObject->Index = streamIndex;
	streamObject->Offset.XOff = xOff;
	streamObject->Offset.YOff = yOff;
	streamObject->Data = data;
	return streamObject;
}

PAGE_OBJ * PDF_CreatePageObject(int index)
{
	PAGE_OBJ* pageObject = new PAGE_OBJ;
	pageObject->Index = index;
	pageObject->Parent = 2;
	pageObject->MediaBox.PageWidth = 595;
	pageObject->MediaBox.PageHeight = 842;
	return pageObject;
}

BOOL PDF_AddStreamObjectToPage(STREAM_OBJ * streamObj, PAGE_OBJ * pageObj)
{
	pageObj->ContentCount++;
	pageObj->Content = (int**) realloc(pageObj->Content, pageObj->ContentCount);
	if(pageObj->Content != NULL)
	{
		pageObj->Content[pageObj->ContentCount - 1] = (int*) streamObj;
		STREAM_OBJ * temp = (STREAM_OBJ*) (pageObj->Content[pageObj->ContentCount - 1]);
		if (pageObj->ContentCount >= 2)
		{
			STREAM_OBJ * temp1 = (STREAM_OBJ*)(pageObj->Content[pageObj->ContentCount - 2]);
			int a = 3;
			a++;
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL PDF_AddPageIndexToPages(int pageIndex, PAGES_OBJ * pagesObj)
{
	pagesObj->KidCount++;
	pagesObj->Kid = (int*) realloc(pagesObj->Kid, pagesObj->KidCount++);
	if(pagesObj->Kid != NULL)
	{
		pagesObj->Kid[pagesObj->KidCount - 1] = pageIndex;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void PDF_WriteBasicInfor(FILE * pFile)
{
	// Write Header
	fprintf(pFile, "%%PDF-1.2\r\n");
	// Hibit bytes
	fprintf(pFile, "%c%c%c%c%c\r\n", 0x25, 0xc7, 0xec, 0x8f, 0xa2);
	// Write infor object
	xref_off[0] = ftell(pFile);
	fprintf(pFile, "1 0 obj\r\n");
	fprintf(pFile,
		"<<\r\n"
		"  /Creator (IFI Solution - NTT data)\r\n"
		"  /Producer (IFI Solution - NTT data)\r\n"
		"  /Title (Test create pdf file)\r\n"
		"  /Author (IFI Solution - NTT data)\r\n"
		"  /Subject (TLV TOOL)\r\n"
		"  /CreationDate (D:Today)\r\n"
		">>\r\n");
	fprintf(pFile, "endobj\r\n");
	// Write catalog object
	xref_off[2] = ftell(pFile);
	fprintf(pFile, "3 0 obj\r\n");
	fprintf(pFile, "<<\r\n"
		"/Type /Catalog\r\n");
	fprintf(pFile,
		"/Pages 2 0 R\r\n"
		">>\r\n");
	fprintf(pFile, "endobj\r\n");
	// Write font object
	xref_off[3] = ftell(pFile);
	fprintf(pFile, "4 0 obj\r\n");
	fprintf(pFile,
		"<<\r\n"
		"  /Type /Font\r\n"
		"  /Subtype /Type1\r\n"
		"  /BaseFont /Times-Roman\r\n"
		"  /Encoding /WinAnsiEncoding\r\n"
		">>\r\n");
	fprintf(pFile, "endobj\r\n");
}

void PDF_WritePageObject(FILE* pFile, PAGE_OBJ * pageObj)
{
	if(pageObj)
	{
		// Write page object
		xref_off[pageObj->Index - 1] = ftell(pFile);
		fprintf(pFile, "%d 0 obj\r\n", pageObj->Index);
		fprintf(pFile,
			"<<\r\n"
			"/Type /Page\r\n"
			"/Parent %d 0 R\r\n",
			pageObj->Parent);
		fprintf(pFile, "/MediaBox [0 0 %d %d]\r\n", pageObj->MediaBox.PageWidth,
			pageObj->MediaBox.PageHeight);
		fprintf(pFile, "/Resources <<\r\n");
		fprintf(pFile, "  /Font <<\r\n");
		fprintf(pFile, "    /F1 4 0 R\r\n");
		fprintf(pFile, "  >>\r\n");
		fprintf(pFile, ">>\r\n");
		fprintf(pFile, "/Contents [\r\n");
		for(int i = 0; i < pageObj->ContentCount; i++)
		{
			STREAM_OBJ* temp = (STREAM_OBJ*) pageObj->Content[i];
			fprintf(pFile, "%d 0 R\r\n", temp->Index);
		}
		fprintf(pFile, "]\r\n");
		fprintf(pFile, ">>\r\n");
		fprintf(pFile, "endobj\r\n");
		// Write stream object
		for (int i = 0; i < pageObj->ContentCount; i++)
		{
			STREAM_OBJ* temp = (STREAM_OBJ*)pageObj->Content[i];
			xref_off[temp->Index - 1] = ftell(pFile);
			fprintf(pFile, "%d 0 obj\r\n", temp->Index);
			fprintf(pFile, "<< /Length %d >>stream\r\n", temp->Data.GetLength() + 73);
			CT2A strTemp(temp->Data);
			fprintf(pFile, "BT %d %d TD /F1 %d Tf 0.000000 0.000000 0.000000 rg 0.000000 Tc (%s) Tj ET\r\n", temp->Offset.XOff, temp->Offset.YOff, temp->Font.FontSize, strTemp);
			fprintf(pFile, "endstream\r\n");
			fprintf(pFile, "endobj\r\n");
		}
	}
}

void PDF_WritePagesObject(FILE * pFile, PAGES_OBJ * pagesObj)
{
	xref_off[1] = ftell(pFile);
	fprintf(pFile, "2 0 obj\r\n");
	fprintf(pFile, "<<\r\n"
		"/Type /Pages\r\n"
		"/Kids [ ");
	for (int i = 0; i < pagesObj->KidCount; i++)
	{
		fprintf(pFile, "%d 0 R ", pagesObj->Kid[i]);
	}
	fprintf(pFile, "]\r\n");
	fprintf(pFile, "/Count %d\r\n", pagesObj->KidCount);
	fprintf(pFile, ">>\r\n");
}

void PDF_WriteXrefAndTrailer(FILE * pFile)
{
	start_xref = ftell(pFile);
	fprintf(pFile, "xref\r\n");
	fprintf(pFile, "0 %d\r\n", xref);
	fprintf(pFile, "0000000000 65535 f\r\n");
	for (int i = 0; i < xref; i++)
	{
		fprintf(pFile, "%10.10d 00000 n\r\n", xref_off[i]);
	}
	fprintf(pFile,
		"trailer\r\n"
		"<<\r\n"
		"/Size %d\r\n",
		xref);
	fprintf(pFile, "/Root 3 0 R\r\n");
	fprintf(pFile, "/Info 1 0 R\r\n");
	/* FIXME: Not actually generating a unique ID */
	fprintf(pFile, "/ID [<%16.16x> <%16.16x>]\r\n", 0x123, 0x123);
	fprintf(pFile, ">>\r\n"
		"startxref\r\n");
	fprintf(pFile, "%d\r\n", start_xref);
	fprintf(pFile, "%%%%EOF\r\n");
}

void main()
{
	PAGES_OBJ pagesObj;
	pagesObj.Index = 2;
	FILE* pFile = fopen("test.pdf","wb");
	PDF_WriteBasicInfor(pFile);
	PAGE_OBJ * pageObj = PDF_CreatePageObject(5);
	xref++;
	PDF_AddStreamObjectToPage(PDF_CreateStreamObject(6, 20, 827, L"this is nothing1"), pageObj);
	xref++;
	PDF_AddStreamObjectToPage(PDF_CreateStreamObject(7, 20, 812, L"this is nothing2"), pageObj);
	xref++;
	PDF_AddStreamObjectToPage(PDF_CreateStreamObject(8, 20, 797, L"this is nothing3"), pageObj);
	xref++;
	PDF_AddStreamObjectToPage(PDF_CreateStreamObject(9, 20, 782, L"this is nothing4"), pageObj);
	xref++;
	PDF_AddStreamObjectToPage(PDF_CreateStreamObject(10, 20, 767, L"this is nothing5"), pageObj);
	xref++;
	PDF_WritePageObject(pFile, pageObj);	
	pagesObj.KidCount++;
	pagesObj.Kid = (int*) realloc(pagesObj.Kid, pagesObj.KidCount);
	pagesObj.Kid[pagesObj.KidCount - 1] = pageObj->Index;
	PDF_WritePagesObject(pFile, &pagesObj);
	PDF_WriteXrefAndTrailer(pFile);
	fclose(pFile);
}