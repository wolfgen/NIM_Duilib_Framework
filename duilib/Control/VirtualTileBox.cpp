#include "stdafx.h"
#include "base/base.h"
#include "VirtualTileBox.h"
#include <atltrace.h>

namespace ui
{

	VirtualTileInterface::VirtualTileInterface() :
		m_CountChangedNotify(),
		m_DataChangedNotify()
	{

	}

	void VirtualTileInterface::RegNotifys(const DataChangedNotify& dcNotify,
										  const CountChangedNotify& ccNotify)
	{
		m_DataChangedNotify = dcNotify;
		m_CountChangedNotify = ccNotify;
	}

	void VirtualTileInterface::EmitDataChanged(int nStartIndex, int nEndIndex)
	{
		if (m_DataChangedNotify) m_DataChangedNotify(nStartIndex, nEndIndex);
	}

	void VirtualTileInterface::EmitCountChanged()
	{
		if (m_CountChangedNotify) m_CountChangedNotify();
	}


	VirtualTileLayout::VirtualTileLayout() :
		m_bAutoCalcColumn(false)
	{
		m_nColumns = 1;
	}

	ui::CSize
		VirtualTileLayout::ArrangeChild(const std::vector<ui::Control*>& items,
										ui::UiRect rc)
	{
		//assert(nbase::ThreadManager::CurrentThread() != nullptr);

		ui::CSize sz(rc.GetWidth(), rc.GetHeight());

		VirtualTileBox* pList = dynamic_cast<VirtualTileBox*>(m_pOwner);
		ASSERT(pList);


		int nTotalHeight = GetElementsHeight(-1);
		sz.cy = max(nTotalHeight, sz.cy);
		LazyArrangeChild();
		return sz;
	}

	ui::CSize
		VirtualTileLayout::AdjustSizeByChild(const std::vector<ui::Control*>& items,
											ui::CSize szAvailable)
	{
		//assert(nbase::ThreadManager::CurrentThread() != nullptr);

		VirtualTileBox* pList = dynamic_cast<VirtualTileBox*>(m_pOwner);
		ASSERT(pList);

		ui::CSize size = m_pOwner->Control::EstimateSize(szAvailable);
		if (size.cx == DUI_LENGTH_AUTO || size.cx == 0)
		{
			size.cx = m_szItem.cx * m_nColumns + m_iChildMargin * (m_nColumns - 1);
		}
		return size;
	}


	bool
		VirtualTileLayout::SetAttribute(const std::wstring& strName,
										const std::wstring& strValue)
	{
		if (strName == L"column")
		{
			int iValue = _ttoi(strValue.c_str());
			if (iValue > 0)
			{
				SetColumns(iValue);
				m_bAutoCalcColumn = false;
			}
			else {
				m_bAutoCalcColumn = true;
			}
			return true;
		}
		else {
			return __super::SetAttribute(strName, strValue);
		}
	}

	int
		VirtualTileLayout::GetElementsHeight(int nCount)
	{
		//assert(nbase::ThreadManager::CurrentThread() != nullptr);

		if (nCount < m_nColumns && nCount != -1) return m_szItem.cy + m_iChildMargin;

		VirtualTileBox* pList = dynamic_cast<VirtualTileBox*>(m_pOwner);
		ASSERT(pList);

		if (nCount < 0)
			nCount = pList->GetElementCount();

		int rows = nCount / m_nColumns;
		if (nCount % m_nColumns != 0) {
			rows += 1;
		}
		if (nCount > 0) {
			int childMarginTotal;
			if (nCount % m_nColumns == 0) {
				childMarginTotal = (nCount / m_nColumns - 1) * m_iChildMargin;
			}
			else {
				childMarginTotal = (nCount / m_nColumns) * m_iChildMargin;
			}

			return m_szItem.cy * (rows + 1) + childMarginTotal;
		}
		return 0;
	}

