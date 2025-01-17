
#pragma once
#include <ul_Include.hpp>
#include <hb/hb_Target.hpp>

constexpr const char PrivateServiceName[] = "ulsf:p";
/* constexpr const char PublicServiceName[] = "ulsf:u"; */

namespace dmi {

    enum class MenuStartMode : u32 {
        Invalid,
        StartupScreen,
        Menu,
        MenuApplicationSuspended,
        MenuLaunchFailure
    };

    enum class MenuMessage : u32 {
        Invalid,
        HomeRequest
    };

    enum class DaemonMessage : u32 {
        Invalid,
        SetSelectedUser,
        LaunchApplication,
        ResumeApplication,
        TerminateApplication,
        LaunchHomebrewLibraryApplet,
        LaunchHomebrewApplication,
        OpenWebPage,
        OpenAlbum,
        RestartMenu
    };

    struct DaemonStatus {
        AccountUid selected_user;
        hb::HbTargetParams params; // Set if homebrew (launched as an application) is suspended
        u64 app_id; // Set if any normal application is suspended
        char fw_version[0x18]; // System version (sent by uDaemon so that it contains Atmosphere/EmuMMC info)
    };

    using CommandFunction = Result(*)(void*, const size_t, const bool);

    struct CommandCommonHeader {
        u32 magic;
        u32 val;
    };

    constexpr u32 CommandMagic = 0x444D4930;
    constexpr size_t CommandStorageSize = 0x800;

    namespace impl {

        using PopStorageFunction = Result(*)(AppletStorage*, const bool);
        using PushStorageFunction = Result(*)(AppletStorage*);

        template<PushStorageFunction PushStorageFn>
        class ScopedStorageWriterBase {
            protected:
                AppletStorage st;
                size_t cur_offset;

            public:
                ScopedStorageWriterBase() : st(), cur_offset(0) {}

                ~ScopedStorageWriterBase() {
                    UL_RC_ASSERT(PushStorage(&this->st));
                    appletStorageClose(&this->st);
                }

                static inline Result PushStorage(AppletStorage *st) {
                    return PushStorageFn(st);
                }

                inline void Initialize(const AppletStorage &st) {
                    this->st = st;
                }

                inline Result PushData(const void *data, const size_t size) {
                    if((cur_offset + size) <= CommandStorageSize) {
                        UL_RC_TRY(appletStorageWrite(&this->st, this->cur_offset, data, size));
                        this->cur_offset += size;
                        return ResultSuccess;
                    }
                    else {
                        return ResultOutOfPushSpace;
                    }
                }

                template<typename T>
                inline Result Push(const T t) {
                    return this->PushData(&t, sizeof(T));
                }
        };

        template<PopStorageFunction PopStorageFn>
        class ScopedStorageReaderBase {
            protected:
                AppletStorage st;
                size_t cur_offset;

            public:
                ScopedStorageReaderBase() : st(), cur_offset(0) {}

                ~ScopedStorageReaderBase() {
                    appletStorageClose(&this->st);
                }

                static inline Result PopStorage(AppletStorage *st, const bool wait) {
                    return PopStorageFn(st, wait);
                }

                inline void Initialize(const AppletStorage &st) {
                    this->st = st;
                }

                inline Result PopData(void *out_data, const size_t size) {
                    if((cur_offset + size) <= CommandStorageSize) {
                        UL_RC_TRY(appletStorageRead(&this->st, this->cur_offset, out_data, size));
                        this->cur_offset += size;
                        return ResultSuccess;
                    }
                    else {
                        return ResultOutOfPopSpace;
                    }
                }
                
                template<typename T>
                inline Result Pop(T &out_t) {
                    return this->PopData(std::addressof(out_t), sizeof(T));
                }
        };

        template<typename StorageReader>
        inline Result OpenStorageReader(StorageReader &reader, const bool wait) {
            AppletStorage st = {};
            UL_RC_TRY(StorageReader::PopStorage(&st, wait));

            reader.Initialize(st);
            return ResultSuccess;
        }

