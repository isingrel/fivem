/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIPClient.h"
#include "NUIRenderHandler.h"
#include "NUISchemeHandlerFactory.h"
#include "CefOverlay.h"
#include "memdbgon.h"

#include <CoreConsole.h>

NUIPClient::NUIPClient(NUIClient* client)
	: m_owner(client)
{}

auto NUIPClient::OnBeforePopup(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	const CefString& target_url,
	const CefString& target_frame_name,
	CefLifeSpanHandler::WindowOpenDisposition target_disposition,
	bool user_gesture,
	const CefPopupFeatures& popupFeatures,
	CefWindowInfo& windowInfo,
	CefRefPtr<CefClient>& client,
	CefBrowserSettings& settings,
	CefRefPtr<CefDictionaryValue>& extra_info,
	bool* no_javascript_access) -> bool
{
	
	std::string channel = "nui:OUT:OnBeforePopup";
	console::Printf(channel, "%s %s %i %i\n", target_url, target_frame_name, target_disposition, frame->IsMain());
	return true;
}


auto NUIPClient::OnConsoleMessage(CefRefPtr<CefBrowser> browser, cef_log_severity_t level, const CefString& message, const CefString& source, int line) -> bool
{
	std::wstring sourceStr = source.ToWString();
	std::wstring messageStr = message.ToWString();

	// skip websocket error messages and mpMenu messages
	// some of these can't be blocked from script and users get very confused by them appearing in console
	if (messageStr.find(L"WebSocket connection to") != std::string::npos || sourceStr.find(L"nui-game-internal") != std::string::npos || sourceStr.find(L"chrome-devtools") != std::string::npos)
	{
		return false;
	}

	std::string channel = "nui:console";

	if (sourceStr.find(L"nui://") == 0)
	{
		static std::wregex re{ L"nui://(.*?)/(.*)" };
		std::wsmatch match;

		if (std::regex_search(sourceStr, match, re))
		{
			channel = fmt::sprintf("script:%s:nui", ToNarrow(match[1].str()));
			sourceStr = fmt::sprintf(L"@%s/%s", match[1].str(), match[2].str());
		}
	}
	else if (sourceStr.find(L"https://cfx-nui-") == 0)
	{
		static std::wregex re{ L"https://cfx-nui-(.*?)/(.*)" };
		std::wsmatch match;

		if (std::regex_search(sourceStr, match, re))
		{
			channel = fmt::sprintf("script:%s:nui", ToNarrow(match[1].str()));
			sourceStr = fmt::sprintf(L"@%s/%s", match[1].str(), match[2].str());
		}
	}

	console::Printf(channel, "%s\n", ToNarrow(fmt::sprintf(L"%s (%s:%d)", messageStr, sourceStr, line)));

	return false;
}

void NUIPClient::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
	// Set the popup window title
	auto hwnd = browser->GetHost()->GetWindowHandle();
	SetWindowText(hwnd, fmt::sprintf(L"FiveM - %s", title.ToWString()).c_str());
}

CefRefPtr<CefLifeSpanHandler> NUIPClient::GetLifeSpanHandler()
{
	return this;
}

CefRefPtr<CefDisplayHandler> NUIPClient::GetDisplayHandler()
{
	return this;
}

CefRefPtr<CefContextMenuHandler> NUIPClient::GetContextMenuHandler()
{
	return m_owner;
}

CefRefPtr<CefLoadHandler> NUIPClient::GetLoadHandler()
{
	return m_owner;
}

CefRefPtr<CefRequestHandler> NUIPClient::GetRequestHandler()
{
	return m_owner;
}

