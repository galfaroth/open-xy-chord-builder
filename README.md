# Chords by Veff

A two-hand chord player for OP-XY / Open-XY SDK 1.8. The left octave chooses a chord quality; the right octave plays scale degrees. It transforms notes before the track's existing instrument, so use a polyphonic synth or sample engine for full chords. It does not replace the instrument or add audio effects.

Default: C major, automatic diatonic triads, close position, no strum. Right F plays C major; G plays D minor; A plays E minor. Select **maj** on the left to force major chords on every degree instead. Multiple right-hand roots can sound together.

## Keyboard

Positions follow the OP-XY's **two F–E octaves**, at the current keyboard octave. All choices and toggles latch: tap a control, then release it. Control keys are silent; releasing a sounding degree releases its complete chord, including queued strum notes. There is no sustain latch.

| Left key | Quality | Right key | Action |
|---|---|---|---|
| F | minor | F | degree I |
| F# | automatic diatonic triad | F# | inversion up, maximum +3 |
| G | major | G | degree II |
| G# | sus2 | G# | inversion down, minimum −3 |
| A | sus4 | A | degree III |
| A# | diminished | A# | toggle open voicing |
| B | major sixth | B | degree IV |
| C | minor seventh | C | degree V |
| C# | dominant seventh | C# | toggle root bass, one octave down |
| D | major seventh | D | degree VI |
| D# | dominant ninth | D# | toggle ninth + eleventh colour |
| E | major add9 | E | degree VII |

The caption shows quality, signed inversion and active `O` (open), `B` (bass), `9/11` (colour) switches. Input dots mark the selected quality, enabled toggles and held roots. The lower keyboard is SDK-fed **actually emitted notes**, not a simulated chord graphic; delayed strum notes light when they sound.

The tonic sits on C's octave immediately below the right-hand F. The **Key** parameter then transposes it. Changing the keyboard octave moves new chords by an octave; already-held chords keep their register and release using their original MIDI note. Notes outside the current 24-key range pass through unchanged.

## Encoders

| Control | Choices |
|---|---|
| Key | C through B, chromatic |
| Scale | major, natural minor, Dorian, Mixolydian, Lydian, harmonic minor |
| Strum | 0, 1, 2, 3, 4, 6, 8 or 12 native player steps between tones |
| Style | Close, Open, Wide, Flume |

**Open** raises alternating chord tones one octave. **Wide** spreads successive tones over three registers. **Flume** is an original Flume-inspired electronic voicing: wide upper tones, added ninth/thirteenth, diatonic seventh in Auto, and a low root. It is not a transcription of any artist's chords or a claim to reproduce Orchid's proprietary algorithm. The keyboard's open/bass/colour switches combine with styles. Added colour tones may leave the chosen scale; Auto's basic triad is diatonic.

Inversions apply before spreading. At MIDI limits pitches fold by octaves, preserving pitch class; duplicate pitches are removed. A chord has at most eight tones. The stock instrument track has up to eight sounding voices; seven held degree keys do not mean 56 simultaneous voices. Voice stealing still applies with rich or overlapping chords.

A quality, inversion, toggle, key, scale, style or strum change **releases and restrums** each held chord. It is not a pitch glide or seamless voice-leading mode. Strum uses the stock player's clock steps, not milliseconds. Stop/all-notes-off clears held roots and control-key ownership; performance selections remain until the player state is recreated.

## Recording and saving

The four encoder parameters are the SDK's saved player settings. Keyboard quality, inversion and toggle selections are transient performance state, **not guaranteed project data**. Choose them again after loading/recreating the player. Recording the relevant control-key notes can reproduce the changes.

The player transforms live and sequenced **input** notes. Generated chord tones are not promised to be baked into the sequencer: recorded degree/control notes are transformed on playback. Their role follows the keyboard's then-current lowest F. Keep the same keyboard octave and parameters to reproduce a take. This player does not change stock recording behaviour.

Keep the firmware's `layout.json`: SDK 1.8 project companions do not yet remap player identities. Root builds the combined development firmware separately; this repository contains source only.

## Development

Tested against SDK `7fd8fdf1757e11ab6fdca915b09f1773d7611a31`. Point `SDK` below at that checkout:

```sh
SDK=../../five-machines-sdk
TMPDIR=/private/tmp python3 "$SDK/sdk.py" test --machine .
python3 artwork.py
```

Host semantic tests cover all 24 keys across five octaves, exact chord qualities, all scales/styles, MIDI limits, live changes, seven-root update draining, strum scheduling, release ownership, key repeat and reset. `artwork.py` regenerates the original SVG; native compilation also validates its bindings. Players have no SDK browser audio audition or audio-render CPU estimate.

That SDK revision's **player-only** `compile` hits an unused helper warning in generated `startup.cpp`. Native validation therefore uses a collection containing this player and the official core `fm2op`; The prepared workflow in `ci/machine.yml.example` does the same without changing SDK code or suppressing warnings. It is inactive: the publishing token lacks GitHub workflow permission. Move it to `.github/workflows/machine.yml` using an account permitted to manage workflows to enable CI. The combined firmware build also includes other machines.

Hardware timing, recording and live performance validation remain pending. No device operations or CPU measurement are implied by host/native compilation.

## Sources and licence

Implementation and artwork are original MIT-licensed work. The public player API and official [Open-XY Chords example](https://github.com/etcetc000/open-xy-core-machines/tree/main/machines/chords) informed SDK integration.

Conceptual references requested for research: [ChoralRoot FM-1](https://github.com/Quixotic7/ChoralRootFM1) and [CoralFlower](https://github.com/FicusFicusov/CoralFlower), both GPL-3.0 projects. Their README descriptions informed the two-hand chord-instrument idea; no source code, voicing tables or artwork from those projects was copied or linked. This is neither their port nor affiliated with Telepathic Instruments, Orchid, Flume or Teenage Engineering.
