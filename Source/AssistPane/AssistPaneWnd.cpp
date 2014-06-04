#include "AssistPaneWnd.h"
#include "../duictrlex/tinyxml/tinyxml.h"
#include <Shlwapi.h>
#include <direct.h>
//////////////////////////////////////////////////////////////////////////
using namespace DuiLib;
//////////////////////////////////////////////////////////////////////////
AssistPaneWnd::AssistPaneWnd()
{
	m_hParentHWND = NULL;

	LoadConfigFromLocal();
}

AssistPaneWnd::~AssistPaneWnd()
{
	WriteConfigToLocal();
}

DuiLib::CDuiString AssistPaneWnd::GetSkinFolder()
{
	return DuiLib::CDuiString("");
}
DuiLib::CDuiString AssistPaneWnd::GetSkinFile()
{
	return DuiLib::CDuiString("AssistPane/main.xml");
}
LPCTSTR AssistPaneWnd::GetWindowClassName(void) const
{
	return "AssistPaneWndCls";
}

void AssistPaneWnd::AdjustWindowPos()
{
	if(NULL == m_hParentHWND)
	{
		return;
	}

	if(!IsWindow(m_hParentHWND))
	{
		return;
	}

	CDuiRect rcParent;
	GetWindowRect(m_hParentHWND, &rcParent);

	CDuiRect rcPane;
	GetWindowRect(GetHWND(), &rcPane);

	int nPaneWidth = rcPane.GetWidth();
	int nPaneHeight = rcPane.GetHeight();

	rcPane.left = rcParent.right;
	rcPane.right = rcPane.left + nPaneWidth;
	rcPane.top = rcParent.top;
	rcPane.bottom = rcPane.top + nPaneHeight;
	MoveWindow(GetHWND(), rcPane.left, rcPane.top, rcPane.GetWidth(), rcPane.GetHeight(), TRUE);
}

void AssistPaneWnd::Notify(DuiLib::TNotifyUI& msg)
{
	__super::Notify(msg);
}

void AssistPaneWnd::LoadConfigFromLocal()
{
	char szRootPath[MAX_PATH] = {0};
	GetModuleFileName(NULL, szRootPath, sizeof(szRootPath));
	PathRemoveFileSpec(szRootPath);
	strcat(szRootPath, "/assist/local.xml");

	if(!PathFileExists(szRootPath))
	{
		return;
	}

	TiXmlDocument xmldoc;
	if(!xmldoc.LoadFile(szRootPath))
	{
		return;
	}

	TiXmlElement* pRoot = xmldoc.RootElement();
	//	item visible
	TiXmlElement* pItemVisibleRoot = pRoot->FirstChildElement("ItemVisible");
	if(pItemVisibleRoot)
	{
		int nSectionCounter = 0;
		const char* pszSecValue = pItemVisibleRoot->Attribute("counter");

		if(NULL != pszSecValue)
		{
			nSectionCounter = atoi(pszSecValue);
		}

		if(nSectionCounter > 0)
		{
			for(int i = 0; i < nSectionCounter; ++i)
			{
				char szSection[32] = {0};
				sprintf(szSection, "data_%d", i);

				TiXmlElement* pNode = pItemVisibleRoot->FirstChildElement(szSection);

				if(pNode)
				{
					const char* pszName = pNode->Attribute("name");

					if(pszName)
					{
						m_xItemVisibleSet.insert(pszName);
					}
				}
			}
		}
	}

	//	key map
	TiXmlElement* pKeyMapRoot = pRoot->FirstChildElement("KeyMap");
	if(pKeyMapRoot)
	{
		int nSectionCounter = 0;
		const char* pszSecValue = pKeyMapRoot->Attribute("counter");

		if(NULL != pszSecValue)
		{
			nSectionCounter = atoi(pszSecValue);
		}

		if(nSectionCounter > 0)
		{
			for(int i = 0; i < nSectionCounter; ++i)
			{
				char szSection[32] = {0};
				sprintf(szSection, "data_%d", i);

				TiXmlElement* pNode = pKeyMapRoot->FirstChildElement(szSection);

				if(pNode)
				{
					const char* pszToBeMappedKey = pNode->Attribute("tobemappedkey");
					const char* pszMappedKey = pNode->Attribute("mappedkey");

					if(pszToBeMappedKey &&
						pszMappedKey &&
						(0 != strcmp("0", pszToBeMappedKey)) &&
						(0 != strcmp("0", pszMappedKey)))
					{
						m_xKeyMap.insert(std::make_pair(atoi(pszToBeMappedKey), atoi(pszMappedKey)));
					}
				}
			}
		}
	}
}

void AssistPaneWnd::WriteConfigToLocal()
{
	char szRootPath[MAX_PATH] = {0};
	GetModuleFileName(NULL, szRootPath, sizeof(szRootPath));
	PathRemoveFileSpec(szRootPath);
	strcat(szRootPath, "/assist/");

	if(!PathFileExists(szRootPath))
	{
		mkdir(szRootPath);
	}

	strcat(szRootPath, "local.xml");

	TiXmlDocument xmldoc;

	//	root
	TiXmlNode* pRoot = xmldoc.InsertEndChild(TiXmlElement("AssistConfig"));

	//	item visible
	TiXmlElement xItemVisibleNode("ItemVisible");
	xItemVisibleNode.SetAttribute("counter", m_xItemVisibleSet.size());
	TiXmlNode* pItemVisibleRoot = pRoot->InsertEndChild(xItemVisibleNode);
	
	if(!m_xItemVisibleSet.empty())
	{
		StringSet::iterator begIter = m_xItemVisibleSet.begin();
		StringSet::const_iterator endIter = m_xItemVisibleSet.end();
		int nSectionCounter = 0;

		for(begIter;
			begIter != endIter;
			++begIter)
		{
			string& refValue = *begIter;

			char szSection[32] = {0};
			sprintf(szSection, "data_%d", nSectionCounter);
			TiXmlElement xSubNode(szSection);
			xSubNode.SetAttribute("name", refValue.c_str());

			pItemVisibleRoot->InsertEndChild(xSubNode);
			++nSectionCounter;
		}
	}

	//	key map
	TiXmlElement xKeyMapNode("KeyMap");
	xKeyMapNode.SetAttribute("counter", m_xKeyMap.size());
	TiXmlNode* pKeyMapRoot = pRoot->InsertEndChild(xKeyMapNode);

	if(!m_xKeyMap.empty())
	{
		KintVintMap::iterator begIter = m_xKeyMap.begin();
		KintVintMap::const_iterator endIter = m_xKeyMap.end();
		int nSectionCounter = 0;

		for(begIter;
			begIter != endIter;
			begIter++)
		{
			int nToBeMappedKey = begIter->first;
			int nMappedKey = begIter->second;

			if(nToBeMappedKey != 0 &&
				nMappedKey != 0)
			{
				char szKeyBuf[32] = {0};
				char szSection[32] = {0};
				sprintf(szSection, "data_%d", nSectionCounter);

				TiXmlElement xSubNode(szSection);
				xSubNode.SetAttribute("tobemappedkey", itoa(nToBeMappedKey, szKeyBuf, 10));
				xSubNode.SetAttribute("mappedkey", itoa(nMappedKey, szKeyBuf, 10));

				pKeyMapRoot->InsertEndChild(xSubNode);
				++nSectionCounter;
			}
		}
	}

	xmldoc.SaveFile(szRootPath);
}