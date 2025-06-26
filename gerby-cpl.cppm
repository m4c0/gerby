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
  export struct bom {
    const char * comment;
    const char * ids;
    const char * footprint;
    const char * part_no;
  };
  export struct builder {
    virtual ~builder() {}
    virtual void bom(struct bom) = 0;
    virtual void part(struct part) = 0;
  };

  export using b_t = void(cpl::builder *);

  class file_builder : public builder {
    out::file m_cpl { "out/board.cpl.csv", "wb" };
    out::file m_bom { "out/board.bom.csv", "wb" };
  public:
    file_builder() {
      fputln(m_cpl, "Designator,Mid X,Mid Y,Layer,Rotation");
      fputln(m_bom, "Comment,Designator,Footprint,Part #");
    }

    void part(struct part p) override {
      const char * l = p.top_layer ? "T" : "B";
      d::mm mx = p.mid_x;
      d::mm my = p.mid_y;
      fputln(m_cpl,
          p.id, ",",
          mx.raw_value(), "mm,",
          my.raw_value(), "mm,",
          l, ",", 
          p.rotation);
    }
    void bom(struct bom b) override {
      fputln(m_bom,
          b.comment, ",",
          b.ids, ",",
          b.footprint, ",",
          b.part_no);
    }
  };
  
  export void write(b_t fn) {
    silog::log(silog::info, "Generating BOM and CPL");

    file_builder fb {};
    fn(&fb);
  }
}
