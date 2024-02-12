#pragma leco app
#pragma leco add_shader "poc.vert"
#pragma leco add_shader "poc.frag"

import casein;
import dotz;
import sith;
import vee;
import voo;

struct vtx {
  dotz::vec2 delta;
  float w;
};
struct inst {
  dotz::vec2 a;
  dotz::vec2 b;
  float r;
};
constexpr const auto q_count = 3;
constexpr const auto t_count = 2 * q_count;
constexpr const auto v_count = 3 * t_count;

constexpr const auto i_count = 8;

struct upc {
  dotz::vec2 center;
  float scale;
  float aspect;
};

class vertices : voo::update_thread {
  voo::h2l_buffer m_vs;

  void build_cmd_buf(vee::command_buffer cb) override {
    voo::cmd_buf_one_time_submit pcb{cb};
    m_vs.setup_copy(*pcb);
  }

  void load_buffer() {
    voo::mapmem m{m_vs.host_memory()};

    auto *v = static_cast<vtx *>(*m);

    // TODO: use a triangle strip

    v[0] = {{-1.f, -1.f}, 0};
    v[1] = {{-1.f, 1.f}, 0};
    v[2] = {{0.f, -1.f}, 0};

    v[3] = v[2];
    v[4] = v[1];
    v[5] = {{0.f, 1.f}, 0};

    v[6] = v[2];
    v[7] = v[5];
    v[8] = {{0.f, -1.f}, 1};

    v[9] = v[8];
    v[10] = v[5];
    v[11] = {{0.f, 1.f}, 1};

    v[12] = v[8];
    v[13] = v[11];
    v[14] = {{1.f, 1.f}, 1};

    v[15] = v[8];
    v[16] = v[14];
    v[17] = {{1.f, -1.f}, 1};
  }

public:
  explicit vertices(voo::device_and_queue *dq)
      : update_thread{dq}
      , m_vs{*dq, v_count * sizeof(vtx)} {
    load_buffer();
    run_once();
  }

  [[nodiscard]] constexpr auto local_buffer() const noexcept {
    return m_vs.local_buffer();
  }
};

class pen {
  voo::mapmem m_map;
  inst *m_buf;
  dotz::vec2 m_p{};
  float m_d{};

public:
  explicit pen(vee::device_memory::type mem)
      : m_map{mem}
      , m_buf{static_cast<inst *>(*m_map)} {}

  constexpr void aperture(float d) { m_d = d; }

  void draw(float x, float y) {
    dotz::vec2 np{x, y};
    *m_buf++ = {m_p, np, m_d};
    m_p = {x, y};
  }
  void draw_x(float x) { draw(x, m_p.y); }
  void draw_y(float y) { draw(m_p.x, y); }

  void move(float x, float y) { m_p = {x, y}; }
  void move_x(float x) { move(x, m_p.y); }
  void move_y(float y) { move(m_p.x, y); }
};

class instances : voo::update_thread {
  static constexpr const auto max_insts = 10240;

  voo::h2l_buffer m_is;

  void build_cmd_buf(vee::command_buffer cb) override {
    voo::cmd_buf_one_time_submit pcb{cb};
    m_is.setup_copy(*pcb);
  }

public:
  explicit instances(voo::device_and_queue *dq)
      : update_thread{dq}
      , m_is{*dq, max_insts * sizeof(vtx)} {}

  [[nodiscard]] constexpr auto local_buffer() const noexcept {
    return m_is.local_buffer();
  }

  [[nodiscard]] auto pen() noexcept { return ::pen{m_is.host_memory()}; }

  using update_thread::run_once;
};

class thread : public voo::casein_thread {
public:
  void run() override {
    voo::device_and_queue dq{"gerby", native_ptr()};

    vee::pipeline_layout pl =
        vee::create_pipeline_layout({vee::vertex_push_constant_range<upc>()});

    vertices vs{&dq};
    instances is{&dq};

    // TODO: fix validation issues while resizing
    while (!interrupted()) {
      voo::swapchain_and_stuff sw{dq};

      {
        auto p = is.pen();
        p.aperture(0.01);

        p.move(0, 0);
        p.draw(5, 0);
        p.draw_y(5);
        p.draw_x(0);
        p.draw_y(0);

        p.move_x(6);
        p.draw_x(11);
        p.draw_y(5);
        p.draw_x(6);
        p.draw_y(0);
      }
      is.run_once();

      auto gp = vee::create_graphics_pipeline({
          .pipeline_layout = *pl,
          .render_pass = dq.render_pass(),
          .back_face_cull = false,
          .depth_test = false,
          .shaders{
              voo::shader("poc.vert.spv").pipeline_vert_stage(),
              voo::shader("poc.frag.spv").pipeline_frag_stage(),
          },
          .bindings{
              vee::vertex_input_bind(sizeof(vtx)),
              vee::vertex_input_bind_per_instance(sizeof(inst)),
          },
          .attributes{
              vee::vertex_attribute_vec2(0, 0),
              vee::vertex_attribute_vec2(1, 0),
              vee::vertex_attribute_vec2(1, sizeof(dotz::vec2)),
              vee::vertex_attribute_float(0, sizeof(dotz::vec2)),
              vee::vertex_attribute_float(1, 2 * sizeof(dotz::vec2)),
          },
      });

      extent_loop(dq, sw, [&] {
        upc pc{
            .center = {5.5f, 2.5f},
            .scale = 4.0f,
            .aspect = sw.aspect(),
        };

        sw.queue_one_time_submit(dq, [&](auto pcb) {
          auto scb = sw.cmd_render_pass(pcb);
          vee::cmd_bind_gr_pipeline(*scb, *gp);
          vee::cmd_push_vertex_constants(*scb, *pl, &pc);
          vee::cmd_bind_vertex_buffers(*scb, 0, vs.local_buffer());
          vee::cmd_bind_vertex_buffers(*scb, 1, is.local_buffer());
          vee::cmd_draw(*scb, v_count, i_count);
        });
      });
    }
  }
};

extern "C" void casein_handle(const casein::event &e) {
  static thread t{};
  t.handle(e);
}
