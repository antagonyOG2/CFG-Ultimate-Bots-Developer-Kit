#include "VehicleOccupancyObservation.hpp"
#define main CfgOldOccupancyFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
int main(){
    using cfgvehicleoccupancy::Result;Fixture f;f.cap=1536;
    const auto seatType=f.Object(),car=f.Object(),driver=f.bot,seat=f.Object(seatType),passenger=f.Object(seatType);
    const std::uintptr_t data=0x740000;
    const auto serial=[&](std::uintptr_t object){std::int32_t i=0;CHECK(f.Read(object+0xC,i));f.Put(f.objects.data+24ULL*i+16,std::int32_t{100+i});};
    for(auto object:{car,driver,seat,passenger})serial(object);
    f.Put(car+cfgvehicleinteraction::disk::Seats,cfgspawn::PlayerArray{data,2,2});
    f.Put(data,seat);f.Put(data+8,passenger);
    for(auto s:{seat,passenger}){f.Put(s+cfgvehicleinteraction::disk::ParentVehicle,car);f.Put(s+cfgvehicleinteraction::disk::CounselorInSeat,std::uintptr_t{0});}
    f.Put(seat+cfgvehicleoccupancy::disk::DriverSeat,std::uint8_t{1});
    f.Put(passenger+cfgvehicleoccupancy::disk::DriverSeat,std::uint8_t{0});
    f.Put(seat+cfgvehicleinteraction::disk::CounselorInSeat,driver);
    f.Put(driver+cfgvehicleoccupancy::disk::CurrentSeat,seat);
    f.Put(driver+cfgvehicleoccupancy::disk::LeavingSeat,std::uint8_t{0});
    const auto observe=[&](Fixture& r,cfgvehicleoccupancy::Proof& proof){return cfgvehicleoccupancy::Observe(r,r.objects,car,seatType,r.counselorBase,proof);};
    cfgvehicleoccupancy::Proof proof;f.reads=0;CHECK(observe(f,proof)==Result::Occupied&&proof.driver==driver&&proof.seat==seat);
    CHECK(cfgvehicleoccupancy::Current(f,f.objects,seatType,f.counselorBase,proof));CHECK(f.reads<1536);
    const auto peak=f.reads;
    // Native cars have up to eight admitted seats. Exercise the real adapter
    // with deep class ancestry and charge a conservative outer/native reserve.
    auto worst=f;worst.cap=1536;worst.reads=512;
    auto ancestor=seatType;
    for(unsigned depth=0;depth<10;++depth){const auto child=worst.Object();worst.Put(child+0x30,ancestor);ancestor=child;}
    worst.Put(car+cfgvehicleinteraction::disk::Seats,cfgspawn::PlayerArray{data,8,8});
    for(unsigned k=0;k<8;++k){const auto s=k==0?seat:worst.Object(ancestor);
        if(k==0)worst.Put(s+0x10,ancestor);
        std::int32_t i=0;CHECK(worst.Read(s+0xC,i));worst.Put(worst.objects.data+24ULL*i+16,std::int32_t{100+i});
        worst.Put(data+8ULL*k,s);worst.Put(s+cfgvehicleinteraction::disk::ParentVehicle,car);
        worst.Put(s+cfgvehicleoccupancy::disk::DriverSeat,std::uint8_t(k==0));
        worst.Put(s+cfgvehicleinteraction::disk::CounselorInSeat,k==0?driver:std::uintptr_t{0});
    }
    worst.reads=512;cfgvehicleoccupancy::Proof full;CHECK(observe(worst,full)==Result::Occupied);
    CHECK(cfgvehicleoccupancy::Current(worst,worst.objects,seatType,worst.counselorBase,full));
    CHECK(worst.reads<=1536);
    std::printf("Occupancy eight-seat/depth10 plus512 outer reads=%u/1536\n",worst.reads);
    // The old car.Driver field is deliberately absent. Only reciprocal native seating counts.
    for(unsigned mode=0;mode<8;++mode){auto bad=f;bad.reads=0;cfgvehicleoccupancy::Proof p;
        if(mode==0)bad.Put(driver+cfgvehicleoccupancy::disk::CurrentSeat,std::uintptr_t{0});
        if(mode==1)bad.Put(driver+cfgvehicleoccupancy::disk::LeavingSeat,std::uint8_t{1});
        if(mode==2)bad.Put(seat+cfgvehicleinteraction::disk::CounselorInSeat,std::uintptr_t{0});
        if(mode==3)bad.Put(seat+cfgvehicleinteraction::disk::ParentVehicle,driver);
        if(mode==4)bad.Put(passenger+cfgvehicleoccupancy::disk::DriverSeat,std::uint8_t{1});
        if(mode==5)bad.Put(driver+cfgvehicleoccupancy::disk::LeavingSeat,std::uint8_t{2});
        if(mode==6)bad.Put(data+8,seat);
        if(mode==7)bad.cap=10;
        const auto result=observe(bad,p);
        CHECK(result==(mode==0||mode==2?Result::Empty:mode==1?Result::Exiting:Result::Unknown));
        CHECK(!cfgvehicleoccupancy::Current(bad,bad.objects,seatType,bad.counselorBase,proof));
    }
    auto exit=f;exit.reads=0;exit.trigger=driver+cfgvehicleoccupancy::disk::CurrentSeat;
    exit.mutation=exit.trigger;exit.newPointer=0;exit.triggerAt=2;
    cfgvehicleoccupancy::Proof changed;CHECK(observe(exit,changed)==Result::Unknown);
    auto replaced=f;replaced.reads=0;std::int32_t i=0;CHECK(replaced.Read(seat+0xC,i));
    replaced.Put(replaced.objects.data+24ULL*i+16,std::int32_t{999});
    CHECK(!cfgvehicleoccupancy::Current(replaced,replaced.objects,seatType,replaced.counselorBase,proof));
    std::printf("CFG reciprocal vehicle occupancy tests: %s; peak=%u/1536; no game access.\n",failures?"FAIL":"PASS",peak);
    return failures?1:0;
}
