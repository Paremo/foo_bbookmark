#pragma once

#include "libPPUI/CListControlComplete.h"

class CListControlBookmark;

class IListControlBookmarkSource {
public:
	typedef const CListControlBookmark * ctx_t;

	virtual size_t listGetItemCount(ctx_t) = 0;
	virtual pfc::string8 listGetSubItemText(ctx_t, size_t item, size_t subItem) = 0;
	virtual bool listCanReorderItems(ctx_t) { return false; }
	virtual bool listReorderItems(ctx_t, const size_t*, size_t) { return false; }
	virtual bool listRemoveItems(ctx_t, pfc::bit_array const &) { return false; }
	virtual void listItemAction(ctx_t, size_t) {}
	virtual void listSubItemClicked(ctx_t, size_t, size_t) {}
	virtual bool listCanSelectItem(ctx_t, size_t) { return true; }
	virtual pfc::string8 listGetEditField(ctx_t ctx, size_t item, size_t subItem, size_t & lineCount) {
		lineCount = 1; return listGetSubItemText(ctx, item, subItem);
	}
	virtual void listSetEditField(ctx_t ctx, size_t item, size_t subItem, const char * val) {}
	virtual uint32_t listGetEditFlags(ctx_t ctx, size_t item, size_t subItem) { return 0; }
	virtual bool listIsColumnEditable(ctx_t, size_t) { return false; }
	virtual bool listKeyDown(ctx_t, UINT nChar, UINT nRepCnt, UINT nFlags) { return false; }
	virtual bool listKeyUp(ctx_t, UINT nChar, UINT nRepCnt, UINT nFlags) { return false; }
	virtual bool listContextMenu(ctx_t, CWindow wnd, CPoint point) { return false; }

	// Allow type-find in this view?
	// Called prior to a typefind pass attempt, you can either deny entirely, or prepare any necessary data and allow it.
	virtual bool listAllowTypeFind(ctx_t) { return false; }
	// Allow type-find in a specific item/column?
	virtual bool listAllowTypeFindHere(ctx_t, size_t item, size_t subItem) { return false; }

	virtual void listColumnHeaderClick(ctx_t, size_t subItem) {}

	virtual void listBeforeDrawItemText(ctx_t, size_t item, CDCHandle dc) {}

	virtual void listSelChanged(ctx_t) {}
	virtual void listFocusChanged(ctx_t) {}

	virtual void listColumnsChanged(ctx_t) {}

	virtual bool listEditCanAdvanceHere(ctx_t, size_t item, size_t subItem, uint32_t whatHappened) { (void)item; (void)subItem, (void)whatHappened; return true; }
};

class IListControlBookmarkCells {
public:
	typedef const CListControlBookmark * cellsCtx_t;
	virtual CListControl::cellType_t listCellType(cellsCtx_t, size_t item, size_t subItem) = 0;
	virtual size_t listCellSpan(cellsCtx_t, size_t item, size_t subItem) { return 1; }
	virtual bool listCellCheckState(cellsCtx_t, size_t item, size_t subItem) { return false; }
	virtual void listCellSetCheckState(cellsCtx_t, size_t item, size_t subItem, bool state) {}
};

class CListControlBookmark : public CListControlComplete {	//TODO: use readonly clist instead
	IListControlBookmarkSource * const m_host;
public:

	CListControlBookmark(IListControlBookmarkSource * h) : m_host(h) {}

	BEGIN_MSG_MAP_EX(CListControlBookmark)
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_KEYUP(OnKeyUp)
		MSG_WM_SYSKEYDOWN(OnKeyDown)
		MSG_WM_SYSKEYUP(OnKeyUp)
		CHAIN_MSG_MAP(CListControlComplete)
	END_MSG_MAP()

	bool CanSelectItem(size_t idx) const override {
		return m_host->listCanSelectItem(this, idx);
	}
	size_t GetItemCount() const override {
		return m_host->listGetItemCount(this);
	}
	bool GetSubItemText(size_t item, size_t subItem, pfc::string_base & out) const override {
		out = m_host->listGetSubItemText(this, item, subItem);
		return true;
	}
	void OnSubItemClicked(size_t item, size_t subItem, CPoint pt) override {
		__super::OnSubItemClicked(item, subItem, pt); // needed to toggle checkboxes etc
		m_host->listSubItemClicked(this, item, subItem);
	}

