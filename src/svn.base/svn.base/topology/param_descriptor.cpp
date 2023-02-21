#include <svn.base/topology/param_descriptor.hpp>

#include <cassert>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace svn::base { 

param_io
param_descriptor_data::io_type() const
{
  switch (type)
  {
  case param_type::real: 
    return param_io::real;
  case param_type::list: 
  case param_type::knob_list: 
    return param_io::text;
  case param_type::toggle: 
  case param_type::knob: 
  case param_type::list_knob: 
    return param_io::discrete;
  default: assert(false); return static_cast<param_io>(-1);
  }
}

std::string 
param_descriptor_data::format(bool io, param_value val) const
{
  std::size_t size = format(io, val, nullptr, 0);
  std::vector<char> result(size, '\0');
  format(io, val, result.data(), size);
  return std::string(result.data());
}

bool 
real_descriptor::parse(char const* buffer, float& val) const
{
  std::stringstream str(buffer);
  float inf = std::numeric_limits<float>::infinity();
  str >> val;
  if (!std::strcmp("-inf", buffer)) val = -inf;
  if (val < display.min) return false;
  if (val > display.max) return false;
  return true;
} 

bool
discrete_descriptor::parse(param_type type, bool io, char const* buffer, std::int32_t& val) const
{
  std::stringstream str(buffer);
  switch (type)
  {
  case param_type::knob:
  case param_type::text:
  case param_type::list_knob:
    str >> val;
    if (val < min) return false;
    if (val > max) return false;
    return true;
  case param_type::toggle:
    if (!std::strcmp("On", buffer)) val = 1;
    else if (!std::strcmp("Off", buffer)) val = 0;
    else return false;
    return true;
  case param_type::knob_list:
    assert(names != nullptr);
    for (std::int32_t i = 0; i <= max; i++)
      if (!std::strcmp((*names)[i].c_str(), buffer)) return val = i, true;
    return false;
  case param_type::list:
    assert(items != nullptr);
    for (std::int32_t i = 0; i <= max; i++)
      if (io && !std::strcmp((*items)[i].id.c_str(), buffer)) return val = i, true;
      else if (!io && !std::strcmp((*items)[i].name.c_str(), buffer)) return val = i, true;
    return false;
  default:
    assert(false);
    return false;
  }
}

bool 
param_descriptor_data::parse(bool io, char const* buffer, param_value& val) const
{
  switch (type)
  {
  case param_type::real: return real.parse(buffer, val.real);
  default: return discrete.parse(type, io, buffer, val.discrete);
  }
}

std::size_t 
param_descriptor_data::format(bool io, param_value val, char* buffer, std::size_t size) const
{
  std::string prefix = "";
  std::stringstream stream;
  switch (type) 
  { 
  case param_type::toggle: 
    stream << (val.discrete == 0 ? "Off" : "On"); 
    break;
  case param_type::knob_list:
    assert(discrete.names != nullptr);
    stream << (*discrete.names)[val.discrete];
    break;
  case param_type::list:
    assert(discrete.items != nullptr);
    if (io) stream << (*discrete.items)[val.discrete].id;
    else stream << (*discrete.items)[val.discrete].name;
    break;
  case param_type::knob: case param_type::text: case param_type::list_knob:
    if (discrete.min < 0 && discrete.max > 0 && val.discrete > 0) prefix = "+";
    stream << prefix << val.discrete;
    break;
  case param_type::real: 
    if(real.display.min < 0.0f && real.display.max > 0.0f && val.real > 0.0f) prefix = "+";
    stream << prefix << std::setprecision(real.precision) << std::fixed << val.real; 
    break;
  default: 
    assert(false); 
    break;
  }
  std::string str = stream.str();
  if(buffer == nullptr || size == 0) return str.length() + 1;
  std::memset(buffer, 0, size * sizeof(char));
  std::strncpy(buffer, str.c_str(), size - 1);
  return str.length() + 1;
} 
 
} // namespace svn::base