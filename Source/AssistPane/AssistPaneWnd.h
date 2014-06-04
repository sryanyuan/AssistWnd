#ifndef _INC_ASSISTPANEWND_
#define _INC_ASSISTPANEWND_
//////////////////////////////////////////////////////////////////////////
#include <UIlib.h>
#include <set>
#include <string>
#include <map>
//////////////////////////////////////////////////////////////////////////
typedef std::set<string> StringSet;
typedef std::map<int, int> KintVintMap;
//////////////////////////////////////////////////////////////////////////
class AssistPaneWnd : public DuiLib::WindowImplBase
{
public:
	AssistPaneWnd();
	virtual ~AssistPaneWnd();

protected:
	virtual DuiLib::CDuiString GetSkinFolder();
	virtual DuiLib::CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName() const;

	virtual void Notify(DuiLib::TNotifyUI& msg);

public:
	inline void SetParentHWND(HWND _hParent)
	{
		m_hParentHWND = _hParent;
	}

public:
	void AdjustWindowPos();

public:
	//	Game interface
	bool CheckItemVisible(const char* _pszItemName);
	bool CheckItemAlert(const char* _pszItemName);
	bool CheckMonsterAlert(const char* _pszMonsName);

	int CheckMappedKey(int _nKey);

	//	internal implement
	void LoadItemVisibleFromLocal(){}
	void LoadItemAlertFromLoad(){}
	void LoadMonsterAlertFromLocal(){}

	void WriteItemVisibleToLocal(){}
	void WriteItemAlertToLocal(){}
	void WriteMonsterAlertToLocal(){}

	void LoadConfigFromLocal();
	void WriteConfigToLocal();

private:
	void ProcessTabChange(DuiLib::TNotifyUI& msg);

	void ProcessPageOK(DuiLib::TNotifyUI& msg);
	void ApplyItemVisible();

protected:
	HWND m_hParentHWND;

	//	for item visible using
	StringSet m_xItemVisibleSet;
	//	for item alert using
	StringSet m_xItemAlertSet;
	//	for monster alert using
	StringSet m_xMonsterAlertSet;

	//	for key map
	KintVintMap m_xKeyMap;

	//	controls
	DuiLib::CTabLayoutUI* m_pTabLayout;
};
//////////////////////////////////////////////////////////////////////////
#endif