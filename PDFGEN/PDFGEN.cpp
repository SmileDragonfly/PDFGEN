#include "PDFGEN.h"
CPDFGEN::CPDFGEN()
{
	m_xref = 0;
	m_startXrefAddress = 0;
	m_pagesObj.Index = 2;
}
STREAM_OBJ * CPDFGEN::PDF_CreateStreamObject(int xOff, int yOff, CString data)
{
	m_xref++;
	STREAM_OBJ* streamObject = new STREAM_OBJ;
	streamObject->Index = m_xref;
	streamObject->Offset.XOff = xOff;
	streamObject->Offset.YOff = yOff;
	streamObject->Data = data;
	return streamObject;
}

PAGE_OBJ * CPDFGEN::PDF_CreatePageObject()
{
	m_xref++;
	PAGE_OBJ* pageObject = new PAGE_OBJ;
	pageObject->Index = m_xref;
	pageObject->Parent = m_pagesObj.Index;
	pageObject->MediaBox.PageWidth = 595;
	pageObject->MediaBox.PageHeight = 842;
	PDF_AddPageIndexToPages(m_xref, &m_pagesObj);
	m_currentPageObj = pageObject;
	return pageObject;
}

BOOL CPDFGEN::PDF_AddStreamObjectToPage(int xOff, int yOff, CString data)
{
	STREAM_OBJ* streamObj = PDF_CreateStreamObject(xOff,yOff,data);
	m_currentPageObj->ContentCount++;
	m_currentPageObj->Content = (int**)realloc(m_currentPageObj->Content, m_currentPageObj->ContentCount * sizeof(int*));
	if (m_currentPageObj->Content != NULL)
	{
		m_currentPageObj->Content[m_currentPageObj->ContentCount - 1] = (int*)streamObj;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CPDFGEN::PDF_AddPageIndexToPages(int pageIndex, PAGES_OBJ * pagesObj)
{
	pagesObj->KidCount++;
	pagesObj->Kid = (int*)realloc(pagesObj->Kid, pagesObj->KidCount * sizeof(int));
	if (pagesObj->Kid != NULL)
	{
		pagesObj->Kid[pagesObj->KidCount - 1] = pageIndex;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void CPDFGEN::PDF_DeleteCurrentPage()
{
	if (m_currentPageObj)
	{
		for (int i = 0; i < m_currentPageObj->ContentCount; i++)
		{
			STREAM_OBJ* streamObjTemp = (STREAM_OBJ*) m_currentPageObj->Content[i];
			delete streamObjTemp;
		}
		free(m_currentPageObj->Content);
	}
}

void CPDFGEN::PDF_DeletePagesObject()
{
	free(m_pagesObj.Kid);
}

void CPDFGEN::PDF_WriteBasicInfor(FILE * pFile)
{
	// Write Header
	fprintf(pFile, "%%PDF-1.2\r\n");
	// Hibit bytes
	fprintf(pFile, "%c%c%c%c%c\r\n", 0x25, 0xc7, 0xec, 0x8f, 0xa2);
	// Write infor object
	m_xrefOffset.push_back(ftell(pFile));
	m_xref++;
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
	m_xref = m_xref + 2;
	m_xrefOffset.push_back(ftell(pFile));
	fprintf(pFile, "3 0 obj\r\n");
	fprintf(pFile, "<<\r\n"
		"/Type /Catalog\r\n");
	fprintf(pFile,
		"/Pages 2 0 R\r\n"
		">>\r\n");
	fprintf(pFile, "endobj\r\n");
	// Write font object
	m_xrefOffset.push_back(ftell(pFile));
	m_xref++;
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

void CPDFGEN::PDF_WritePageObject(FILE* pFile)
{
	if (m_currentPageObj)
	{
		// Write page object
		m_xrefOffset.push_back(ftell(pFile));
		fprintf(pFile, "%d 0 obj\r\n", m_currentPageObj->Index);
		fprintf(pFile,
			"<<\r\n"
			"/Type /Page\r\n"
			"/Parent %d 0 R\r\n",
			m_currentPageObj->Parent);
		fprintf(pFile, "/MediaBox [0 0 %d %d]\r\n", m_currentPageObj->MediaBox.PageWidth,
			m_currentPageObj->MediaBox.PageHeight);
		fprintf(pFile, "/Resources <<\r\n");
		fprintf(pFile, "  /Font <<\r\n");
		fprintf(pFile, "    /F1 4 0 R\r\n");
		fprintf(pFile, "  >>\r\n");
		fprintf(pFile, ">>\r\n");
		fprintf(pFile, "/Contents [\r\n");
		for (int i = 0; i < m_currentPageObj->ContentCount; i++)
		{
			STREAM_OBJ* temp = (STREAM_OBJ*)m_currentPageObj->Content[i];
			fprintf(pFile, "%d 0 R\r\n", temp->Index);
		}
		fprintf(pFile, "]\r\n");
		fprintf(pFile, ">>\r\n");
		fprintf(pFile, "endobj\r\n");
		// Write stream object
		for (int i = 0; i < m_currentPageObj->ContentCount; i++)
		{
			STREAM_OBJ* temp = (STREAM_OBJ*)m_currentPageObj->Content[i];
			m_xrefOffset.push_back(ftell(pFile));
			fprintf(pFile, "%d 0 obj\r\n", temp->Index);
			fprintf(pFile, "<< /Length %d >>stream\r\n", temp->Data.GetLength() + 73);
			CT2A strTemp(temp->Data);
			fprintf(pFile, "BT %d %d TD /F1 %d Tf 0.000000 0.000000 0.000000 rg 0.000000 Tc (%s) Tj ET\r\n", temp->Offset.XOff, temp->Offset.YOff, temp->Font.FontSize, strTemp);
			fprintf(pFile, "endstream\r\n");
			fprintf(pFile, "endobj\r\n");
		}
		PDF_DeleteCurrentPage();
	}
}

void CPDFGEN::PDF_WritePagesObject(FILE * pFile)
{
	m_xrefOffset.push_back(ftell(pFile));
	fprintf(pFile, "2 0 obj\r\n");
	fprintf(pFile, "<<\r\n"
		"/Type /Pages\r\n"
		"/Kids [ ");
	for (int i = 0; i < m_pagesObj.KidCount; i++)
	{
		fprintf(pFile, "%d 0 R ", m_pagesObj.Kid[i]);
	}
	fprintf(pFile, "]\r\n");
	fprintf(pFile, "/Count %d\r\n", m_pagesObj.KidCount);
	fprintf(pFile, ">>\r\n");
	PDF_DeletePagesObject();
}

void CPDFGEN::PDF_WriteXrefAndTrailer(FILE * pFile)
{
	m_startXrefAddress = ftell(pFile);
	fprintf(pFile, "xref\r\n");
	fprintf(pFile, "0 %d\r\n", m_xref);
	fprintf(pFile, "0000000000 65535 f\r\n");
	for (int i = 0; i < m_xref; i++)
	{
		if (i == 1)
		{
			fprintf(pFile, "%10.10d 00000 n\r\n", m_xrefOffset.at(m_xref - 1));
		}
		else if (i == (m_xref - 1))
		{
			fprintf(pFile, "%10.10d 00000 n\r\n", m_xrefOffset.at(1));
		}
		else
		{
			fprintf(pFile, "%10.10d 00000 n\r\n", m_xrefOffset.at(i));
		}
	}
	fprintf(pFile,
		"trailer\r\n"
		"<<\r\n"
		"/Size %d\r\n",
		m_xref);
	fprintf(pFile, "/Root 3 0 R\r\n");
	fprintf(pFile, "/Info 1 0 R\r\n");
	/* FIXME: Not actually generating a unique ID */
	fprintf(pFile, "/ID [<%16.16x> <%16.16x>]\r\n", 0x123, 0x123);
	fprintf(pFile, ">>\r\n"
		"startxref\r\n");
	fprintf(pFile, "%d\r\n", m_startXrefAddress);
	fprintf(pFile, "%%%%EOF\r\n");
}

void main()
{
	FILE* pFile = fopen("test.pdf", "wb");
	CPDFGEN pdfObj;
	pdfObj.PDF_WriteBasicInfor(pFile);
	for(int i = 0; i < 100; i++)
	{
		pdfObj.PDF_CreatePageObject();
		pdfObj.PDF_AddStreamObjectToPage(20, 827, L"Toi lac quan giua dam dong");
		pdfObj.PDF_AddStreamObjectToPage(20, 812, L"Nhung khi mot minh thi lai khong");
		pdfObj.PDF_AddStreamObjectToPage(20, 797, L"Co to ra la minh on nhung sau ben trong nuoc mat la bien rong");
		pdfObj.PDF_AddStreamObjectToPage(20, 782, L"Lam luc chi muon co ai do");
		pdfObj.PDF_AddStreamObjectToPage(20, 767, L"Dang tay om lay toi vao long");
		pdfObj.PDF_AddStreamObjectToPage(20, 752, L"Cho tieng cuoi trong mat duoc vang vong co don 1 lan roi khoi nhung khoang trong");
		pdfObj.PDF_WritePageObject(pFile);
	}
	pdfObj.PDF_WritePagesObject(pFile);
	pdfObj.PDF_WriteXrefAndTrailer(pFile);
	fclose(pFile);
}