export module gerbyv;
import :buffers;
import :pipeline;
import casein;
import dl;
import dotz;
import gerby;
import hai;
import silog;
import sith;
import vapp;
import vee;
import voo;

namespace gerby {
  static constexpr const auto max_layers = 1024;
  static constexpr const auto max_insts = 10240;
  static constexpr const auto max_rvtx = 10240;

  struct objects {
    voo::device_and_queue * dq;

    pipeline lines_ppl = line_pipeline(dq);
    pipeline regions_ppl = region_pipeline(dq);

    vertices vs { dq };

    hai::array<voo::bound_buffer> is { max_layers };
    hai::array<voo::bound_buffer> rs { max_layers };
  };

class layer {
  dotz::vec4 m_colour;
  unsigned m_idx;

protected:
  [[nodiscard]] constexpr auto colour() const noexcept { return m_colour; }
  [[nodiscard]] constexpr auto index() const noexcept { return m_idx; }

public:
  explicit layer(dotz::vec4 c, unsigned i) : m_colour{c}, m_idx{i} {}

  virtual ~layer() = default;

  virtual void cmd_draw(vee::command_buffer cb, upc *pc) = 0;
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
    return (m_max + m_min) / 2.0;
  }
  [[nodiscard]] float scale() const noexcept {
    auto [sx, sy] = casein::window_size;
    auto sa = sy / sx;

    auto c = (m_max - m_min) / 2.0;
    auto m = sa * c.x > c.y ? sa * c.x : c.y;
    return m * 1.20;
  }
};

export class pen : public cnc::pen {
  voo::mapmem m_map;
  inst *m_buf;
  dotz::vec2 m_p{};
  unsigned m_count{};
  minmax *m_minmax;

public:
  explicit pen(vee::device_memory::type mem, minmax *mm)
      : m_map{mem}
      , m_buf{static_cast<inst *>(*m_map)}
      , m_minmax{mm} {}

  [[nodiscard]] constexpr auto count() const noexcept { return m_count; }

  void draw(d::inch x, d::inch y) override {
    auto d = aperture().diameter();
    auto r = aperture().roundness();

    dotz::vec2 np{x.as_float(), y.as_float()};
    m_minmax->enclose(m_p);
    m_minmax->enclose(np);
    m_buf[m_count++] = {m_p, np, d, r};
    m_p = np;
  }
  void draw_x(d::inch x) override { draw(x, m_p.y); }
  void draw_y(d::inch y) override { draw(m_p.x, y); }

  void flash(d::inch x, d::inch y) override {
    auto d = aperture().diameter();
    auto r = aperture().roundness();
    auto s = aperture().smear();

    m_p = {x.as_float(), y.as_float()};
    m_minmax->enclose(m_p);
    m_buf[m_count++] = {m_p - s, m_p + s, d, r};
  }
  void flash_x(d::inch x) override { flash(x, m_p.y); }
  void flash_y(d::inch y) override { flash(m_p.x, y); }

  void move(d::inch x, d::inch y) override {
    m_p = {x.as_float(), y.as_float()};
  }
  void move_x(d::inch x) override { move(x, m_p.y); }
  void move_y(d::inch y) override { move(m_p.x, y); }
};

export class fanner : public cnc::fanner {
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

  void move(d::inch x, d::inch y) override {
    m_fan_ref = {x.as_float(), y.as_float()};
    m_v[m_count++] = {m_fan_ref};
    m_v[m_count++] = {m_fan_ref};
  }

  void draw(d::inch x, d::inch y) override {
    m_v[m_count++] = {m_fan_ref};
    m_v[m_count++] = {{x.as_float(), y.as_float()}};
    m_minmax->enclose({x.as_float(), y.as_float()});
  }
  void draw_x(d::inch x) override { draw(x, m_v[m_count - 1].pos.y); }
  void draw_y(d::inch y) override { draw(m_v[m_count - 1].pos.x, y); }
};

class lines : public layer {
  objects * m_o;
  unsigned m_i_count{};

  void update(void (*load)(cnc::pen &), minmax *mm) {
    gerby::pen p { *m_o->is[index()].memory, mm };
    load(p);
    m_i_count = p.count();
  }

public:
  explicit lines(objects * o, unsigned n, dotz::vec4 colour)
    : layer { colour, n }
    , m_o { o }
  {}

  void cmd_draw(vee::command_buffer cb, upc *pc) override {
    pc->colour = colour();
    m_o->lines_ppl.bind(cb, pc);
    vee::cmd_bind_vertex_buffers(cb, 0, m_o->vs.buffer());
    vee::cmd_bind_vertex_buffers(cb, 1, *m_o->is[index()].buffer);
    vee::cmd_draw(cb, v_count, m_i_count);
  }

  static auto create(objects * o, unsigned n, void (*load)(cnc::pen &),
                     dotz::vec4 colour, minmax *mm) {
    auto *res = new lines { o, n, colour };
    res->update(load, mm);
    return hai::uptr<layer>{res};
  }
};

class region : public layer {
  objects * m_o;
  unsigned m_count{};

