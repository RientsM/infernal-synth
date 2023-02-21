#ifndef SVN_SYNTH_VOICE_STATE_HPP
#define SVN_SYNTH_VOICE_STATE_HPP

#include <svn.base/processor/state.hpp>
#include <cstdint>

namespace svn::synth {

struct cpu_usage;

// Voice params. 
// Portamento params are global, but we need them here.
// Voices are recycled in monophonic modes, only switching midi.
struct voice_input
{
  std::int32_t port_mode;
  std::int32_t port_trig;
  std::int32_t port_samples;
  std::int32_t master_mode;
  std::int32_t release_sample;
  std::int32_t new_midi;
  std::int32_t new_midi_start_pos;
  base::block_input_data const* block;
};

} // namespace svn::synth
#endif // SVN_SYNTH_VOICE_STATE_HPP
