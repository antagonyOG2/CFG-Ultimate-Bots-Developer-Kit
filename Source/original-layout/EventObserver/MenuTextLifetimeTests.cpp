#include "MenuTextLifetime.hpp"
#include <cstdio>
int main(){
    using namespace cfgmenu::reflection;unsigned failures=0;
    auto check=[&](bool b){if(!b)++failures;};RetainedMenuText pool;RetainedMenuText::Request a,b;
    check(!pool.Begin(u"",a)&&pool.Used()==0);check(!pool.Begin(std::u16string_view(u"a\0b",3),a));
    check(pool.Begin(u"Counselor bot skill",a)&&a.packet->input.count==20&&!pool.Borrow(a));
    for(auto value:a.packet->result.bytes)check(value==0);
    const std::uintptr_t pointers[]{0x100000,0x200000};std::memcpy(a.packet->result.bytes.data(),pointers,16);
    check(pool.Complete(a,true)&&pool.Borrow(a)==&a.packet->result&&!pool.Complete(a,true));
    check(pool.Begin(u"Failed conversion",b));std::memcpy(b.packet->result.bytes.data(),pointers,16);
    check(!pool.Complete(b,false)&&!pool.Borrow(b)&&!pool.Complete(b,true));
    auto wrong=a;wrong.packet=b.packet;check(!pool.Borrow(wrong)&&!pool.Complete(wrong,true));
    check(pool.Begin(u"Missing native output",b)&&!pool.Complete(b,true)&&!pool.Borrow(b));
    while(pool.Used()<RetainedMenuText::Capacity)check(pool.Begin(u"Retained",b));check(!pool.Begin(u"Full",b)&&!b.packet);
    check(pool.Borrow(a)==&a.packet->result&&a.packet->input.data[0]==u'C');
    std::printf("MenuTextLifetimeTests: %u failures; opaque retained packets only, no native calls\n",failures);return failures?1:0;
}
