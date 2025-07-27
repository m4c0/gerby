#pragma leco dll
import dotz;
import gerby;
import traits;
import pocpoc;

using namespace gerby::literals;
using namespace gerby::palette;
using namespace gerby;
using namespace pocpoc;

static constexpr const auto board_w = 50.0_mm;
static constexpr const auto board_h = 50.0_mm;
  
// MC14553 - 3-digit BCD counter
const auto ic1 = dip<16>{{ 12.0_mm, 7.0_mm}};
// MC14511 - BCD to 7 segments
const auto ic2 = dip<16>{{-6.0_mm, 7.0_mm}, {}, dip_pin_tc_1down};

enum hdr_pins {
  h_nil = 0, // not a real pin
  h_overflow,
  h_reset,
  h_clock,
  h_disable,
  h_strobe,
  h_v_plus,
  h_v_minus,
};
const auto hdr = header<7> {{
  -0.4_in + board_w / 2.0,
  -0.1_in + board_h / 2.0,
}};

// 100k
const auto r1 = r0603(ic1.pin(12).plus(-3.0_mm, 0));
const auto r2 = r0603(ic1.pin(11).plus(-3.0_mm, 0));
const auto r3 = r0603(ic1.pin(10).plus(-3.0_mm, 0));
const auto r4 = r0603(ic1.pin(13).plus(-3.0_mm, 0));
// 68
const auto r5  = r0603(ic2.pin(13).plus(-3.0_mm, 0));
const auto r6  = r0603(ic2.pin(12).plus(-3.0_mm, 0));
const auto r7  = r0603(ic2.pin(11).plus(-3.0_mm, 0));
const auto r8  = r0603(ic2.pin(10).plus(-3.0_mm, 0));
const auto r9  = r0603(ic2.pin( 9).plus(-3.0_mm, 0));
const auto r10 = r0603(ic2.pin(15).plus(-3.0_mm, 0));
const auto r11 = r0603(ic2.pin(14).plus(-3.0_mm, 0));

const auto vr8  = via { r8.pin(2).plus(-1.3_mm, 0) };
const auto vr9  = via { r9.pin(2).plus(-1.3_mm, 0) };
const auto vr11 = via { r11.pin(2).plus(-1.3_mm, 0) };

// 1nF
const auto c1 = r0603(ic1.pin(2).plus(3.0_mm, 0));
// 10nF
const auto c2 = r0603(hdr.pin(h_v_minus).plus(-0.5_mm, -2.9_mm));

// BJT NPN
const auto q1 = sot23({ 4.0_mm, 17.0_mm });
const auto q2 = sot23({ 4.0_mm, 13.0_mm });
const auto q3 = sot23({ 4.0_mm,  9.0_mm });

const auto vq1 = via { q1.pin(sot23::c).plus(-1.3_mm, 0) };
const auto vq2 = via { q2.pin(sot23::c).plus(-1.3_mm, 0) };
const auto vq3 = via { q3.pin(sot23::c).plus(-1.3_mm, 0) };

// 7-digit displays
const auto msd = dip<14>{{-12.0_mm, -board_h/2 + 10.0_mm}};
const auto nsd = dip<14>{{  0.0_mm, -board_h/2 + 10.0_mm}};
const auto lsd = dip<14>{{ 12.0_mm, -board_h/2 + 10.0_mm}};

struct compos : generic_layers<
  r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11,
  c1, c2,
  q1, q2, q3,
  msd, nsd, lsd,
  ic1, ic2,
  hdr,
  vr8, vr9, vr11,
  vq1, vq2, vq3>
{};

void link_digits(turtle & t, unsigned pin, cnc::point d0, d::inch d1x) {
  t.move(msd.pin(pin));
  t.draw(msd.pin(pin).plus(d0));
  t.draw_x(d1x);
  t.draw(nsd.pin(pin));
  t.draw(nsd.pin(pin).plus(d0));
  t.draw_x(d1x);
  t.draw(lsd.pin(pin));
}
void link_digit_gnd(turtle & t, const auto & d) {
  t.move(d.pin(12));
  t.draw(d.pin(4));
}

