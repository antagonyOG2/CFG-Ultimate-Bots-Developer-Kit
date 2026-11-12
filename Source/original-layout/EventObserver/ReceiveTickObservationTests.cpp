#include "ReceiveTickObservation.hpp"
#define main ExistingAiFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
struct TickFixture:Fixture {
    std::uintptr_t function=0;cfgnames::Words name{73,0};
    TickFixture(){
        function=Object();cap=512;
        Put(module+cfgtick::disk::Wrapper,cfgtick::disk::WrapperCode);
        Put(module+cfgtick::disk::Name,name);Put(function+cfgnames::disk::ObjectName,name);
        Put(function+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{4});
        Put(module+cfgprobe::lifetime::ObjectArrayRva+cfgprobe::lifetime::ArrayDataOffset,objects.data);
        Put(module+cfgprobe::lifetime::ObjectArrayRva+cfgprobe::lifetime::ArrayCountOffset,objects.count);
        for(int i=0;i<nextSlot;i++)Put(objects.data+static_cast<std::uintptr_t>(i)*24+16,std::int32_t{i+1});
        // A dynamic Blueprint function intentionally differs from base cache.
        Put(module+cfgcompat::cfg35::functions::ActorReceiveTick.cacheRva,function+0x4000);
    }
    bool Observe(std::uintptr_t pc=0,cfgtick::Failure* failure=nullptr){return cfgtick::Observe(*this,module,pawn,function,pc?pc:module+cfgtick::disk::ReturnPc,failure);}
    void CopyKnownSlots(std::uintptr_t destination){
        for(const auto object:{pawn,function}){
            std::int32_t index=0,serial=0;std::uint32_t flags=0;std::uintptr_t value=0;
            CHECK(Read(object+cfgprobe::lifetime::ObjectIndexOffset,index));
            const auto source=objects.data+static_cast<std::uintptr_t>(index)*cfgprobe::lifetime::ItemBytes;
            const auto target=destination+static_cast<std::uintptr_t>(index)*cfgprobe::lifetime::ItemBytes;
            CHECK(Read(source+cfgprobe::lifetime::ItemObjectOffset,value));Put(target+cfgprobe::lifetime::ItemObjectOffset,value);
            CHECK(Read(source+8,flags));Put(target+8,flags);
            CHECK(Read(source+cfgprobe::lifetime::ItemSerialOffset,serial));Put(target+cfgprobe::lifetime::ItemSerialOffset,serial);
        }
    }
};
int main(){
    CHECK(cfgtick::CompletedWorkCandidate(true,false,true,true,false));
    CHECK(!cfgtick::CompletedWorkCandidate(false,false,true,true,false));
    CHECK(!cfgtick::CompletedWorkCandidate(true,true,true,true,false));
    CHECK(!cfgtick::CompletedWorkCandidate(true,false,false,true,false));
    CHECK(!cfgtick::CompletedWorkCandidate(true,false,true,false,false));
    CHECK(!cfgtick::CompletedWorkCandidate(true,false,true,true,true));
    {TickFixture f;cfgtick::Failure e{};CHECK(f.Observe(0,&e)&&e==cfgtick::Failure::None);CHECK(f.reads<100);}
    {TickFixture f;cfgtick::Failure e{};CHECK(!f.Observe(f.module+cfgtick::disk::ReturnPc+1,&e)&&!f.reads&&e==cfgtick::Failure::Identity);}
    {TickFixture f;cfgtick::Failure e{};f.Put(f.module+cfgtick::disk::Wrapper,std::uint8_t{});CHECK(!f.Observe(0,&e)&&e==cfgtick::Failure::Wrapper);}
    {TickFixture f;cfgtick::Failure e{};f.Put(f.function+cfgnames::disk::ObjectName,cfgnames::Words{74,0});CHECK(!f.Observe(0,&e)&&e==cfgtick::Failure::FunctionName);}
    {TickFixture f;cfgtick::Failure e{};f.Put(f.function+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{8});CHECK(!f.Observe(0,&e)&&e==cfgtick::Failure::ParameterSize);}
    {TickFixture f;cfgtick::Failure e{};f.trigger=f.module+cfgprobe::lifetime::ObjectArrayRva+cfgprobe::lifetime::ArrayCountOffset;
     f.mutation=f.trigger;f.newPointer=static_cast<std::uintptr_t>(f.objects.count+1);f.triggerAt=2;
     CHECK(f.Observe(0,&e)&&e==cfgtick::Failure::None);}
    {TickFixture f;cfgtick::Failure e{};constexpr std::uintptr_t moved=0x720000;f.CopyKnownSlots(moved);
     f.trigger=f.module+cfgprobe::lifetime::ObjectArrayRva+cfgprobe::lifetime::ArrayDataOffset;
     f.mutation=f.trigger;f.newPointer=moved;f.triggerAt=2;
     const bool observed=f.Observe(0,&e);if(!observed)std::printf("Relocated-header diagnostic: failure=%u reads=%u\n",static_cast<unsigned>(e),f.reads);
     CHECK(observed&&e==cfgtick::Failure::None);}
    {TickFixture f;cfgtick::Failure e{};constexpr std::uintptr_t moved=0x720000;f.CopyKnownSlots(moved);
     int index=0;CHECK(f.Read(f.function+cfgprobe::lifetime::ObjectIndexOffset,index));
     f.Put(moved+static_cast<std::uintptr_t>(index)*cfgprobe::lifetime::ItemBytes+
         cfgprobe::lifetime::ItemSerialOffset,std::int32_t{99});
     f.trigger=f.module+cfgprobe::lifetime::ObjectArrayRva+cfgprobe::lifetime::ArrayDataOffset;
     f.mutation=f.trigger;f.newPointer=moved;f.triggerAt=2;
     CHECK(!f.Observe(0,&e)&&e==cfgtick::Failure::RegistryStable);}
    {TickFixture f;cfgtick::Failure e{};constexpr std::uintptr_t moved=0x720000;f.CopyKnownSlots(moved);
     f.Put(f.module+cfgprobe::lifetime::ObjectArrayRva+cfgprobe::lifetime::ArrayDataOffset,moved);
     f.trigger=f.pawn+cfgprobe::lifetime::ObjectIndexOffset;
     f.mutation=f.module+cfgprobe::lifetime::ObjectArrayRva+cfgprobe::lifetime::ArrayDataOffset;
     f.newPointer=0x740000;
     // Relocation during endpoint slot checks must still be refused.
     CHECK(!f.Observe(0,&e)&&e==cfgtick::Failure::RegistryStable);}
    {TickFixture f;cfgtick::Failure e{};int index=0;f.Read(f.function+cfgprobe::lifetime::ObjectIndexOffset,index);
     f.Put(f.objects.data+static_cast<std::uintptr_t>(index)*cfgprobe::lifetime::ItemBytes+
         cfgprobe::lifetime::ItemSerialOffset,std::int32_t{});
     CHECK(f.Observe(0,&e)&&e==cfgtick::Failure::None);}
    {TickFixture f;cfgtick::Failure e{};int index=0;f.Read(f.pawn+cfgprobe::lifetime::ObjectIndexOffset,index);
     f.Put(f.objects.data+static_cast<std::uintptr_t>(index)*cfgprobe::lifetime::ItemBytes+
         cfgprobe::lifetime::ItemSerialOffset,std::int32_t{});
     CHECK(!f.Observe(0,&e)&&e==cfgtick::Failure::ReceiverSlot);}
    {TickFixture f;cfgtick::Failure e{};int index=0;f.Read(f.function+cfgprobe::lifetime::ObjectIndexOffset,index);
     f.Put(f.objects.data+static_cast<std::uintptr_t>(index)*cfgprobe::lifetime::ItemBytes,
         std::uintptr_t{0x7F000000});
     CHECK(!f.Observe(0,&e)&&e==cfgtick::Failure::FunctionSlot);}
    {TickFixture f;cfgtick::Failure e{};int index=0;f.Read(f.function+cfgprobe::lifetime::ObjectIndexOffset,index);
     f.Put(f.objects.data+static_cast<std::uintptr_t>(index)*cfgprobe::lifetime::ItemBytes+8,
         std::uint32_t{0x20000000});
     CHECK(!f.Observe(0,&e)&&e==cfgtick::Failure::FunctionFlagged);}
    {TickFixture f;cfgtick::Failure e{};int index=0;f.Read(f.function+cfgprobe::lifetime::ObjectIndexOffset,index);
     f.memory.erase(f.objects.data+static_cast<std::uintptr_t>(index)*cfgprobe::lifetime::ItemBytes+8);
     CHECK(!f.Observe(0,&e)&&e==cfgtick::Failure::FunctionFlagsRead);}
    {TickFixture f;f.trigger=f.module+cfgtick::disk::Name;f.mutation=f.trigger;f.newPointer=74;CHECK(!f.Observe());}
    {TickFixture f;f.trigger=f.function+cfgnames::disk::ObjectName;f.mutation=f.trigger;f.newPointer=74;CHECK(!f.Observe());}
    {TickFixture f;f.Put(f.module+cfgtick::disk::Name,cfgnames::Words{});CHECK(!f.Observe());}
    std::printf("ReceiveTick dynamic override/origin/name/frame/serial stability: %u failures; synthetic memory only\n",failures);
    return failures?1:0;
}
