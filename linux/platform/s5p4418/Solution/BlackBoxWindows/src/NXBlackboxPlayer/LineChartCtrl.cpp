#include "stdafx.h"
#include "LineChartCtrl.h"
#include "Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define GRAPH_WIDTH		15

/////////////////////////////////////////////////////////////////////////////
// CLineChartCtrl

CLineChartCtrl::CLineChartCtrl()
{
	CLineChartCtrl::RegisterWndClass(AfxGetInstanceHandle());
}

CLineChartCtrl::~CLineChartCtrl()
{
	int nCount = m_items.GetSize();
	
	for (int i = 0; i < nCount; i++)
		delete m_items.GetAt(i);
	m_items.RemoveAll();
}


BEGIN_MESSAGE_MAP(CLineChartCtrl, CWnd)
	//{{AFX_MSG_MAP(CLineChartCtrl)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CLineChartCtrl::RegisterWndClass(HINSTANCE hInstance)
{
	WNDCLASS wc;
	wc.lpszClassName = L"LINE_CHART";	// matches class name in client
	wc.hInstance = hInstance;
	wc.lpfnWndProc = ::DefWindowProc;
	wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = 0;
	wc.lpszMenuName = NULL;
	wc.hbrBackground = (HBRUSH) ::GetStockObject(LTGRAY_BRUSH);
	wc.style = CS_GLOBALCLASS;	// To be modified
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;

	return (::RegisterClass(&wc) != 0);
}

void CLineChartCtrl::InvalidateCtrl()
{
	CClientDC dc(this);
	CRect rcClient;
	GetClientRect(rcClient);

	if ( m_BackDC.GetSafeHdc() == NULL ) {
		m_BackDC.CreateCompatibleDC( &dc );
		//m_BackBitmap.LoadBitmap( IDB_BITMAP_BG_GSENSOR );
		m_BackDC.SelectObject( &m_BackBitmap );
	}

	if ( m_ChartDC.GetSafeHdc() == NULL ) {
		m_ChartDC.CreateCompatibleDC(&dc);
		m_ChartBitmap.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height());
		m_ChartDC.SelectObject( m_ChartBitmap );
	}

	if (m_ChartDC.GetSafeHdc() == NULL)
	{
		m_ChartDC.CreateCompatibleDC(&dc);
		m_MemBitmap.CreateCompatibleBitmap(&dc,rcClient.Width(),rcClient.Height());
		m_ChartDC.SelectObject(m_MemBitmap);
		
		// draw scale
		//m_MemDC.SetBkColor(RGB(0,0,0));
		//CBrush bkBrush(HS_CROSS,RGB(0,0,0));
		//m_MemDC.FillRect(rcClient,&bkBrush);

		// Draw Axis
		//CPen pen(PS_SOLID, 1, RGB(255, 255, 255));
		//CPen* pOldPen = m_MemDC.SelectObject(&pen);
		//m_MemDC.MoveTo(CPoint(rcClient.left, rcClient.top));
		//m_MemDC.LineTo(CPoint(rcClient.left, rcClient.bottom));

		//m_MemDC.MoveTo(CPoint(rcClient.left, rcClient.top + rcClient.bottom / 2));
		//m_MemDC.LineTo(CPoint(rcClient.right, rcClient.top + rcClient.bottom / 2));
	}

	InvalidateRect(rcClient, FALSE);
}

UINT CLineChartCtrl::SetPos(int nIndex, UINT nPos)
{
	if (nIndex >= m_items.GetSize())
		return 0;

	CLineChartItem* pItem = m_items.GetAt(nIndex);

	if (nPos > pItem->m_nUpper)
		nPos = pItem->m_nUpper;
	if (nPos < pItem->m_nLower)
		nPos = pItem->m_nLower;

	pItem->m_nOldPos = pItem->m_nPos;
	pItem->m_nPos = nPos;

	return pItem->m_nOldPos;
}

void CLineChartCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect rcClient;
	GetClientRect(rcClient);

	// draw scale
	if (m_ChartDC.GetSafeHdc() != NULL) {
	
		//dc.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &m_MemDC, 0, 0, SRCCOPY);
		//dc.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &m_BackDC, 0, 0, SRCCOPY);
		dc.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &m_BackDC, 0, 0, SRCCOPY);
		dc.TransparentBlt(19, 0, rcClient.Width() - 40, rcClient.Height(), &m_ChartDC, 0, 0, rcClient.Width(), rcClient.Height(), RGB(0, 0, 0));
	}
}

void CLineChartCtrl::DrawSpike()
{
	CRect rcClient;
	GetClientRect(rcClient);

	if (m_ChartDC.GetSafeHdc() != NULL)
	{
		m_ChartDC.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &m_ChartDC, GRAPH_WIDTH, 0, SRCCOPY);

		// Draw Scale
		CRect rcRight = rcClient;
		rcRight.left = rcRight.right - GRAPH_WIDTH;
		rcRight.left = rcRight.right - GRAPH_WIDTH;
		m_ChartDC.SetBkColor(RGB(0, 0, 0));

		CBrush bkBrush(HS_HORIZONTAL,RGB(0, 0, 0));  
		m_ChartDC.FillRect(rcRight, &bkBrush);

		// Draw Vertical lines only every 2step
		//static BOOL bDrawVerticle = FALSE;
		//bDrawVerticle = !bDrawVerticle;
		//
		//if( bDrawVerticle )
		//{
		//	CPen pen(PS_SOLID, 1, RGB(0, 0, 0));
		//	CPen* pOldPen = m_MemDC.SelectObject(&pen);
		//	m_MemDC.MoveTo(CPoint(rcClient.right - 2, rcClient.top));
		//	m_MemDC.LineTo(CPoint(rcClient.right - 2, rcClient.bottom));
		//	m_MemDC.SelectObject(pOldPen);
		//}

		// Draw Axis
		//CPen pen(PS_SOLID, 1, RGB(255, 255, 255));
		//CPen* pOldPen = m_MemDC.SelectObject(&pen);
		//m_MemDC.MoveTo(CPoint(rcClient.left, rcClient.top));
		//m_MemDC.LineTo(CPoint(rcClient.left, rcClient.bottom));
		//
		//m_MemDC.MoveTo(CPoint(rcClient.right - GRAPH_WIDTH, rcClient.top + rcClient.bottom / 2));
		//m_MemDC.LineTo(CPoint(rcClient.right, rcClient.top + rcClient.bottom / 2));

		int nCount = m_items.GetSize();
		CLineChartItem* pItem;
		CPoint ptOld, ptNew;
		for (int i=0; i<nCount; i++)
		{
			pItem = m_items.GetAt(i);

			UINT  nRange = pItem->m_nUpper - pItem->m_nLower;
			ptOld.x = rcRight.left - 1; // Minus one to make sure to draw inside the area
			ptNew.x = rcRight.right - 1;
			ptOld.y = (int)((((float)(nRange - pItem->m_nOldPos))/(float)nRange)
				* (float)rcRight.Height());
			ptNew.y = (int)((((float)(nRange - pItem->m_nPos))/(float)nRange)
				* (float)rcRight.Height());

			CPen pen(PS_SOLID, 2, pItem->m_colorLine);
			CPen* pOldPen = m_ChartDC.SelectObject(&pen);
			m_ChartDC.MoveTo(ptOld);
			m_ChartDC.LineTo(ptNew);
			
			m_ChartDC.SelectObject(pOldPen);
		}
	}
}

BOOL CLineChartCtrl::Add(COLORREF color, UINT Upper, UINT Lower)
{
	CLineChartItem* pItem = new CLineChartItem;
	pItem->m_colorLine = color;
	pItem->m_nLower = Lower;
	pItem->m_nUpper = Upper;
	pItem->m_nPos = 0;
	pItem->m_nOldPos = 0;

	try {
		m_items.Add(pItem);

		InvalidateCtrl();
		return TRUE;
	}
	catch (CMemoryException* e) {
		if (pItem !=NULL) 
			delete pItem;
		e->Delete();
		return FALSE;
	}
}

void CLineChartCtrl::Go()
{
	DrawSpike();
	Invalidate(FALSE);
}

