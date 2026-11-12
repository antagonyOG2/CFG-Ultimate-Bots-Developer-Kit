#pragma once
#include "SettingsStartSlots.hpp"
#include "OwnedPointerCell.hpp"
namespace cfgmenu::startslots {
template<class Reader> class NativeAccess {
    Reader& reader_;const startbinding::Binding& binding_;HMODULE adapter_=nullptr;bool exactImage_=false;
    bool ImageAddress(std::uintptr_t address)noexcept{
        MEMORY_BASIC_INFORMATION memory{};
        if(VirtualQuery(reinterpret_cast<const void*>(address),&memory,sizeof(memory))!=sizeof(memory)||memory.State!=MEM_COMMIT||
            memory.Type!=MEM_IMAGE||reinterpret_cast<HMODULE>(memory.AllocationBase)!=adapter_||
            (memory.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return false;
        return (memory.Protect&(PAGE_EXECUTE|PAGE_EXECUTE_READ|PAGE_EXECUTE_READWRITE|PAGE_EXECUTE_WRITECOPY))!=0;
    }
public:
    NativeAccess(Reader& reader,const startbinding::Binding& binding,HMODULE adapter,bool exactImageVerified):
        reader_(reader),binding_(binding),adapter_(adapter),exactImage_(exactImageVerified){}
    bool Current(std::uintptr_t wrapper,std::uintptr_t graph){
        if(!exactImage_||binding_.context.module!=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr))||
            GetCurrentThreadId()!=binding_.context.thread)return false;
        std::array<unsigned char,startdisk::ScriptExecutorCode.size()> bytes{};
        return reader_.ReadBytes(binding_.context.module+startdisk::ScriptExecutor,bytes.data(),bytes.size())&&bytes==startdisk::ScriptExecutorCode&&
            startbinding::Current(reader_,binding_,GetCurrentThreadId(),wrapper,graph);
    }
    bool ThunksResident(std::uintptr_t wrapper,std::uintptr_t graph)noexcept{
        if(!exactImage_||!adapter_||!ImageAddress(wrapper)||!ImageAddress(graph))return false;
        HMODULE first=nullptr,second=nullptr;
        return GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,reinterpret_cast<LPCWSTR>(wrapper),&first)&&
            first==adapter_&&GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,reinterpret_cast<LPCWSTR>(graph),&second)&&second==adapter_;
    }
    sandboxstart::WriteResult Exchange(startroute::SlotKind kind,std::uintptr_t old,std::uintptr_t next)noexcept{
        if(!exactImage_||GetCurrentThreadId()!=binding_.context.thread)return sandboxstart::WriteResult::Refused;
        const auto function=kind==startroute::SlotKind::Wrapper?binding_.start.field.object:binding_.graph.field.object;
        return ExchangeOwnedPointer(function+startdisk::Executor,old,next);
    }
};
}