	void
		VirtualTileLayout::LazyArrangeChild()
	{
		//assert(nbase::ThreadManager::CurrentThread() != nullptr);

		VirtualTileBox* pList = dynamic_cast<VirtualTileBox*>(m_pOwner);

		ASSERT(pList);
		ASSERT(m_nColumns);


		ui::UiRect rc = pList->GetPaddingPos();

		int iPosLeft = rc.left;

		int iPosTop = rc.top + pList->GetScrollPos().cy / GetElementsHeight(0) * GetElementsHeight(0);

		ui::CPoint ptTile(iPosLeft, iPosTop);

		int nTopBottom = 0;
		int nTopIndex = pList->GetTopElementIndex(nTopBottom);

		int iCount = 0, _iSelectedItemIndex = -2;

		pList->SelectItem(-2, false, false);

		for (auto pControl : pList->m_items)
		{
			ui::UiRect rcTile(ptTile.x, ptTile.y, ptTile.x + m_szItem.cx, ptTile.y + m_szItem.cy);
			pControl->SetPos(rcTile);

			int nElementIndex = nTopIndex + iCount;
			if (nElementIndex < pList->GetElementCount())
			{
				if (!pControl->IsVisible()) pControl->SetVisible(true);
/*
				nbase::ThreadManager::PostTask(0, [pList, pControl, nElementIndex]()
											  {*/
												   pList->FillElement(pControl, nElementIndex);
											 // });
				//
			}
			else {
				if (pControl->IsVisible()) pControl->SetVisible(false);
			}

			if (pList->GetSelectedElementIndex() == nElementIndex)
				_iSelectedItemIndex = iCount;
				
			if ((++iCount % m_nColumns) == 0) {
				ptTile.x = iPosLeft;
				ptTile.y += m_szItem.cy + m_iChildMargin;
			}
			else {
				ptTile.x += rcTile.GetWidth() + m_iChildMargin;
			}
		}

		pList->SelectItem(_iSelectedItemIndex);
	}

	int
		VirtualTileLayout::AdjustMaxItem()
	{
		//assert(nbase::ThreadManager::CurrentThread() != nullptr);

		ui::UiRect rc = m_pOwner->GetPaddingPos();

		if (m_bAutoCalcColumn)
		{
			if (m_szItem.cx > 0) m_nColumns = (rc.right - rc.left) / (m_szItem.cx + m_iChildMargin / 2);
			if (m_nColumns == 0) m_nColumns = 1;
		}

		int nHeight = m_szItem.cy + m_iChildMargin;
		int nRow = (rc.bottom - rc.top) / nHeight + 1;
		return nRow * m_nColumns;
	}

	VirtualTileBox::VirtualTileBox(ui::Layout* pLayout /*= new VirtualTileLayout*/)
		: ui::ListBox(pLayout)
		, m_pDataProvider(nullptr)
		, m_nMaxItemCount(0)
		, m_nOldYScrollPos(0)
		, m_bArrangedOnce(false)
		, m_bForceArrange(false)
	{


	}

	VirtualTileBox::~VirtualTileBox()
	{

	}

	void
		VirtualTileBox::SetDataProvider(VirtualTileInterface* pProvider)
	{
		ASSERT(pProvider);
		m_pDataProvider = pProvider;

		pProvider->RegNotifys(
			nbase::Bind(&VirtualTileBox::OnModelDataChanged, this, std::placeholders::_1, std::placeholders::_2),
			nbase::Bind(&VirtualTileBox::OnModelCountChanged, this));
	}


	VirtualTileInterface* VirtualTileBox::GetDataProvider()
	{
		return m_pDataProvider;
	}

	void
		VirtualTileBox::Refresh()
	{
		//assert(nbase::ThreadManager::CurrentThread() != nullptr);

		m_nMaxItemCount = GetTileLayout()->AdjustMaxItem();

		int nElementCount = GetElementCount();
		int nItemCount = GetCount();

		if (nItemCount > nElementCount)
		{

			int n = nItemCount - nElementCount;
			for (int i = 0; i < n; i++)
				this->RemoveAt(0);
		}

		else if (nItemCount < nElementCount) {
			int n = 0;
			if (nElementCount <= m_nMaxItemCount)
			{
				n = nElementCount - nItemCount;
			}
			else {
				n = m_nMaxItemCount - nItemCount;
			}

			for (int i = 0; i < n; i++) {
				Control* pControl = CreateElement();
				this->Add(pControl);
			}
		}

		if (nElementCount <= 0)
			return;

		ReArrangeChild(false);
		Arrange();

	}

	void
		VirtualTileBox::RemoveAll()
	{
		//assert(nbase::ThreadManager::CurrentThread() != nullptr);

		__super::RemoveAll();

		if (m_pVerticalScrollBar)
			m_pVerticalScrollBar->SetScrollPos(0);

		m_nOldYScrollPos = 0;
		m_bArrangedOnce = false;
		m_bForceArrange = false;
	}

	void
		VirtualTileBox::SetForceArrange(bool bForce)
	{
		m_bForceArrange = bForce;
	}

	void
		VirtualTileBox::GetDisplayCollection(std::vector<int>& collection)
	{
		//assert(nbase::ThreadManager::CurrentThread() != nullptr);

		collection.clear();

		if (GetCount() == 0)
			return;

		ui::UiRect rcThis = this->GetPaddingPos();

		int nEleHeight = GetRealElementHeight();

		int min = (GetScrollPos().cy / nEleHeight) * GetColumns();
		int max = min + (rcThis.GetHeight() / nEleHeight) * GetColumns();

		int nCount = GetElementCount();
		if (max >= nCount)
			max = nCount - 1;

		for (auto i = min; i <= max; i++)
			collection.push_back(i);
	}