        template<typename StorageWriter>
        inline Result OpenStorageWriter(StorageWriter &writer) {
            AppletStorage st = {};
            UL_RC_TRY(appletCreateStorage(&st, CommandStorageSize));
            
            writer.Initialize(st);
            return ResultSuccess;
        }

        template<typename StorageWriter, typename StorageReader, typename MessageType>
        inline Result SendCommandImpl(const MessageType msg_type, std::function<Result(StorageWriter&)> push_fn, std::function<Result(StorageReader&)> pop_fn) {
            {
                const CommandCommonHeader in_header = {
                    .magic = CommandMagic,
                    .val = static_cast<u32>(msg_type)
                };

                StorageWriter writer;
                UL_RC_TRY(OpenStorageWriter(writer));
                UL_RC_TRY(writer.Push(in_header));

                UL_RC_TRY(push_fn(writer));
            }

            {
                CommandCommonHeader out_header = {};

                StorageReader reader;
                UL_RC_TRY(OpenStorageReader(reader, true));
                UL_RC_TRY(reader.Pop(out_header));
                if(out_header.magic != CommandMagic) {
                    return ResultInvalidOutHeaderMagic;
                }

                UL_RC_TRY(out_header.val);

                UL_RC_TRY(pop_fn(reader));
            }

            return ResultSuccess;
        }

        template<typename StorageWriter, typename StorageReader, typename MessageType>
        inline Result ReceiveCommandImpl(std::function<Result(const MessageType, StorageReader&)> pop_fn, std::function<Result(const MessageType, StorageWriter&)> push_fn) {
            CommandCommonHeader in_out_header = {};
            auto msg_type = MessageType();

            {
                StorageReader reader;
                UL_RC_TRY(OpenStorageReader(reader, false));
                UL_RC_TRY(reader.Pop(in_out_header));
                if(in_out_header.magic != CommandMagic) {
                    return dmi::ResultInvalidInHeaderMagic;
                }

                msg_type = static_cast<MessageType>(in_out_header.val);
                in_out_header.val = pop_fn(msg_type, reader);
            }

            {
                StorageWriter writer;
                UL_RC_TRY(OpenStorageWriter(writer));
                UL_RC_TRY(writer.Push(in_out_header));

                if(R_SUCCEEDED(in_out_header.val)) {
                    UL_RC_TRY(push_fn(msg_type, writer));
                }
            }

            return ResultSuccess;
        }

    }

    namespace dmn {

        Result PopStorage(AppletStorage *st, const bool wait);
        Result PushStorage(AppletStorage *st);

        using DaemonScopedStorageReader = impl::ScopedStorageReaderBase<&PopStorage>;
        using DaemonScopedStorageWriter = impl::ScopedStorageWriterBase<&PushStorage>;

        // Daemon only receives commands from Menu

        inline Result ReceiveCommand(std::function<Result(const DaemonMessage, DaemonScopedStorageReader&)> pop_fn, std::function<Result(const DaemonMessage, DaemonScopedStorageWriter&)> push_fn) {
            return impl::ReceiveCommandImpl(pop_fn, push_fn);
        }

    }

    namespace menu {

        Result PopStorage(AppletStorage *st, const bool wait);
        Result PushStorage(AppletStorage *st);

        using MenuScopedStorageReader = impl::ScopedStorageReaderBase<&PopStorage>;
        using MenuScopedStorageWriter = impl::ScopedStorageWriterBase<&PushStorage>;

        // Menu only sends commands to Daemon

        inline Result SendCommand(const DaemonMessage msg, std::function<Result(MenuScopedStorageWriter&)> push_fn, std::function<Result(MenuScopedStorageReader&)> pop_fn) {
            return impl::SendCommandImpl(msg, push_fn, pop_fn);
        }

    }

}