#pragma leco app
import casein;
import gerbyv;

extern "C" void casein_handle(const casein::event &e) {
  static gerby::thread t{"poc-555-smd"};
  t.handle(e);
}
