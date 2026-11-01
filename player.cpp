#include <opxy/player.hpp>

namespace veffchord {
using opxy::u8;
using opxy::u32;
using opxy::i32;
using namespace opxy::player;
enum Quality { Auto, Minor, Major, Sus4, Major6, Minor7, Major7, Add9, Sus2, Dim, Dom7, Ninth };
struct Root { u32 note, degree, velocity; i32 anchor; bool held; Plan last; };
struct State {
    u8 consumed[128]; // Release ownership survives keyboard octave changes.
    Root roots[8];
    u32 quality;
    i32 inversion;
    bool open, bass, color;
};
const u8 degrees[6][7] = {{0,2,4,5,7,9,11},{0,2,3,5,7,8,10},{0,2,3,5,7,9,10},
    {0,2,4,5,7,9,10},{0,2,4,6,7,9,11},{0,2,3,5,7,8,11}};
const i32 white[7] = {0,2,4,6,7,9,11};
const char* const qualities[] = {"auto","min","maj","sus4","maj6","min7","maj7","add9","sus2","dim","dom7","9"};
const char* const keys[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
const char* const scales[] = {"Major","Minor","Dorian","Mixolyd","Lydian","HarmMin"};
const char* const styles[] = {"Close","Open","Wide","Flume"};
const u32 strums[] = {0,1,2,3,4,6,8,12};
// Multiply only the upper Q31 bits: exact enum buckets without wide arithmetic.
u32 bucket(i32 value, u32 choices) {
    const u32 n = ((value < 0 ? 0u : static_cast<u32>(value) >> 16) * choices) >> 15;
    return n < choices ? n : choices - 1;
}
i32 scale_interval(u32 scale, u32 degree, u32 distance) {
    const u32 d = degree + distance;
    return degrees[scale][d % 7] + static_cast<i32>(12 * (d / 7)) - degrees[scale][degree];
}
void add(i32* notes, u32& count, i32 value) {
    if (count == max_tones) return;
    for (u32 i=0;i<count;++i) if (notes[i]==value) return;
    notes[count++]=value;
}
void sort(i32* notes,u32 count) {
    for(u32 i=1;i<count;++i) for(u32 j=i;j && notes[j]<notes[j-1];--j) {
        const i32 value=notes[j]; notes[j]=notes[j-1]; notes[j-1]=value;
    }
}
Plan chord(const State& s,const Controls& c,const Root& root) {
    const u32 scale=bucket(c.value[1],6), style=bucket(c.value[3],4);
    const i32 tonic=root.anchor+static_cast<i32>(bucket(c.value[0],12))+degrees[scale][root.degree];
    i32 notes[8];u32 count=0;
    const u32 q=s.quality;
    add(notes,count,0);
    const i32 third=q==Auto ? scale_interval(scale,root.degree,2) :
        q==Minor || q==Minor7 || q==Dim ? 3 : q==Sus4 ? 5 : q==Sus2 ? 2 : 4;
    add(notes,count,third);
    add(notes,count,q==Auto ? scale_interval(scale,root.degree,4) : q==Dim ? 6 : 7);
    if(q==Major6) add(notes,count,9);
    if(q==Minor7 || q==Dom7 || q==Ninth) add(notes,count,10);
    if(q==Major7) add(notes,count,11);
    if(q==Add9 || q==Ninth || s.color || style==3) add(notes,count,14);
    if(s.color) add(notes,count,17);
    if(style==3) { add(notes,count,21); if(q==Auto) add(notes,count,scale_interval(scale,root.degree,6)); }
    sort(notes,count);
    for(i32 n=0;n<s.inversion;++n) { notes[0]+=12;sort(notes,count); }
    for(i32 n=0;n>s.inversion;--n) { notes[count-1]-=12;sort(notes,count); }
    if(s.open || style==1) for(u32 n=1;n<count;n+=2) notes[n]+=12;
    if(style==2 || style==3) for(u32 n=1;n<count;++n) notes[n]+=12*static_cast<i32>(n%3);
    if(s.bass || style==3) add(notes,count,-12);
    sort(notes,count);
    i32 emitted[8]; u32 emitted_count=0;
    for(u32 i=0;i<count;++i) {
        i32 pitch=tonic+notes[i];
        // Fold at MIDI edges by octaves, preserving pitch classes instead of clipping.
        while(pitch<0) pitch+=12;
        while(pitch>127) pitch-=12;
        add(emitted,emitted_count,pitch);
    }
    sort(emitted,emitted_count);
    Plan plan{};
    const u32 strum=strums[bucket(c.value[2],8)];
    for(u32 i=0;i<emitted_count;++i)
        plan.tones[plan.count++]={static_cast<u8>(emitted[i]),static_cast<u8>(root.velocity),static_cast<opxy::u16>(i*strum)};
    return plan;
}
bool equal(const Plan& a,const Plan& b) {
    if(a.count!=b.count) return false;
    for(u32 i=0;i<a.count;++i) if(a.tones[i].pitch!=b.tones[i].pitch || a.tones[i].velocity!=b.tones[i].velocity || a.tones[i].delay!=b.tones[i].delay) return false;
    return true;
}
bool press(State& s,const Controls& c,u32 note,u32 velocity,Plan& plan) {
    plan=Plan{};
    if(note>127 || velocity>127 || c.lowest_key>127) return false;
    if(s.consumed[note]) return true; // Key repeat must not toggle twice.
    for(const auto& held:s.roots) if(held.held && held.note==note) {plan=held.last;return true;}
    if(note<c.lowest_key || note-c.lowest_key>=24) return false;
    const u32 pos=note-c.lowest_key;
    i32 degree=-1;
    for(u32 i=0;i<7;++i) if(pos==12+static_cast<u32>(white[i])) degree=static_cast<i32>(i);
    if(degree>=0) {
        Root* slot=nullptr;
        for(auto& r:s.roots) if(r.held && r.note==note) slot=&r;
        if(!slot) for(auto& r:s.roots) if(!r.held) {slot=&r;break;}
        if(!slot) return true;
        *slot=Root{note,static_cast<u32>(degree),velocity,static_cast<i32>(c.lowest_key)+7,true,{}};
        slot->last=chord(s,c,*slot);plan=slot->last;
        return true;
    }
    s.consumed[note]=1;
    if(pos<12) {
        const u8 mapping[12]={Minor,Auto,Major,Sus2,Sus4,Dim,Major6,Minor7,Dom7,Major7,Ninth,Add9};
        s.quality=mapping[pos];
    } else {
        if(pos==13 && s.inversion<3) ++s.inversion;
        if(pos==15 && s.inversion>-3) --s.inversion;
        if(pos==17) s.open=!s.open;
        if(pos==20) s.bass=!s.bass;
        if(pos==22) s.color=!s.color;
    }
    return true;
}
bool release(State& s,u32 note) {
    if(note>127) return false;
    // Sounding ownership takes priority if octave changes made an old root a control.
    for(auto& r:s.roots) if(r.held && r.note==note) {r.held=false;return false;}
    if(s.consumed[note]) {s.consumed[note]=0;return true;}
    return false;
}
bool update(State& s,const Controls& c,Update& out) {
    for(auto& r:s.roots) if(r.held) {
        const Plan next=chord(s,c,r);
        if(!equal(next,r.last)) {r.last=next;out.root=r.note;out.mode=Replace;out.plan=next;return true;}
    }
    return false;
}
void reset(State& s) {
    for(auto& r:s.roots) r.held=false;
    for(auto& held:s.consumed) held=0;
}
void copy(char* to,u32 size,const char* from) {u32 i=0;for(;i+1<size && from[i];++i) to[i]=from[i];to[i]=0;}
void display(const State& s,const Controls& c,Display& out) {
    out=Display{};
    copy(out.values[0],8,keys[bucket(c.value[0],12)]);
    copy(out.values[1],8,scales[bucket(c.value[1],6)]);
    const char* const delays[] = {"0","1 step","2 steps","3 steps","4 steps","6 steps","8 steps","12steps"};
    copy(out.values[2],8,delays[bucket(c.value[2],8)]);
    copy(out.values[3],8,styles[bucket(c.value[3],4)]);
    // Current quality plus inversion/toggle status in the stock 32-byte caption.
    char* text=out.text;u32 n=0;const char* q=qualities[s.quality<12?s.quality:0];
    while(*q && n<10) text[n++]=*q++;
    text[n++]=' ';text[n++]='i';text[n++]=s.inversion<0?'-':'+';
    text[n++]=static_cast<char>('0'+(s.inversion<0?-s.inversion:s.inversion));
    if(s.open) {text[n++]=' ';text[n++]='O';}
    if(s.bass) {text[n++]=' ';text[n++]='B';}
    if(s.color) {text[n++]=' ';text[n++]='9';text[n++]='/';text[n++]='1';text[n++]='1';}
    text[n]=0;
    const u8 quality_key[12]={1,0,2,4,6,7,9,11,3,5,8,10};
    out.lit=1u<<quality_key[s.quality<12?s.quality:0];
    if(s.open) out.lit|=1u<<17;
    if(s.bass) out.lit|=1u<<20;
    if(s.color) out.lit|=1u<<22;
    for(const auto& r:s.roots) if(r.held && r.note>=c.lowest_key && r.note-c.lowest_key<24) out.lit|=1u<<(r.note-c.lowest_key);
}
}