void top_nets(cnc::pen & p) {
  turtle t { &p };

  link_digits(t,  1, { 2.2_mm,  2.2_mm }, 7.0_mm);
  link_digits(t,  2, { 4.0_mm,  4.0_mm }, 4.0_mm);
  link_digits(t, 14, { 3.8_mm, -3.8_mm }, 4.5_mm);
  link_digits(t, 13, { 3.8_mm, -3.8_mm }, 4.5_mm);

  link_digits(t, 8,  { 1.25_mm, -1.25_mm }, 8.0_mm);
  link_digits(t, 7,  { 1.25_mm,  1.25_mm }, 8.0_mm);
  link_digits(t, 6,  { 1.25_mm,  1.25_mm }, 8.0_mm);

  link_digit_gnd(t, msd);
  link_digit_gnd(t, nsd);
  link_digit_gnd(t, lsd);

  t.move(ic1.pin(15));
  t.draw(q1.pin(sot23::b));

  t.move(ic1.pin(1));
  t.draw(q2.pin(sot23::b));

  t.move(ic1.pin(2));
  t.draw(q3.pin(sot23::b));

  bus hb0 { ic1.pin(15).plus(1.5_mm, 0) };
  bus hb1 = bus { hdr.pin(h_strobe).plus(-1.5_mm, -3.0_mm) }.flip();

  t.move(hdr.pin(h_overflow));
  t.draw(hb1.pin(0));
  t.draw(hb0.pin(0));
  t.draw_ld(ic1.pin(14));

  t.move(hdr.pin(h_reset));
  t.draw(hb1.pin(1));
  t.draw(hb0.pin(1));
  t.draw_ld(ic1.pin(13)); t.draw(r4.pin(1)); 

  t.move(hdr.pin(h_clock));
  t.draw(hb1.pin(2));
  t.draw(hb0.pin(2));
  t.draw_ld(ic1.pin(12)); t.draw(r1.pin(1)); 

  t.move(hdr.pin(h_disable));
  t.draw(hb0.pin(3));
  t.draw_ld(ic1.pin(11)); t.draw(r2.pin(1)); 

  t.move(hdr.pin(h_strobe));
  t.draw(hb0.pin(4));
  t.draw_ld(ic1.pin(10)); t.draw(r3.pin(1)); 

  t.move(hdr.pin(h_v_plus));
  t.draw(c2.pin(2));

  t.move(ic1.pin(3));
  t.draw(c1.pin(2));

  t.move(ic1.pin(4));
  t.draw(c1.pin(1));

  t.move(ic1.pin(9));
  t.draw(ic2.pin(1).plus(0, -2.0_mm));
  t.draw(ic2.pin(6).plus(-1.8_mm, 0));
  t.draw_ld(ic2.pin(7));

  t.move(ic1.pin(7));
  t.draw(ic2.pin(1));

  t.move(ic1.pin(6));
  t.draw(ic2.pin(2));

  t.move(ic1.pin(5));
  t.draw_ld(ic2.pin(6));

  t.move(ic2.pin(13));
  t.draw(r5.pin(1));
  t.move(r5.pin(2));
  t.draw_x(-2.0_mm);
  t.draw(msd.pin(2).plus(-1.5_mm, -0.3_mm));
  t.draw(msd.pin(2).plus(0, -1.3_mm));
  t.draw_ld(msd.pin(14));

  t.move(ic2.pin(12));
  t.draw(r6.pin(1));
  t.move(r6.pin(2));
  t.draw_x(-2.0_mm);
  t.draw(msd.pin(3).plus(-2.5_mm, -0.3_mm));
  t.draw(msd.pin(3).plus(0, -1.3_mm));
  t.draw_ld(msd.pin(13));

  t.move(ic2.pin(11));
  t.draw(r7.pin(1));
  t.move(r7.pin(2));
  t.draw_x(-2.0_mm);
  t.draw(msd.pin(7).plus(-3.5_mm, -0.3_mm));
  t.draw(msd.pin(7).plus(0, -1.3_mm));
  t.draw_ld(msd.pin(8));

  t.move(ic2.pin(10));
  t.draw(r8.pin(1));
  t.move(r8.pin(2));
  t.draw(vr8);

  t.move(ic2.pin(9));
  t.draw(r9.pin(1));
  t.move(r9.pin(2));
  t.draw(vr9);

  t.move(ic2.pin(15));
  t.draw(r10.pin(1));
  t.move(r10.pin(2));
  t.draw_ld(msd.pin(1));

  t.move(ic2.pin(14));
  t.draw(r11.pin(1));
  t.move(r11.pin(2));
  t.draw(vr11);

  t.move(q1.pin(sot23::c)); t.draw(vq1);
  t.move(q2.pin(sot23::c)); t.draw(vq2);
  t.move(q3.pin(sot23::c)); t.draw(vq3);
}
void bottom_nets(cnc::pen & p) {
  turtle t { &p };
  t.move(vr11);
  t.draw(vr11.plus(0.5_mm, -0.5_mm));
  t.draw_ld(msd.pin(2));

  t.move(vr8);
  t.draw(vr8.plus(1.5_mm, -1.5_mm));
  t.draw_ld(msd.pin(7));

  t.move(vr9);
  t.draw(vr9.plus(-4.0_mm, -4.0_mm));
  t.draw_ld(msd.pin(6));

  t.move(vq1);
  t.draw(ic2.pin(1).plus(2.0_mm, 0));
  t.draw(ic2.pin(1).plus(-2.0_mm, -2.0_mm));
  t.draw(msd.pin(13).plus(1.5_mm, 0));
  t.draw_ld(msd.pin(12));

  t.move(vq2);
  t.draw(ic2.pin(1).plus(3.0_mm, 0));
  t.draw_ld(nsd.pin(4));

  t.move(vq3);
  t.draw(ic1.pin(8).plus(-2.0_mm, 0));
  t.draw_ld(lsd.pin(4));
}

