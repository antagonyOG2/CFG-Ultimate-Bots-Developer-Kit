// Single owner-thread, non-activating Win32 status surface. No UE calls.
#pragma comment(lib,"gdi32.lib")
std::array<wchar_t,160> hostedBadgeText{};
LRESULT CALLBACK HostedBadgeProc(HWND window,UINT message,WPARAM w,LPARAM l){
 if(message==WM_NCHITTEST)return HTTRANSPARENT;
 if(message==WM_ERASEBKGND)return 1;
 if(message==WM_PAINT){PAINTSTRUCT paint;const auto dc=BeginPaint(window,&paint);RECT rect;GetClientRect(window,&rect);
  FillRect(dc,&rect,static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
  const auto font=SelectObject(dc,GetStockObject(DEFAULT_GUI_FONT));SetBkMode(dc,TRANSPARENT);SetTextColor(dc,RGB(225,225,225));
  rect.left+=8;rect.right-=8;DrawTextW(dc,hostedBadgeText.data(),-1,&rect,DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS);
  SelectObject(dc,font);EndPaint(window,&paint);return 0;}
 return DefWindowProcW(window,message,w,l);
}
void ServiceHostedStatus(){
 static HWND badge=nullptr;static std::uint64_t next=0;const auto now=GetTickCount64();if(now<next)return;next=now+250;
 DWORD process=0;const auto foreground=GetForegroundWindow();if(foreground)GetWindowThreadProcessId(foreground,&process);
 Reader r;r.cap=8192;const bool host=process==GetCurrentProcessId()&&cfghostedlobby::InputHost(r,plan.module,observerOwnerThread);
 if(!host){if(badge)ShowWindow(badge,SW_HIDE);return;}
 const auto& s=hostedH1;const wchar_t* status=s.disarmed?L"OFF":s.adoptionPending?L"BINDING LOCAL COUNSELOR":s.adopted?(s.adoptionRefusal?L"DRIVER REFUSED":L"ACTIVE"):s.armed?(s.frameValid&&s.frame.count>=8?L"LOBBY FULL - leave 1 slot":s.frameValid&&s.frame.eligibility==cfghosted::takeover35::Eligibility::HostSelected?L"HOST PICKED - local HUD refresh unavailable":s.frameValid?L"ARMED":L"WAITING FOR MATCH"):cfghostedoptions::WaitingForMatchBadge(cfghostedoptions::Current().values.jason!=0,s.disarmed,s.armed,s.adoptionPending,s.adopted)?L"WAITING FOR MATCH":L"OFF";
 std::array<wchar_t,160> text{};const bool refused=hostedLobbyReport.f3Refused&&hostedLobbyReport.f3Reason==10;
 const bool botsFull=cfghostedlobby::F3RosterFull(hostedLobbyReport);
 swprintf_s(text.data(),text.size(),L"AI JASON: %s%s",status,botsFull?L" | BOTS FULL":
     hostedLobbyReport.f3CapReached?L" | BOTS: 7/7":refused?L" | F3: match only":L"");
 if(!badge){WNDCLASSW cls{};cls.lpfnWndProc=HostedBadgeProc;cls.hInstance=GetModuleHandleW(nullptr);cls.lpszClassName=L"CFG_HostedStatus_24";
  if(!RegisterClassW(&cls)&&GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)return;
  badge=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE|WS_EX_TRANSPARENT|WS_EX_LAYERED,cls.lpszClassName,L"",WS_POPUP,
      0,0,1,1,nullptr,nullptr,cls.hInstance,nullptr);}
 if(!badge)return;
 if(text!=hostedBadgeText){hostedBadgeText=text;InvalidateRect(badge,nullptr,FALSE);}
 SIZE size{};const auto dc=GetDC(badge);const auto font=SelectObject(dc,GetStockObject(DEFAULT_GUI_FONT));
 GetTextExtentPoint32W(dc,text.data(),static_cast<int>(wcslen(text.data())),&size);SelectObject(dc,font);ReleaseDC(badge,dc);
 RECT client;if(!GetClientRect(foreground,&client))return;
 const int width=(std::min)((std::max)(size.cx+16L,120L),(std::max)(120L,client.right-32L));
 const int height=(std::max)(size.cy+8L,22L);POINT origin{(std::max)(8L,client.right-width-16L),12};
 if(!ClientToScreen(foreground,&origin))return;
 SetLayeredWindowAttributes(badge,0,215,LWA_ALPHA);
 SetWindowPos(badge,HWND_TOPMOST,origin.x,origin.y,width,height,SWP_NOACTIVATE|SWP_SHOWWINDOW);
}
