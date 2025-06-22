export module gerbyv:buffers;
import dotz;
import voo;

namespace gerby {
  struct vtx {
    dotz::vec2 delta;
    float w;
  };
  static_assert(sizeof(vtx) == 3 * sizeof(float));

  constexpr const auto v_count = 8;

  struct inst {
    dotz::vec2 a;
    dotz::vec2 b;
    float diam;
    float rnd;
  };
  static_assert(sizeof(inst) == 6 * sizeof(float));

  struct rvtx {
    dotz::vec2 pos;
    dotz::vec2 pad{};
  };

  class vertices {
    voo::bound_buffer m_vs;
  
    void load_buffer() {
      voo::mapmem m { *m_vs.memory };
  
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
      : m_vs { voo::bound_buffer::create_from_host(
        dq->physical_device(), v_count * sizeof(vtx),
        vee::buffer_usage::vertex_buffer
      ) }
    {
      load_buffer();
    }
  
    [[nodiscard]] constexpr auto buffer() const { return *m_vs.buffer; }
  };
  
  class instances {
    static constexpr const auto max_insts = 10240;
  
    voo::bound_buffer m_is;
  
  public:
    explicit instances(voo::device_and_queue *dq)
      : m_is { voo::bound_buffer::create_from_host(
          dq->physical_device(), max_insts * sizeof(vtx),
          vee::buffer_usage::vertex_buffer
      ) }
    {}
  
    [[nodiscard]] constexpr auto buffer() const { return *m_is.buffer; }
    [[nodiscard]] constexpr auto memory() const { return *m_is.memory; }
  };

  class rvertices {
    static constexpr const auto max_vtx = 10240;
  
    voo::bound_buffer m_is;
  
  public:
    explicit rvertices(voo::device_and_queue *dq)
      : m_is { voo::bound_buffer::create_from_host(
          dq->physical_device(), max_vtx * sizeof(rvtx),
          vee::buffer_usage::vertex_buffer
      ) }
    {}
  
    [[nodiscard]] constexpr auto buffer() const { return *m_is.buffer; }
    [[nodiscard]] constexpr auto memory() const { return *m_is.memory; }
  };
} 