	void
		VirtualTileBox::EnsureVisible(int iIndex, bool bToTop /*= false*/)
	{
		//assert(nbase::ThreadManager::CurrentThread() != nullptr);

		if (iIndex < 0 || iIndex >= GetElementCount())
			return;

		if (!m_pVerticalScrollBar)
			return;

		auto nPos = GetScrollPos().cy;
		int nTopIndex = (nPos / GetRealElementHeight()) * GetColumns();
		int nNewPos = 0;

		if (bToTop)
		{
			nNewPos = CalcElementsHeight(iIndex);
			if (nNewPos >= m_pVerticalScrollBar->GetScrollRange())
				return;
		}
		else {
			if (IsElementDisplay(iIndex))
				return;

			if (iIndex > nTopIndex)
			{
				int height = CalcElementsHeight(iIndex + 1);
				nNewPos = height - m_rcItem.GetHeight();
			}
			else
			{
				nNewPos = CalcElementsHeight(iIndex);
			}
		}
		ui::CSize sz(0, nNewPos);
		SetScrollPos(sz);
	}

	void
		VirtualTileBox::SetScrollPos(ui::CSize szPos)
	{	
		//assert(nbase::ThreadManager::CurrentThread() != nullptr);

		m_nOldYScrollPos = GetScrollPos().cy;
		ListBox::SetScrollPos(szPos);

		ReArrangeChild(false);
	}

	void
		VirtualTileBox::HandleMessage(ui::EventArgs& event)
	{
		if (!IsMouseEnabled() && event.Type > ui::kEventMouseBegin && event.Type < ui::kEventMouseEnd) {
			if (m_pParent != nullptr)
				m_pParent->HandleMessageTemplate(event);
			else
				ui::ScrollableBox::HandleMessage(event);
			return;
		}

		switch (event.Type) {
		case ui::kEventKeyDown: {
			switch (event.chKey) {
			case VK_UP: {
				OnKeyDown(VK_UP);
				return;
			}
			case VK_DOWN: {
				OnKeyDown(VK_DOWN);
				return;
			}
			case VK_HOME:
				SetScrollPosY(0);
				return;
			case VK_END: {
				int range = GetScrollPos().cy;
				SetScrollPosY(range);
				return;
			}
			}
		}
		case ui::kEventKeyUp: {
			switch (event.chKey) {
			case VK_UP: {
				OnKeyUp(VK_UP);
				return;
			}
			case VK_DOWN: {
				OnKeyUp(VK_DOWN);
				return;
			}
			}
		}
		}

		__super::HandleMessage(event);
	}

	void
		VirtualTileBox::SetPos(ui::UiRect rc)
	{
		//assert(nbase::ThreadManager::CurrentThread() != nullptr);

		bool bChange = false;
		if (!m_rcItem.Equal(rc))
			bChange = true;

		ListBox::SetPos(rc);

		if (bChange) {

			//Refresh();


			ULONGLONG ullTimeStamp = static_cast<ULONGLONG>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

			if (m_bRefreshPending.load() == false)
			{
				ATLTRACE(L"VirtualTileBox::SetPos() -- Refresh called at %lld\n", ullTimeStamp);
				m_bRefreshPending.store(true);
				::PostMessage(this->GetWindow()->GetHWND(), NIMDUI_WM_VIRTUALBOX_REFRESH, (WPARAM)this, 0);
				//Arrange();			
			}
			else
			{
				//ATLTRACE(L"VirtualTileBox::SetPos() --  Refresh pending at %lld\n", ullTimeStamp);
			}
		}
	}

	bool 
		VirtualTileBox::SelectItem(int iIndex, 
								   bool bTakeFocus /*= false*/, 
								   bool bTrigger /*= true*/)
	{
		//assert(nbase::ThreadManager::CurrentThread() != nullptr);

		if (iIndex != -2)
			m_nSelectedElementIndex = ItemIndexToElementIndex(iIndex);

		return __super::SelectItem(iIndex, bTakeFocus, bTrigger);
	}

	void
		VirtualTileBox::ReArrangeChild(bool bForce)
	{
		//assert(nbase::ThreadManager::CurrentThread() != nullptr);

		ScrollDirection direction = kScrollUp;
		if (!bForce && !m_bForceArrange) {
			if (!NeedReArrange(direction))
				return;
		}

		LazyArrangeChild();
	}


