module gerbyv;

using namespace gerby;

pipeline gerby::line_pipeline(voo::device_and_queue * dq) {
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
pipeline gerby::region_pipeline(voo::device_and_queue * dq) {
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