  void update(void (*load)(cnc::fanner &), minmax *mm) {
    gerby::fanner p { *m_o->rs[index()].memory, mm };
    load(p);
    m_count = p.count();
  }

public:
  explicit region(objects * o, unsigned n, dotz::vec4 colour)
    : layer { colour, n }
    , m_o { o }
  {}

  void cmd_draw(vee::command_buffer cb, upc *pc) override {
    pc->colour = colour();

    m_o->regions_ppl.bind(cb, pc);
    vee::cmd_bind_vertex_buffers(cb, 0, *m_o->rs[index()].buffer);
    vee::cmd_draw(cb, m_count);
  }

  static auto create(objects * o, unsigned n, void (*load)(cnc::fanner &),
                     dotz::vec4 colour, minmax *mm) {
    auto res = new region { o, n, colour };
    res->update(load, mm);
    return hai::uptr<layer>{res};
  }
};

class builder : public cnc::builder {
  objects m_objs;

  hai::varray<hai::uptr<gerby::layer>> m_layers{max_layers};
  minmax m_mm{};

public:
  explicit builder(voo::device_and_queue *dq) : m_objs { dq } {
    for (auto & i : m_objs.is) {
      i = voo::bound_buffer::create_from_host(
        dq->physical_device(), max_insts * sizeof(vtx),
        vee::buffer_usage::vertex_buffer
      );
    }
    for (auto & r : m_objs.rs) {
      r = voo::bound_buffer::create_from_host(
        dq->physical_device(), max_rvtx * sizeof(rvtx),
        vee::buffer_usage::vertex_buffer
      );
    }
  }

  void reset() {
    m_layers.truncate(0);
    m_mm = {};
  }

  void add_lines(void (*fn)(cnc::pen &), dotz::vec4 colour) override {
    m_layers.push_back(lines::create(&m_objs, m_layers.size(), fn, colour, &m_mm));
  }
  void add_region(void (*fn)(cnc::fanner &), dotz::vec4 colour) override {
    m_layers.push_back(region::create(&m_objs, m_layers.size(), fn, colour, &m_mm));
  }

  [[nodiscard]] constexpr auto &layers() noexcept { return m_layers; }
  [[nodiscard]] constexpr auto &minmax() noexcept { return m_mm; }
};

export class thread : public vapp {
  cnc::grb_layer m_layer{};
  bool m_fast_cycle{};
  bool m_redraw{true};

  const char *m_libname;
  hai::uptr<dl::lib> m_lib{};
  void (*m_lb)(cnc::builder *, cnc::grb_layer);

  void cycle_layer_right() {
    m_layer = static_cast<cnc::grb_layer>((m_layer + 1) % cnc::gl_count);
    m_redraw = true;
  }

  void load_builder() {
    m_lib.reset(nullptr);

    silog::log(silog::info, "Loading renderer library");
    m_lib = dl::open(m_libname);
    if (!m_lib) silog::die("failed to load renderer library");
    m_lb = m_lib->fn<void(cnc::builder *, cnc::grb_layer)>("draw");
    if (!m_lb) silog::die("failed to find draw function in renderer library");
    m_redraw = true;
  }

public:
  explicit constexpr thread(const char * libname) : m_libname { libname } {
    using namespace casein;

    handle(KEY_DOWN, K_LEFT, [this] {
      m_layer = static_cast<cnc::grb_layer>((m_layer + cnc::gl_count - 1) % cnc::gl_count);
      m_redraw = true;
    });
    handle(KEY_DOWN, K_RIGHT, [this] {
      cycle_layer_right();
    });
    handle(KEY_DOWN, K_R, [this] {
      m_lb = nullptr;
    });

    handle(KEY_DOWN, K_SPACE, [this] { m_fast_cycle = true;  });
    handle(KEY_UP,   K_SPACE, [this] { m_fast_cycle = false; });
  }

  void run() override {
    voo::device_and_queue dq{"gerby", casein::native_ptr};
    auto rp = voo::single_att_render_pass(dq);

    while (!interrupted()) {
      builder b{&dq};
      voo::swapchain_and_stuff sw{dq, *rp};

      extent_loop(dq.queue(), sw, [&] {
        if (m_lb == nullptr || m_lib->modified()) {
          load_builder();
        }
        if (m_fast_cycle) {
          cycle_layer_right();
        }
        if (m_redraw) {
          m_redraw = false;
          b.reset();
          m_lb(&b, m_layer);
          m_lb(&b, cnc::gl_border);
        }

        auto mm = b.minmax();
        upc pc{
            .center = mm.center(),
            .scale = mm.scale(),
            .aspect = sw.aspect(),
        };

        sw.queue_one_time_submit(dq.queue(), [&] {
          auto scb = sw.cmd_render_pass({
            .command_buffer = sw.command_buffer(),
            .clear_colours { vee::clear_colour(0.01, 0.02, 0.03, 1.0) },
          });
          for (auto &l : b.layers()) {
            l->cmd_draw(sw.command_buffer(), &pc);
          }
        });
      });
    }
  }
};
} // namespace gerby
