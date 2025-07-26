#pragma leco dll
import dotz;
import gerby;
import pocpoc;
import traits;

using namespace gerby::literals;
using namespace gerby::palette;
using namespace gerby;
using namespace pocpoc;

// 4017
const auto ic1 = dip<16>({});
// 4518
const auto ic2 = dip<16>({});
// 4001
const auto ic3 = dip<14>({});
const auto ic4 = dip<14>({});
// 4016
const auto ic5 = dip<14>({});
// 4081
const auto ic6 = dip<14>({});
// 4026
const auto ic7 = dip<16>({});

// 10k
const auto r1 = r0603({});
// 1M
const auto r2 = r0603({});
// 220k
const auto r3 = r0603({});
// 390k
const auto r4 = r0603({});
// 820k
const auto r5 = r0603({});
// 470k
const auto r6 = r0603({});
// 1M
const auto r7 = r0603({});
// 470k
const auto r8 = r0603({});
// 10k
const auto r9 = r0603({});
// 10M
const auto r10 = r0603({});
// 470k
const auto r11 = r0603({});

// 100n
const auto c1 = r0603({});
// 1u
const auto c2 = r0603({});
// 100n
const auto c3 = r0603({});
// 220n
const auto c4 = r0603({});

// Common Cathode 7-Segment Display
const auto dis = dip<14>({});

const auto bat = header<2>({});

// Red LEDs
const auto d1 = d0603({});
const auto d2 = d0603({});
const auto d3 = d0603({});
const auto d4 = d0603({});
const auto d5 = d0603({});
const auto d6 = d0603({});
const auto d7 = d0603({});
const auto d8 = d0603({});
const auto d9 = d0603({});
// Green LED
const auto d10 = d0603({});

// TODO: 3 push
// TODO: switch

#ifdef LECO_TARGET_WINDOWS
#define A __declspec(dllexport)
#else
#define A
#endif

extern "C" A void draw(cnc::builder * b, cnc::grb_layer l) {
}

extern "C" A void cpl(cpl::builder * b) {
}
