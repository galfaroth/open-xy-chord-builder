"""Generate original monochrome input legend and the SDK's output-keyboard bindings."""
from pathlib import Path
from html import escape
lines=['<g id="veffchord_panel">']
def text(x,y,s,size=10,fill='#cbcbcb',ident=None):
    attr='' if ident is None else f' id="{ident}"'
    lines.append(f'<text{attr} transform="matrix(1 0 0 1 {x} {y})" text-anchor="middle" font-family="UniversNextPro-Light" font-size="{size}" fill="{fill}">{escape(s)}</text>')
text(240,45,'CHORDS by Veff',14)
text(240,68,'auto i+0',15,ident='veffchord_text')
text(124,85,'QUALITY',8);text(344,85,'DEGREE / VOICING',8)
white=[0,2,4,6,7,9,11]
labels=['min','auto','maj','sus2','sus4','dim','maj6','min7','dom7','maj7','9','add9','I','inv+','II','inv-','III','open','IV','V','bass','VI','color','VII']
for octave in range(2):
    origin=24+octave*220
    for j,offset in enumerate(white):
        x=origin+30*j;idx=octave*12+offset
        lines.append(f'<path d="M0 0h28v60h-28Z" transform="matrix(1 0 0 1 {x} 94)" fill="#cbcbcb" stroke="#000000" stroke-width="1"/>')
        text(x+14,144,labels[idx],9,'#000000')
        lines.append(f'<circle id="veffchord_dot_{idx}" cx="0" cy="0" r="3" transform="matrix(1 0 0 1 {x+14} 128)" fill="#ffffff" stroke="#000000" stroke-width="1"/>')
    for offset,left in [(1,0),(3,1),(5,2),(8,4),(10,5)]:
        x=origin+left*30+20;idx=octave*12+offset
        lines.append(f'<path d="M0 0h20v29h-20Z" transform="matrix(1 0 0 1 {x} 94)" fill="#161616" stroke="#000000" stroke-width="1"/>')
        text(x+10,105,labels[idx],7)
        lines.append(f'<circle id="veffchord_dot_{idx}" cx="0" cy="0" r="2" transform="matrix(1 0 0 1 {x+10} 115)" fill="#ffffff" stroke="#000000" stroke-width="1"/>')
text(240,167,'SOUNDING NOTES',8)
# White paths must precede black paths. SDK positions/scrolls all output keys.
for black in [False,True]:
    for note in range(128):
        if ((note%12) in [1,3,6,8,10])!=black:continue
        w,h=(6,27) if black else (10,44)
        fill='#161616' if black else '#cbcbcb'
        lines.append(f'<path id="veffchord_key_{note}" d="M0 0h{w}v{h}h-{w}Z" transform="matrix(1 0 0 1 640 174)" fill="{fill}" stroke="#000000" stroke-width="1"/>')
for note in range(128):
    lines.append(f'<circle id="veffchord_played_{note}" cx="0" cy="0" r="2" transform="matrix(1 0 0 1 640 208)" fill="#ffffff" stroke="#000000" stroke-width="1"/>')
lines.append('</g>')
Path(__file__).with_name('panel.svg.fragment').write_text('\n'.join(lines)+'\n')
