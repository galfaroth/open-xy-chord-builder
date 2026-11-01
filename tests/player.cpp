#include <initializer_list>
#include <cstdio>
#include <cstdlib>
#include "../player.cpp"
using namespace veffchord;
void require(bool ok,const char* what) {if(!ok){std::fprintf(stderr,"FAIL %s\n",what);std::exit(1);}}
i32 value(u32 index,u32 count) {return static_cast<i32>((static_cast<unsigned long long>(2*index+1)*0x7fffffff)/(2*count));}
Controls controls(u32 low=53) {Controls c{};c.lowest_key=low;return c;}
void tap(State& s,Controls& c,u32 key) {Plan p{};require(press(s,c,c.lowest_key+key,90,p)&&p.count==0,"silent control");require(release(s,c.lowest_key+key),"silent release");}
void valid(const Plan& p) {require(p.count<=8,"8 tone bound");for(u32 i=0;i<p.count;++i){require(p.tones[i].pitch<=127 && p.tones[i].velocity<=127,"MIDI range");if(i)require(p.tones[i].pitch>p.tones[i-1].pitch,"ascending strum after MIDI folding");for(u32 j=0;j<i;++j)require(p.tones[j].pitch!=p.tones[i].pitch,"unique pitches");}}
int main() {
    require(sizeof(State)<=state_bytes && alignof(State)<=state_alignment,"state ABI");
    for(u32 low: {5u,29u,53u,77u,101u}) {
        Controls c=controls(low);
        for(u32 key=0;key<24;++key) {
            State s{};Plan p{};bool white_root=false;
            for(i32 w:white)if(key==12+static_cast<u32>(w))white_root=true;
            require(press(s,c,low+key,103,p),"all24 claimed");
            require((p.count>0)==white_root,"24 key roles");valid(p);
            c.lowest_key=low+12;require(release(s,low+key)==!white_root,"release across octave change");
            Update u{};require(!update(s,c,u),"no released root resurrected");c.lowest_key=low;
        }
    }
    const u8 expected[12][5]={{60,63,67,0,0},{60,64,67,0,0},{60,64,67,0,0},{60,62,67,0,0},
        {60,65,67,0,0},{60,63,66,0,0},{60,64,67,69,0},{60,63,67,70,0},{60,64,67,70,0},
        {60,64,67,71,0},{60,64,67,70,74},{60,64,67,74,0}};
    for(u32 key=0;key<12;++key){State a{};Controls b=controls();Plan q{};tap(a,b,key);press(a,b,65,100,q);
        u32 count=0;while(count<5&&expected[key][count])++count;require(q.count==count,"quality tone count");
        for(u32 n=0;n<count;++n)require(q.tones[n].pitch==expected[key][n],"quality pitches");}
    Controls c=controls();State s{};Plan p{};Update u{};
    press(s,c,65,100,p);require(p.count==3&&p.tones[0].pitch==60&&p.tones[1].pitch==64&&p.tones[2].pitch==67,"C major degreeI");
    tap(s,c,0);require(update(s,c,u)&&u.root==65&&u.mode==Replace&&u.plan.tones[1].pitch==63,"held major to minor");require(!update(s,c,u),"update drains");
    tap(s,c,2);require(update(s,c,u)&&u.plan.tones[1].pitch==64,"major restores");
    tap(s,c,13);require(update(s,c,u)&&u.plan.tones[0].pitch==64,"first inversion");
    for(int n=0;n<8;++n)tap(s,c,13);require(s.inversion==3,"upper inversion bounded");
    for(int n=0;n<10;++n)tap(s,c,15);require(s.inversion==-3,"lower inversion bounded");
    tap(s,c,17);tap(s,c,20);tap(s,c,22);require(s.open&&s.bass&&s.color,"all voicing toggles");
    reset(s);require(s.open&&s.bass&&s.color&&s.quality==Major,"reset keeps performance selection");require(!update(s,c,u),"reset clears roots");
    State auto_s{};for(u32 degree=0;degree<7;++degree){press(auto_s,c,65+white[degree],90,p);require(p.tones[0].pitch==60+degrees[0][degree],"diatonic root mapping");release(auto_s,65+white[degree]);}
    press(auto_s,c,65,90,p);c.value[0]=value(9,12);c.value[1]=value(1,6);require(update(auto_s,c,u)&&u.plan.tones[0].pitch==69&&u.plan.tones[1].pitch==72,"live key and scale");
    c.value[2]=value(7,8);c.value[3]=value(3,4);require(update(auto_s,c,u)&&u.plan.count>=5,"wide extended style");for(u32 i=0;i<u.plan.count;++i)require(u.plan.tones[i].delay==12*i,"strum delays");
    release(auto_s,65);require(!update(auto_s,c,u),"strum release leaves no update");
    // Test every quality/style/scale at both MIDI ends, with all modifiers.
    for(u32 low: {0u,5u,101u,104u})for(u32 scale=0;scale<6;++scale)for(u32 style=0;style<4;++style)for(u32 q=0;q<12;++q){
        State a{};a.quality=q;a.inversion=3;a.open=a.bass=a.color=true;Controls b=controls(low);b.value[0]=value(11,12);b.value[1]=value(scale,6);b.value[3]=value(style,4);
        for(u32 d=0;d<7;++d)if(low+12+white[d]<=127){press(a,b,low+12+white[d],127,p);valid(p);release(a,low+12+white[d]);}
    }
    // All seven roots update once, then quiesce; releases never consume sounding roots.
    State many{};c=controls();for(i32 w:white)press(many,c,65+w,90,p);tap(many,c,7);u32 updates=0;while(update(many,c,u)){require(++updates<=7,"bounded update drain");valid(u.plan);}require(updates==7,"all held roots follow");for(i32 w:white)require(!release(many,65+w),"root stock release");
    State repeated{};c=controls();press(repeated,c,70,90,p);press(repeated,c,70,90,p);require(repeated.open,"repeat control toggles once");release(repeated,70);tap(repeated,c,17);require(!repeated.open,"next real press toggles");
    require(!press(repeated,c,12,90,p)&&!release(repeated,12),"external notes pass through");
    Display d{};display(s,c,d);require(d.text[31]==0,"caption terminated");for(auto& v:d.values)require(v[7]==0,"header terminated");
    std::puts("PASS:24-key mapping, MIDI edges, scales, voicings, releases/reset and live updates");
}
