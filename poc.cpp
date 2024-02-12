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
  float diam;
  float rnd;
};
constexpr const auto v_count = 8;

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

    v[0] = {{-1.f, -1.f}, 0};
    v[1] = {{-1.f, 1.f}, 0};
    v[2] = {{0.f, -1.f}, 0};
    v[3] = {{0.f, 1.f}, 0};
    v[4] = {{0.f, -1.f}, 1};
    v[5] = {{0.f, 1.f}, 1};
    v[6] = {{1.f, -1.f}, 1};
    v[7] = {{1.f, 1.f}, 1};
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
  float m_round{1};
  dotz::vec2 m_smear{};
  unsigned m_count{};

public:
  explicit pen(vee::device_memory::type mem)
      : m_map{mem}
      , m_buf{static_cast<inst *>(*m_map)} {}

  [[nodiscard]] constexpr auto count() const noexcept { return m_count; }

  constexpr void aperture(float d) {
    m_d = d;
    m_smear = {};
    m_round = 1.0;
  }
  constexpr void aperture(float w, float h, bool round) {
    if (w > h) {
      m_d = h;
      m_smear = {w - h, 0.0};
    } else {
      m_d = w;
      m_smear = {0.0, h - w};
    }
    m_smear = m_smear / 2.0f;
    m_round = round;
  }

  void draw(float x, float y) {
    dotz::vec2 np{x, y};
    m_buf[m_count++] = {m_p, np, m_d, m_round};
    m_p = {x, y};
  }
  void draw_x(float x) { draw(x, m_p.y); }
  void draw_y(float y) { draw(m_p.x, y); }

  void flash(float x, float y) {
    m_p = {x, y};
    m_buf[m_count++] = {m_p - m_smear, m_p + m_smear, m_d, m_round};
  }
  void flash_x(float x) { flash(x, m_p.y); }
  void flash_y(float y) { flash(m_p.x, y); }

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

      unsigned i_count{};
      {
        auto p = is.pen();

        p.aperture(0.1); // D10C,0.1
        p.move(0, 2.5);
        p.draw(0, 0);
        p.draw(2.5, 0);
        p.move(10, 10);
        p.draw_x(15);
        p.draw(20, 15);
        p.move_x(25);
        p.draw_y(10);

        p.aperture(0.6); // D11C,0.6
        p.flash(10, 10);
        p.flash_x(20);
        p.flash_x(25);
        p.flash_y(15);
        p.flash_x(20);

        // we simulate rectangles/obround with a small segment

        // TODO: rectangle aperture
        p.aperture(0.6, 0.6, false); // D12R,0.6x0.6
        p.flash(10, 15);

        p.aperture(0.4, 1.0, false); // D13R,0.4x1.00
        p.flash(30, 15);

        p.aperture(1.0, 0.4, false); // D14R,1.00X0.4
        p.flash_y(12.5);

        p.aperture(0.4, 1.0, true); // D15O0.4X1.00
        p.flash_y(10);

        p.aperture(0.1); // D10C,0.1
        p.move(37.5, 10);
        // p.arc(37.5, 10, 25.0, 0);

        p.aperture(1);
        // p.aperture(1, 3); // D16P,1.00X3 "triangle"
        p.flash(34, 10);
        p.flash(35, 9);

        // TODO: add support for regions
        // Most probably, take note of the count here and draw lines over two
        // calls and regions as a different pipeline?

        // TODO: "clear" polarity

        p.aperture(0.1); // D10
        p.move(15, 28.75);
        p.draw_x(20);

        p.aperture(0.6); // D11
        p.flash(15, 28.75);
        p.flash_x(20);

        // TODO: "thermal" macro

        i_count = p.count();
      }
      is.run_once();

      auto gp = vee::create_graphics_pipeline({
          .pipeline_layout = *pl,
          .render_pass = dq.render_pass(),
          .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
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
              vee::vertex_attribute_float(1, 2 * sizeof(dotz::vec2) +
                                                 sizeof(float)),
          },
      });

      extent_loop(dq, sw, [&] {
        upc pc{
            .center = {25.f / 2.f, 15.f / 1.f},
            .scale = 20.f,
            .aspect = sw.aspect(),
        };

        sw.queue_one_time_submit(dq, [&](auto pcb) {
          auto scb = sw.cmd_render_pass({
              .command_buffer = *pcb,
              .clear_color = {},
          });
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
