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

#ifdef LECO_TARGET_WINDOWS
#define A __declspec(dllexport)
#else
#define A
#endif

extern "C" A void draw(cnc::builder * b, cnc::grb_layer l) {
}

extern "C" A void cpl(cpl::builder * b) {
}