	ui::Control*
		VirtualTileBox::CreateElement()
	{
		if (m_pDataProvider)
			return m_pDataProvider->CreateElement();

		return nullptr;
	}

	void
		VirtualTileBox::FillElement(Control* pControl, int iIndex)
	{
		if (m_pDataProvider)
			m_pDataProvider->FillElement(pControl, iIndex);
	}

	int
		VirtualTileBox::GetElementCount()
	{
		if (m_pDataProvider)
			return m_pDataProvider->GetElementCount();

		return 0;
	}

	int
		VirtualTileBox::CalcElementsHeight(int nCount)
	{
		return GetTileLayout()->GetElementsHeight(nCount);
	}

	int
		VirtualTileBox::GetTopElementIndex(int& bottom)
	{
		int nPos = GetScrollPos().cy;

		int nHeight = GetRealElementHeight();
		int iIndex = (nPos / nHeight) * GetColumns();
		bottom = iIndex * nHeight;

		return iIndex;
	}

	bool
		VirtualTileBox::IsElementDisplay(int iIndex)
	{
		if (iIndex < 0)
			return false;

		int nPos = GetScrollPos().cy;
		int nElementPos = CalcElementsHeight(iIndex);
		if (nElementPos >= nPos) {
			int nHeight = this->GetHeight();
			if (nElementPos - GetRealElementHeight() <= nPos + nHeight)
				return true;
		}

		return false;
	}

	bool
		VirtualTileBox::NeedReArrange(ScrollDirection& direction)
	{
		direction = kScrollUp;
		if (!m_bArrangedOnce) {
			m_bArrangedOnce = true;
			return true;
		}

		int nCount = GetCount();
		if (nCount <= 0)
			return false;

		if (GetElementCount() <= nCount)
			return false;


		ui::UiRect rcThis = this->GetPos();
		if (rcThis.GetWidth() <= 0)
			return false;

		int nPos = GetScrollPos().cy;
		ui::UiRect rcItem;

		rcItem = m_items[0]->GetPos();

		if (nPos >= m_nOldYScrollPos)
		{
			rcItem = m_items[nCount - 1]->GetPos();
			int nSub = (rcItem.bottom - rcThis.top) - (nPos + rcThis.GetHeight());
			if (nSub < 0) {
				direction = kScrollDown;
				return true;
			}
		}
		else
		{
			rcItem = m_items[0]->GetPos();
			if (nPos < (rcItem.top - rcThis.top)) {
				direction = kScrollUp;
				return true;
			}
		}

		return false;

	}

	VirtualTileLayout*
		VirtualTileBox::GetTileLayout()
	{
		auto* pLayout = dynamic_cast<VirtualTileLayout*>(m_pLayout.get());
		return pLayout;
	}

	int
		VirtualTileBox::GetRealElementHeight()
	{
		return GetTileLayout()->GetElementsHeight(0);
	}

	int
		VirtualTileBox::GetColumns()
	{
		return GetTileLayout()->GetColumns();
	}

	void
		VirtualTileBox::LazyArrangeChild()
	{

		GetTileLayout()->LazyArrangeChild();
	}

	void
		VirtualTileBox::OnModelDataChanged(int nStartIndex, int nEndIndex)
	{
		for (auto i = nStartIndex; i <= nEndIndex; i++)
		{
			int nItemIndex = ElementIndexToItemIndex(nStartIndex);

			if (nItemIndex >= 0 && nItemIndex < m_items.size()) {
				FillElement(m_items[nItemIndex], i);
			}
		}
	}

	void
		VirtualTileBox::OnModelCountChanged()
	{

		ULONGLONG ullTimeStamp = static_cast<ULONGLONG>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

		if (m_bRefreshPending.load() == false)
		{
			ATLTRACE(L"Refresh called at %lld\n", ullTimeStamp);
			m_bRefreshPending.store(true);
			::PostMessage(this->GetWindow()->GetHWND(), NIMDUI_WM_VIRTUALBOX_REFRESH, (WPARAM)this, 0);
			//Arrange();			
		}
		else
		{
			//ATLTRACE(L"Refresh pending at %lld\n", ullTimeStamp);
		}

		//Refresh();
	}

	int
		VirtualTileBox::ElementIndexToItemIndex(int nElementIndex)
	{
		if (IsElementDisplay(nElementIndex))
		{
			int nTopItemHeight = 0;
			return nElementIndex - GetTopElementIndex(nTopItemHeight);
		}
		return -1;
	}

	int
		VirtualTileBox::ItemIndexToElementIndex(int nItemIndex)
	{
		int nTopItemHeight = 0;
		return GetTopElementIndex(nTopItemHeight) + nItemIndex;
	}

}