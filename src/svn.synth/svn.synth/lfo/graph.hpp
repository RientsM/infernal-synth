#ifndef SVN_SYNTH_LFO_GRAPH_HPP
#define SVN_SYNTH_LFO_GRAPH_HPP

#include <svn.base/processor/graph_processor.hpp>
#include <cstdint>

namespace svn::synth {

class lfo_graph:
public svn::base::graph_processor
{
public:
  lfo_graph(base::topology_info const* topology, base::part_id id);

  bool needs_repaint(std::int32_t runtime_param) const override;
  bool dsp_to_plot(base::graph_plot_input const& input, std::vector<float>& plot) override;
  std::int32_t sample_count(base::param_value const* state, float sample_rate) const override;
  void process_dsp_core(base::block_input const& input, float* output, float sample_rate) override;
};

inline lfo_graph::
lfo_graph(base::topology_info const* topology, base::part_id id):
svn::base::graph_processor(topology, id) {}

} // namespace svn::synth
#endif // SVN_SYNTH_LFO_GRAPH_HPP