#include <ul/system/la/la_LibraryApplet.hpp>
#include <ul/ul_Result.hpp>

namespace ul::system::la {

    namespace {

        AppletHolder g_AppletHolder;
        AppletId g_MenuAppletId = AppletId_None;
        AppletId g_LastAppletId = AppletId_None;

        struct AppletInfo {
            u64 program_id;
            AppletId applet_id;
        };

        constexpr AppletInfo g_AppletTable[] = {
            { 0x0100000000001001, AppletId_LibraryAppletAuth },
            { 0x0100000000001002, AppletId_LibraryAppletCabinet },
            { 0x0100000000001003, AppletId_LibraryAppletController },
            { 0x0100000000001004, AppletId_LibraryAppletDataErase },
            { 0x0100000000001005, AppletId_LibraryAppletError },
            { 0x0100000000001006, AppletId_LibraryAppletNetConnect },
            { 0x0100000000001007, AppletId_LibraryAppletPlayerSelect },
            { 0x0100000000001008, AppletId_LibraryAppletSwkbd },
            { 0x0100000000001009, AppletId_LibraryAppletMiiEdit },
            { 0x010000000000100A, AppletId_LibraryAppletWeb },
            { 0x010000000000100B, AppletId_LibraryAppletShop },
            { 0x010000000000100D, AppletId_LibraryAppletPhotoViewer },
            { 0x010000000000100E, AppletId_LibraryAppletSet },
            { 0x010000000000100F, AppletId_LibraryAppletOfflineWeb },
            { 0x0100000000001010, AppletId_LibraryAppletLoginShare },
            { 0x0100000000001011, AppletId_LibraryAppletWifiWebAuth },
            { 0x0100000000001013, AppletId_LibraryAppletMyPage }
        };
        constexpr size_t AppletCount = sizeof(g_AppletTable) / sizeof(AppletInfo);

    }

    bool IsActive() {
        if(g_AppletHolder.StateChangedEvent.revent == INVALID_HANDLE) {
            return false;
        }
        if(!serviceIsActive(&g_AppletHolder.s)) {
            return false;
        }
        return !appletHolderCheckFinished(&g_AppletHolder);
    }

    Result Terminate() {
        // Give it 15 seconds
        return appletHolderRequestExitOrTerminate(&g_AppletHolder, 15'000'000'000ul);
    }

    Result Start(const AppletId id, const u32 la_version, const void *in_data, const size_t in_size) {
        if(IsActive()) {
            Terminate();
        }
        appletHolderClose(&g_AppletHolder);
    
        UL_RC_TRY(appletCreateLibraryApplet(&g_AppletHolder, id, LibAppletMode_AllForeground));

        LibAppletArgs la_args;
        libappletArgsCreate(&la_args, la_version);
        UL_RC_TRY(libappletArgsPush(&la_args, &g_AppletHolder));

        if(in_size > 0) {
            UL_RC_TRY(Send(in_data, in_size));
        }

        UL_RC_TRY(appletHolderStart(&g_AppletHolder));
        g_LastAppletId = id;
        return ResultSuccess;
    }

    Result Send(const void *data, const size_t size) {
        return libappletPushInData(&g_AppletHolder, data, size);
    }

    Result Read(void *data, const size_t size) {
        return libappletPopOutData(&g_AppletHolder, data, size, nullptr);
    }

    Result Push(AppletStorage *st) {
        return appletHolderPushInData(&g_AppletHolder, st);
    }

    Result Pop(AppletStorage *st) {
        return appletHolderPopOutData(&g_AppletHolder, st);
    }

    u64 GetProgramIdForAppletId(const AppletId id) {
        for(u32 i = 0; i < AppletCount; i++) {
            const auto info = g_AppletTable[i];
            if(info.applet_id == id) {
                return info.program_id;
            }
        }

        return 0;
    }

    AppletId GetAppletIdForProgramId(const u64 id) {
        for(u32 i = 0; i < AppletCount; i++) {
            const auto info = g_AppletTable[i];
            if(info.program_id == id) {
                return info.applet_id;
            }
        }

        return AppletId_None;
    }

    AppletId GetLastAppletId() {
        auto last_id_copy = g_LastAppletId;
        if(!IsActive()) {
            g_LastAppletId = AppletId_None;
        }
        return last_id_copy;
    }

    bool IsMenu() {
        return IsActive() && (g_MenuAppletId != AppletId_None) && (GetLastAppletId() == g_MenuAppletId);
    }

    void SetMenuAppletId(const AppletId id) {
        g_MenuAppletId = id;
    }

    AppletId GetMenuAppletId() {
        return g_MenuAppletId;
    }

}