
#pragma once
#include <ul/loader/loader_TargetTypes.hpp>
#include <ul/menu/smi/smi_MenuProtocol.hpp>

namespace ul::menu::smi {

    inline Result SetSelectedUser(const AccountUid &user_id) {
        return SendCommand(SystemMessage::SetSelectedUser,
            [&](ScopedStorageWriter &writer) {
                writer.Push(user_id);
                return ResultSuccess;
            },
            [](ScopedStorageReader &reader) {
                // ...
                return ResultSuccess;
            }
        );
    }
    
    inline Result LaunchApplication(const u64 app_id) {
        return SendCommand(SystemMessage::LaunchApplication,
            [&](ScopedStorageWriter &writer) {
                writer.Push(app_id);
                return ResultSuccess;
            },
            [](ScopedStorageReader &reader) {
                // ...
                return ResultSuccess;
            }
        );
    }

    inline Result ResumeApplication() {
        return SendCommand(SystemMessage::ResumeApplication,
            [&](ScopedStorageWriter &writer) {
                return ResultSuccess;
            },
            [](ScopedStorageReader &reader) {
                // ...
                return ResultSuccess;
            }
        );
    }

    inline Result TerminateApplication() {
        return SendCommand(SystemMessage::TerminateApplication,
            [&](ScopedStorageWriter &writer) {
                return ResultSuccess;
            },
            [](ScopedStorageReader &reader) {
                // ...
                return ResultSuccess;
            }
        );
    }

    inline Result LaunchHomebrewLibraryApplet(const std::string &nro_path, const std::string &nro_argv) {
        const auto target_ipt = loader::TargetInput::Create(nro_path, nro_argv, false, "");

        return SendCommand(SystemMessage::LaunchHomebrewLibraryApplet,
            [&](ScopedStorageWriter &writer) {
                writer.Push(target_ipt);
                return ResultSuccess;
            },
            [](ScopedStorageReader &reader) {
                // ...
                return ResultSuccess;
            }
        );
    }

    inline Result LaunchHomebrewApplication(const std::string &nro_path, const std::string &nro_argv) {
        const auto target_ipt = loader::TargetInput::Create(nro_path, nro_argv, false, "");

        return SendCommand(SystemMessage::LaunchHomebrewApplication,
            [&](ScopedStorageWriter &writer) {
                writer.Push(target_ipt);
                return ResultSuccess;
            },
            [](ScopedStorageReader &reader) {
                // ...
                return ResultSuccess;
            }
        );
    }

    inline Result ChooseHomebrew() {
        return SendCommand(SystemMessage::ChooseHomebrew,
            [&](ScopedStorageWriter &writer) {
                // ...
                return ResultSuccess;
            },
            [](ScopedStorageReader &reader) {
                // ...
                return ResultSuccess;
            }
        );
    }

    inline Result OpenWebPage(const char(&url)[500]) {
        return SendCommand(SystemMessage::OpenWebPage,
            [&](ScopedStorageWriter &writer) {
                writer.PushData(url, sizeof(url));
                return ResultSuccess;
            },
            [](ScopedStorageReader &reader) {
                // ...
                return ResultSuccess;
            }
        );
    }

    inline Result OpenAlbum() {
        return SendCommand(SystemMessage::OpenAlbum,
            [&](ScopedStorageWriter &writer) {
                return ResultSuccess;
            },
            [](ScopedStorageReader &reader) {
                // ...
                return ResultSuccess;
            }
        );
    }

    inline Result RestartMenu() {
        return SendCommand(SystemMessage::RestartMenu,
            [&](ScopedStorageWriter &writer) {
                return ResultSuccess;
            },
            [](ScopedStorageReader &reader) {
                // ...
                return ResultSuccess;
            }
        );
    }

    inline Result SetHomebrewTakeoverApplication(const u64 app_id) {
        return SendCommand(SystemMessage::SetHomebrewTakeoverApplication,
            [&](ScopedStorageWriter &writer) {
                writer.Push(app_id);
                return ResultSuccess;
            },
            [](ScopedStorageReader &reader) {
                // ...
                return ResultSuccess;
            }
        );
    }

    inline Result UpdateMenuPath(const char (&menu_path)[FS_MAX_PATH]) {
        return SendCommand(SystemMessage::UpdateMenuPath,
            [&](ScopedStorageWriter &writer) {
                writer.PushData(menu_path, sizeof(menu_path));
                return ResultSuccess;
            },
            [](ScopedStorageReader &reader) {
                // ...
                return ResultSuccess;
            }
        );
    }

    inline Result UpdateMenuIndex(const u32 menu_index) {
        return SendCommand(SystemMessage::UpdateMenuIndex,
            [&](ScopedStorageWriter &writer) {
                writer.Push(menu_index);
                return ResultSuccess;
            },
            [](ScopedStorageReader &reader) {
                // ...
                return ResultSuccess;
            }
        );
    }

}