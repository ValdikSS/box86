#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(XF86VidModeGetViewPort, iFpipp)
GO(XF86VidModeValidateModeLine, iFpip)
#ifdef PANDORA
GOM(XF86VidModeGetGamma, iFpip) //%noE
GOM(XF86VidModeSetGamma, iFpip) //%noE
#else
GO(XF86VidModeGetGamma, iFpip)
GO(XF86VidModeSetGamma, iFpip)
#endif
GO(XF86VidModeSetClientVersion, iFp)
GO(XF86VidModeGetGammaRamp, iFpiippp)
GO(XF86VidModeGetMonitor, iFpip)
GO(XF86VidModeQueryVersion, iFppp)
GO(XF86VidModeDeleteModeLine, iFpip)
GO(XF86VidModeGetModeLine, iFpipp)
GO(XF86VidModeGetAllModeLines, iFpipp)
GO(XF86VidModeSetGammaRamp, iFpiippp)
GO(XF86VidModeGetPermissions, iFpip)
GO(XF86VidModeModModeLine, iFpip)
GO(XF86VidModeSetViewPort, iFpiii)
GO(XF86VidModeSwitchMode, iFpip)
GO(XF86VidModeAddModeLine, iFpipp)
GO(XF86VidModeSwitchToMode, iFpip)
GO(XF86VidModeQueryExtension, iFppp)
GO(XF86VidModeGetGammaRampSize, iFpip)
GO(XF86VidModeGetDotClocks, iFpipppp)
GO(XF86VidModeLockModeSwitch, iFpii)
