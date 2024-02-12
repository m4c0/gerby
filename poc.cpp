#pragma leco app
#pragma leco add_shader "poc.vert"
#pragma leco add_shader "poc.frag"

import casein;
import dotz;
import sith;
import vee;
import voo;

struct vtx {
  dotz::vec2 anchor;
  dotz::vec2 delta;
};
constexpr const auto q_count = 3;
constexpr const auto t_count = 2 * q_count;
constexpr const auto v_count = 3 * t_count;

struct upc {
  float aspect;
};

class thread : public voo::casein_thread {
public:
  void run() override {
    voo::device_and_queue dq{"gerby", native_ptr()};

    vee::pipeline_layout pl =
        vee::create_pipeline_layout({vee::vertex_push_constant_range<upc>()});

    voo::h2l_buffer vs{dq, v_count * sizeof(vtx)};

    // TODO: fix validation issues while resizing
    while (!interrupted()) {
      voo::swapchain_and_stuff sw{dq};

      {
        voo::mapmem m{vs.host_memory()};

        dotz::vec2 p0{-0.5f, -0.3f};
        dotz::vec2 p1{+0.6f, +0.2f};

        auto *v = static_cast<vtx *>(*m);

        // TODO: use a triangle strip

        v[0] = {p0, {-1.f, -1.f}};
        v[1] = {p0, {-1.f, 1.f}};
        v[2] = {p0, {0.f, -1.f}};

        v[3] = v[2];
        v[4] = v[1];
        v[5] = {p0, {0.f, 1.f}};

        v[6] = v[2];
        v[7] = v[5];
        v[8] = {p1, {0.f, -1.f}};

        v[9] = v[8];
        v[10] = v[5];
        v[11] = {p1, {0.f, 1.f}};

        v[12] = v[8];
        v[13] = v[11];
        v[14] = {p1, {1.f, 1.f}};

        v[15] = v[8];
        v[16] = v[14];
        v[17] = {p1, {1.f, -1.f}};
      }

      auto gp = vee::create_graphics_pipeline({
          .pipeline_layout = *pl,
          .render_pass = dq.render_pass(),
          .shaders{
              voo::shader("poc.vert.spv").pipeline_vert_stage(),
              voo::shader("poc.frag.spv").pipeline_frag_stage(),
          },
          .bindings{
              vee::vertex_input_bind(sizeof(vtx)),
          },
          .attributes{
              vee::vertex_attribute_vec2(0, 0),
              vee::vertex_attribute_vec2(0, sizeof(dotz::vec2)),
          },
      });

      extent_loop(dq, sw, [&] {
        upc pc{.aspect = sw.aspect()};

        sw.queue_one_time_submit(dq, [&](auto pcb) {
          vs.setup_copy(*pcb);

          auto scb = sw.cmd_render_pass(pcb);
          vee::cmd_bind_gr_pipeline(*scb, *gp);
          vee::cmd_push_vertex_constants(*scb, *pl, &pc);
          vee::cmd_bind_vertex_buffers(*scb, 0, vs.local_buffer());
          vee::cmd_draw(*scb, v_count);
        });
      });
    }
  }
};

extern "C" void casein_handle(const casein::event &e) {
  static thread t{};
  t.handle(e);
}