void top_copper(cnc::pen & p) {
  compos::penny(p, l::top_copper {});

  p.aperture(10.0_mil);
  top_nets(p);

  thermal(p, q1, sot23::e);
  thermal(p, q2, sot23::e);
  thermal(p, q3, sot23::e);
  thermal(p, ic1, 8);
  thermal(p, ic2, 8);
  thermal(p, ic2, 5);
  thermal(p, r1, 2);
  thermal(p, r2, 2);
  thermal(p, r3, 2);
  thermal(p, r4, 2);
  thermal(p, c2, 1);
  thermal(p, hdr, 7);
}
void top_copper_margin(cnc::pen & p) {
  compos::penny(p, l::top_copper_margin {});

  p.aperture(10.0_mil + def_copper_margin);
  top_nets(p);
}
void top_silk(cnc::pen & p) {
  compos::penny(p, l::silk {});
}
void top_mask(cnc::pen & p) {
  compos::penny(p, l::top_mask {});
}

void bottom_copper(cnc::pen & p) {
  compos::penny(p, l::bottom_copper {});

  p.aperture(10.0_mil);
  bottom_nets(p);

  thermal(p, ic1, 16);
  thermal(p, ic2, 16);
  thermal(p, ic2, 4);
  thermal(p, ic2, 3);
  thermal(p, hdr, 6);
}
void bottom_copper_margin(cnc::pen & p) {
  compos::penny(p, l::bottom_copper_margin {});

  p.aperture(10.0_mil + def_copper_margin);
  bottom_nets(p);
}
void bottom_mask(cnc::pen & p) {
  compos::penny(p, l::bottom_mask {});
}

void holes(cnc::pen & p) {
  compos::penny(p, l::holes {});
}

void border_margin(cnc::pen & p) {
  box(p, 0, 0, board_w, board_h, 25.0_mil);
}
void border(cnc::pen & p) {
  box(p, 0, 0, board_w, board_h, 10.0_mil);
}

void plane(cnc::fanner & p) {
  static constexpr const auto m = 1.0_mm;
  static constexpr const auto w = board_w - m;
  static constexpr const auto h = board_h - m;
  cnc::utils::box(p, 0, 0, w, h);
}

#ifdef LECO_TARGET_WINDOWS
#define A __declspec(dllexport)
#else
#define A
#endif

extern "C" A void draw(cnc::builder * b, cnc::grb_layer l) {
  switch (l) {
    case cnc::gl_top_copper:
      b->add_region(plane, red);
      b->add_lines(top_copper_margin, black);
      b->add_lines(top_copper, red);
      b->add_lines(holes, black);
      break;
    case cnc::gl_top_mask:
      b->add_lines(top_mask, green);
      break;
    case cnc::gl_top_silk:
      b->add_lines(top_silk, white);
      break;
    case cnc::gl_bot_copper:
      b->add_region(plane, blue);
      b->add_lines(bottom_copper_margin, black);
      b->add_lines(bottom_copper, blue);
      b->add_lines(holes, black);
      break;
    case cnc::gl_bot_mask:
      b->add_lines(bottom_mask, dark_green);
      break;
    case cnc::gl_border:
      b->add_lines(border_margin, black);
      b->add_lines(border, purple);
      break;
    case cnc::gl_drill_holes:
      b->add_lines(holes, white);
      break;
    default: break;
  }
}

extern "C" A void cpl(cpl::builder * b) {
  b->part({ "R1",  r1,  true,  0 });
  b->part({ "R2",  r2,  true,  0 });
  b->part({ "R3",  r3,  true,  0 });
  b->part({ "R4",  r4,  true,  0 });
  b->part({ "R5",  r5,  true,  0 });
  b->part({ "R6",  r6,  true,  0 });
  b->part({ "R7",  r7,  true,  0 });
  b->part({ "R8",  r8,  true,  0 });
  b->part({ "R9",  r9,  true,  0 });
  b->part({ "R10", r10, true,  0 });
  b->part({ "R11", r11, true,  0 });
  b->part({ "C1",  c1,  true,  0 });
  b->part({ "C2",  c2,  true,  0 });
  b->part({ "Q1",  q1,  true, 90 });
  b->part({ "Q2",  q2,  true, 90 });
  b->part({ "Q3",  q3,  true, 90 });

  b->bom({ "100k", "R1-4", "0603_R", "C25803" });
  b->bom({ "56", "R5-11", "0603_R", "C23206" });
  b->bom({ "1nF", "C1", "0603_C", "C1588" });
  b->bom({ "10nF", "C2", "0603_C", "C57112" });
  b->bom({ "NPN", "Q1-3", "SOT-23", "C2146" });
}