	uint32_t QueryDragDropTypes() const override {
		return (m_host->listCanReorderItems(this)) ? dragDrop_reorder : 0;
	}

	void RequestReorder(const size_t * order, size_t count) override {
		if (!m_host->listReorderItems(this, order, count)) return;
		this->OnItemsReordered(order, count);
	}
	void RequestRemoveSelection() override {
		auto mask = this->GetSelectionMask();
		size_t oldCount = GetItemCount();
		if (!m_host->listRemoveItems(this, mask)) return;
		this->OnItemsRemoved(mask, oldCount);
	}
	void ExecuteDefaultAction(size_t idx) override {
		m_host->listItemAction(this, idx);
	}
	size_t EvalTypeFind() override {
		if (!m_host->listAllowTypeFind(this)) return SIZE_MAX;
		return __super::EvalTypeFind();
	}
	bool AllowTypeFindInCell(size_t item, size_t subItem) const {
		return __super::AllowTypeFindInCell(item, subItem) && m_host->listAllowTypeFindHere(this, item, subItem);
	}

protected:
	void OnFocusChanged(size_t o, size_t f) override {
		__super::OnFocusChanged(o, f);
		m_host->listFocusChanged(this);
	}
	void OnSelectionChanged(pfc::bit_array const & affected, pfc::bit_array const & status) {
		__super::OnSelectionChanged(affected, status);
		m_host->listSelChanged(this);
	}

	void RenderItemText(t_size p_item, const CRect & p_itemRect, const CRect & p_updateRect, CDCHandle p_dc, bool allowColors) {
		m_host->listBeforeDrawItemText(this, p_item, p_dc);
		__super::RenderItemText(p_item, p_itemRect, p_updateRect, p_dc, allowColors);
	}
	void TableEdit_SetField(t_size item, t_size subItem, const char * value) override {
		m_host->listSetEditField(this, item, subItem, value);
	}
	void TableEdit_GetField(t_size item, t_size subItem, pfc::string_base & out, t_size & lineCount) override {
		lineCount = 1;
		out = m_host->listGetEditField(this, item, subItem, lineCount);
	}

	t_uint32 TableEdit_GetEditFlags(t_size item, t_size subItem) const override {
		return m_host->listGetEditFlags(this, item, subItem);
	}
	bool TableEdit_IsColumnEditable(t_size subItem) const override {
		return m_host->listIsColumnEditable(this, subItem);
	}
	void OnColumnHeaderClick(t_size index) override {
		m_host->listColumnHeaderClick(this, index);
	}
	void OnColumnsChanged() override {
		__super::OnColumnsChanged();
		m_host->listColumnsChanged(this);
	}
private:
	bool TableEdit_CanAdvanceHere(size_t item, size_t subItem, uint32_t whatHappened) const override {
		return m_host->listEditCanAdvanceHere(this, item, subItem, whatHappened);
	}
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
		bool handled = m_host->listKeyDown(this, nChar, nRepCnt, nFlags);
		SetMsgHandled(!!handled);
	}
	void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
		bool handled = m_host->listKeyUp(this, nChar, nRepCnt, nFlags);
		SetMsgHandled(!!handled);
	}
};

class CListControlBookmarkCells : public CListControlBookmark {
	IListControlBookmarkCells * const m_cells;
public:
	CListControlBookmarkCells(IListControlBookmarkSource * source, IListControlBookmarkCells * cells) : CListControlBookmark(source), m_cells(cells) {}

	bool GetCellTypeSupported() const override { return true; }
	bool GetCellCheckState(size_t item, size_t subItem) const override {
		return m_cells->listCellCheckState(this, item, subItem);
	}
	void SetCellCheckState(size_t item, size_t subItem, bool value) override {
		m_cells->listCellSetCheckState(this, item, subItem, value);
	}
	cellType_t GetCellType(size_t item, size_t subItem) const override {
		return m_cells->listCellType(this, item, subItem);
	}
	size_t GetSubItemSpan(size_t row, size_t column) const override {
		return m_cells->listCellSpan(this, row, column);
	}
};