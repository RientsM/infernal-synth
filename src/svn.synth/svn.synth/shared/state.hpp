#ifndef SVN_SYNTH_SHARED_STATE_HPP
#define SVN_SYNTH_SHARED_STATE_HPP

#include <cstdint>

namespace svn::synth {

// Cv attributes.
struct cv_flags
{
  bool bipolar;
  bool inverted;
};

// Cv block with per-block attributes.
struct cv_buffer
{
  float* values;
  cv_flags flags;
};

// Per-voice fixed values.
struct cv_hold_sample
{
  float value;
  cv_flags flags;
};

// Processing time and modulation time.
struct audio_part_output
{
  std::int64_t cv_time;
  std::int64_t own_time;
};

} // namespace svn::synth
#endif // SVN_SYNTH_SHARED_STATE_HPP