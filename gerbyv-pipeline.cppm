#pragma leco add_shader "gerby.vert"
#pragma leco add_shader "gerby.frag"
export module gerbyv:pipeline;
import :buffers;
import dotz;
import traits;
import vee;
import voo;

namespace gerby {
  struct upc {
    dotz::vec4 colour;
    dotz::vec2 center;
    float scale;
    float aspect;
  };
  
  class pipeline {
    vee::pipeline_layout m_pl;
    vee::gr_pipeline m_ppl;

  public:
    void bind(vee::command_buffer cb, upc * pc) {
      vee::cmd_bind_gr_pipeline(cb, *m_ppl);
      vee::cmd_push_vertex_constants(cb, *m_pl, pc);
    }
    static pipeline create(voo::device_and_queue * dq, vee::gr_pipeline_params p) {
      pipeline res {};
      res.m_pl = vee::create_pipeline_layout(vee::vertex_push_constant_range<upc>());
      res.m_ppl = vee::create_graphics_pipeline({
        .pipeline_layout = *res.m_pl,
        .render_pass = dq->render_pass(),
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        .primitive_restart = true,
        .extent = dq->extent_of(),
        .back_face_cull = false,
        .depth_test = false,
        .shaders{
          voo::shader("gerby.vert.spv").pipeline_vert_stage(),
          voo::shader("gerby.frag.spv").pipeline_frag_stage(),
        },
        .bindings = p.bindings,
        .attributes = p.attributes,
      });
      return res;
    }
  };
  
  auto line_pipeline(voo::device_and_queue * dq) {
    return pipeline::create(dq, {
      .bindings {
        vee::vertex_input_bind(sizeof(vtx)),
        vee::vertex_input_bind_per_instance(sizeof(inst)),
      },
      .attributes{
        vee::vertex_attribute_vec2 (0, traits::offset_of(&vtx::delta)),
        vee::vertex_attribute_vec2 (1, traits::offset_of(&inst::a)),
        vee::vertex_attribute_vec2 (1, traits::offset_of(&inst::b)),
        vee::vertex_attribute_float(0, traits::offset_of(&vtx::w)),
        vee::vertex_attribute_float(1, traits::offset_of(&inst::diam)),
        vee::vertex_attribute_float(1, traits::offset_of(&inst::rnd)),
      },
    });
  }
  auto region_pipeline(voo::device_and_queue * dq) {
    return pipeline::create(dq, {
      .bindings {
        vee::vertex_input_bind(sizeof(rvtx)),
      },
      .attributes{
        vee::vertex_attribute_vec2 (0, traits::offset_of(&rvtx::pad)),
        vee::vertex_attribute_vec2 (0, traits::offset_of(&rvtx::pos)),
        vee::vertex_attribute_vec2 (0, traits::offset_of(&rvtx::pos)),
        vee::vertex_attribute_float(0, traits::offset_of(&rvtx::pad)),
        vee::vertex_attribute_float(0, traits::offset_of(&rvtx::pad)),
        vee::vertex_attribute_float(0, traits::offset_of(&rvtx::pad)),
      },
    });
  }
}
