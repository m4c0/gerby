#pragma leco add_shader "gerby.vert"
#pragma leco add_shader "gerby.frag"

export module gerby;

import casein;
import dotz;
import hai;
import sith;
import vee;
import voo;

namespace gerby {
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

struct rvtx {
  dotz::vec2 pos;
  dotz::vec2 pad{};
};

struct upc {
  dotz::vec4 colour;
  dotz::vec2 center;
  float scale;
  float aspect;
};

class layer {
  dotz::vec4 m_colour;

protected:
  [[nodiscard]] constexpr auto colour() const noexcept { return m_colour; }

public:
  explicit layer(dotz::vec4 c) : m_colour{c} {}

  virtual ~layer() = default;

  virtual void cmd_draw(vee::command_buffer cb, upc *pc) = 0;
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

class minmax {
  dotz::vec2 m_min{1e20f, 1e20f};
  dotz::vec2 m_max{-1e20f, -1e20f};

public:
  void enclose(dotz::vec2 p) {
    m_min = dotz::min(p, m_min);
    m_max = dotz::max(p, m_max);
  }

  [[nodiscard]] dotz::vec2 center() const noexcept {
    return (m_max - m_min) / 2.0;
  }
  [[nodiscard]] float scale() const noexcept {
    auto c = center();
    auto m = c.x > c.y ? c.x : c.y;
    return m * 1.20;
  }
};

export class pen {
  voo::mapmem m_map;
  inst *m_buf;
  dotz::vec2 m_p{};
  float m_d{};
  float m_round{1};
  dotz::vec2 m_smear{};
  unsigned m_count{};
  minmax *m_minmax;

public:
  explicit pen(vee::device_memory::type mem, minmax *mm)
      : m_map{mem}
      , m_buf{static_cast<inst *>(*m_map)}
      , m_minmax{mm} {}

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
    m_minmax->enclose(m_p);
    m_minmax->enclose(np);
    m_buf[m_count++] = {m_p, np, m_d, m_round};
    m_p = {x, y};
  }
  void draw_x(float x) { draw(x, m_p.y); }
  void draw_y(float y) { draw(m_p.x, y); }

  void flash(float x, float y) {
    m_p = {x, y};
    m_minmax->enclose(m_p);
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

  [[nodiscard]] auto pen(minmax *mm) noexcept {
    return gerby::pen{m_is.host_memory(), mm};
  }

  using update_thread::run_once;
};

export class fanner {
  voo::mapmem m_m;
  rvtx *m_v;
  unsigned m_count{};
  minmax *m_minmax{};
  dotz::vec2 m_fan_ref{};

public:
  explicit fanner(vee::device_memory::type m, minmax *mm)
      : m_m{m}
      , m_v{static_cast<rvtx *>(*m_m)}
      , m_minmax{mm} {}

  [[nodiscard]] constexpr auto count() const noexcept { return m_count; }

  void move(float x, float y) {
    m_fan_ref = {x, y};
    m_v[m_count++] = {m_fan_ref};
    m_v[m_count++] = {m_fan_ref};
  }

  void draw(float x, float y) {
    m_v[m_count++] = {m_fan_ref};
    m_v[m_count++] = {{x, y}};
    m_minmax->enclose({x, y});
  }
  void draw_x(float x) { draw(x, m_v[m_count - 1].pos.y); }
  void draw_y(float y) { draw(m_v[m_count - 1].pos.x, y); }
};

class rvertices : voo::update_thread {
  static constexpr const auto max_vtx = 10240;

  voo::h2l_buffer m_is;

  void build_cmd_buf(vee::command_buffer cb) override {
    voo::cmd_buf_one_time_submit pcb{cb};
    m_is.setup_copy(*pcb);
  }

public:
  explicit rvertices(voo::device_and_queue *dq)
      : update_thread{dq}
      , m_is{*dq, max_vtx * sizeof(rvtx)} {}

  [[nodiscard]] constexpr auto local_buffer() const noexcept {
    return m_is.local_buffer();
  }

  [[nodiscard]] auto fanner(minmax *mm) const noexcept {
    return gerby::fanner{m_is.host_memory(), mm};
  }

  using update_thread::run_once;
};

class lines : public layer {
  vee::pipeline_layout m_pl =
      vee::create_pipeline_layout({vee::vertex_push_constant_range<upc>()});
  vee::gr_pipeline m_gp;

  vertices m_vs;
  instances m_is;
  unsigned m_i_count{};

