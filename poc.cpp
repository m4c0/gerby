#pragma leco app
import casein;
import dl;
import hai;
import gerby;
import gerbyv;

static hai::uptr<dl::lib> g_lib;
static auto sym() {
  g_lib = dl::open("poc-555-smd");
  return g_lib->fn<void(gerby::cnc::builder *, gerby::cnc::grb_layer)>("draw");
}

static auto g_fn = sym();

extern "C" void casein_handle(const casein::event &e) {
  static gerby::thread t{[](auto b, auto l) { g_fn(b, l); }};
  t.handle(e);

  if (e.type() == casein::KEY_DOWN) {
    if (*e.as<casein::events::key_down>() == 'r') {
      g_fn = sym();
    }
  }
}
