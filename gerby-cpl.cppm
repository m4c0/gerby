export module gerby:cpl;
import :distance;
import :fileout;

namespace gerby::cpl {
  export struct part {
    const char * id;
    d::inch mid_x;
    d::inch mid_y;
    bool top_layer { true };
    int rotation { 0 }; // degrees
  };
  export struct builder {
    virtual ~builder() {}
    virtual void add(part) = 0;
  };

  export using b_t = void(cpl::builder *);

  class file_builder : public builder {
    out::file m_f { "out/board.cpl", "wb" };
  public:
    void add(part p) override {
      const char * l = p.top_layer ? "T" : "B";
      d::mm mx = p.mid_x;
      d::mm my = p.mid_y;
      fputln(m_f,
          p.id, ";",
          mx.raw_value(), "mm;",
          my.raw_value(), "mm;",
          l, ";", 
          p.rotation);
    }
  };
  
  export void write(b_t fn) {
    silog::log(silog::info, "Generating placement [out/board.cpl]");

    file_builder fb {};
    fn(&fb);
  }
}