  void update(void (*load)(pen &), minmax *mm) {
    {
      auto p = m_is.pen(mm);
      load(p);
      m_i_count = p.count();
    }
    m_is.run_once();
  }

public:
  explicit lines(voo::device_and_queue *dq, dotz::vec4 colour)
      : layer{colour}
      , m_gp{vee::create_graphics_pipeline({
            .pipeline_layout = *m_pl,
            .render_pass = dq->render_pass(),
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            .back_face_cull = false,
            .depth_test = false,
            .shaders{
                voo::shader("gerby.vert.spv").pipeline_vert_stage(),
                voo::shader("gerby.frag.spv").pipeline_frag_stage(),
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
        })}
      , m_vs{dq}
      , m_is{dq} {}

  void cmd_draw(vee::command_buffer cb, upc *pc) override {
    pc->colour = colour();

    vee::cmd_bind_gr_pipeline(cb, *m_gp);
    vee::cmd_push_vertex_constants(cb, *m_pl, pc);
    vee::cmd_bind_vertex_buffers(cb, 0, m_vs.local_buffer());
    vee::cmd_bind_vertex_buffers(cb, 1, m_is.local_buffer());
    vee::cmd_draw(cb, v_count, m_i_count);
  }

  static auto create(voo::device_and_queue *dq, void (*load)(pen &),
                     dotz::vec4 colour, minmax *mm) {
    auto *res = new lines{dq, colour};
    res->update(load, mm);
    return hai::uptr<layer>{res};
  }
};

class region : public layer {
  vee::pipeline_layout m_pl =
      vee::create_pipeline_layout({vee::vertex_push_constant_range<upc>()});
  vee::gr_pipeline m_gp;

  rvertices m_vs;
  unsigned m_count{};

  void update(void (*load)(fanner &), minmax *mm) {
    {
      auto p = m_vs.fanner(mm);
      load(p);
      m_count = p.count();
    }
    m_vs.run_once();
  }

public:
  explicit region(voo::device_and_queue *dq, dotz::vec4 colour)
      : layer{colour}
      , m_gp{vee::create_graphics_pipeline({
            .pipeline_layout = *m_pl,
            .render_pass = dq->render_pass(),
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            .back_face_cull = false,
            .depth_test = false,
            .shaders{
                voo::shader("gerby.vert.spv").pipeline_vert_stage(),
                voo::shader("gerby.frag.spv").pipeline_frag_stage(),
            },
            .bindings{
                vee::vertex_input_bind(sizeof(rvtx)),
            },
            .attributes{
                vee::vertex_attribute_vec2(0, sizeof(dotz::vec2)),
                vee::vertex_attribute_vec2(0, 0),
                vee::vertex_attribute_vec2(0, 0),
                vee::vertex_attribute_float(0, sizeof(dotz::vec2)),
                vee::vertex_attribute_float(0, sizeof(dotz::vec2)),
                vee::vertex_attribute_float(0, sizeof(dotz::vec2)),
            },
        })}
      , m_vs{dq} {}

  void cmd_draw(vee::command_buffer cb, upc *pc) override {
    pc->colour = colour();

    vee::cmd_bind_gr_pipeline(cb, *m_gp);
    vee::cmd_push_vertex_constants(cb, *m_pl, pc);
    vee::cmd_bind_vertex_buffers(cb, 0, m_vs.local_buffer());
    vee::cmd_draw(cb, m_count);
  }

  static auto create(voo::device_and_queue *dq, void (*load)(fanner &),
                     dotz::vec4 colour, minmax *mm) {
    auto res = new region{dq, colour};
    res->update(load, mm);
    return hai::uptr<layer>{res};
  }
};

export class builder {
  static constexpr const auto max_layers = 1024;

  voo::device_and_queue *m_dq;
  hai::varray<hai::uptr<gerby::layer>> m_layers{max_layers};
  minmax m_mm{};

public:
  explicit builder(voo::device_and_queue *dq) : m_dq{dq} {}

  void add_lines(auto fn, dotz::vec4 colour) {
    m_layers.push_back(lines::create(m_dq, fn, colour, &m_mm));
  }
  void add_region(auto fn, dotz::vec4 colour) {
    m_layers.push_back(region::create(m_dq, fn, colour, &m_mm));
  }

  [[nodiscard]] constexpr auto &layers() noexcept { return m_layers; }
  [[nodiscard]] constexpr auto &minmax() noexcept { return m_mm; }
};

export class thread : public voo::casein_thread {
  void (*m_lb)(builder *);

public:
  explicit constexpr thread(decltype(m_lb) l) : m_lb{l} {}

  void run() override {
    voo::device_and_queue dq{"gerby", native_ptr()};

    builder b{&dq};
    m_lb(&b);

    auto mm = b.minmax();

    while (!interrupted()) {
      voo::swapchain_and_stuff sw{dq};

      extent_loop(dq, sw, [&] {
        upc pc{
            .center = mm.center(),
            .scale = mm.scale(),
            .aspect = sw.aspect(),
        };

        sw.queue_one_time_submit(dq, [&](auto pcb) {
          auto scb = sw.cmd_render_pass({
              .command_buffer = *pcb,
              .clear_color = {},
          });
          for (auto &l : b.layers()) {
            l->cmd_draw(*scb, &pc);
          }
        });
      });
    }
  }
};
} // namespace gerby