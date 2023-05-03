#include <inf.base.ui/shared/ui.hpp>
#include <inf.base.ui/shared/config.hpp>
#include <inf.base.ui/controls/label.hpp>
#include <inf.base.ui/controls/dropdown.hpp>
#include <inf.base/shared/support.hpp>

using namespace juce;
using namespace inf::base;

namespace inf::base::ui {

static Rectangle<int> 
with_container_padding(Rectangle<int> const& bounds)
{
  return Rectangle<int>(
    bounds.getX() + container_padding, bounds.getY() + container_padding,
    bounds.getWidth() - 2 * container_padding, bounds.getHeight() - 2 * container_padding);
}

Component*
ui_element::build(LookAndFeel const& lnf)
{
  _component.reset(build_core(lnf));
  _component->setVisible(true);
  return _component.get();
}

Component* 
root_element::build_core(LookAndFeel const& lnf)
{
  container_component* result = new container_component(container_component::flags::fill, 0.0f, 0.0f, Colours::black, Colour());
  result->addChildComponent(_content->build(lnf));
  result->setOpaque(true);
  result->setLookAndFeel(&_lnf);
  return result;
}

void
root_element::layout()
{
  std::int32_t w = _width;
  std::int32_t h = _content->pixel_height();
  component()->setBounds(0, 0, w, h);
  _content->component()->setBounds(0, 0, w, h);
  _content->layout();
}

Component*
container_element::build_core(LookAndFeel const& lnf)
{
  float const radius = 8.0f;
  float const thickness = 1.0f;
  container_component* result = new container_component(_flags, radius, thickness, _fill, _outline);
  result->addChildComponent(_content->build(lnf));
  return result;
}

void
container_element::layout()
{
  _content->component()->setBounds(component()->getLocalBounds());
  _content->layout();
}

Component*
group_label_element::build_core(LookAndFeel const& lnf)
{
  Label* result = new inf_label(false);
  result->setText(_text, dontSendNotification);
  result->setJustificationType(Justification::centred);
  result->setFont(juce::Font(get_group_label_font_height(controller()), juce::Font::bold));
  result->setColour(Label::ColourIds::textColourId, lnf.findColour(inf_look_and_feel::colors::group_label_color));
  return result;
}

void
group_label_element::layout()
{
  auto label = dynamic_cast<Label*>(component());
  label->setBorderSize(BorderSize<int>());
  float rotation_angles = _vertical? 270.0f / 360.0f * 2.0f * pi32: 0.0f;
  auto transform = AffineTransform().rotated(rotation_angles, label->getWidth() / 2.0f, label->getHeight() / 2.0f);
  label->setTransform(transform);
  label->setBounds(label->getBounds().transformedBy(transform));
  label->setJustificationType(_vertical? Justification::centred : Justification::centredBottom);
}

Component*
param_label_element::build_core(LookAndFeel const& lnf)
{
  Label* result = new inf_label(true);
  result->setBorderSize(BorderSize<int>());
  auto topology = controller()->topology();
  auto const& desc = topology->get_param_descriptor(_part_id, _param_index);
  std::int32_t index = controller()->topology()->param_index(_part_id, _param_index);
  param_value value = controller()->state()[index];
  if(desc.data.type == param_type::real) value.real = desc.data.real.display.to_range(value.real);
  result->setJustificationType(_justification);
  result->setFont(juce::Font(get_param_label_font_height(controller()), juce::Font::bold));
  result->setText(get_label_text(&desc, _type, value), dontSendNotification);
  _listener.reset(new label_param_listener(controller(), result, index, _type));
  return result;
}

void 
param_edit_element::layout()
{
  // Cant be inline/header because static with_container_padding.
  auto bounds = with_container_padding(component()->getBounds());
  if (_type == edit_type::dropdown)
  {
    float vpad = get_dropdown_vpad(controller());
    float text_height = get_dropdown_font_height(controller());
    float total_height = text_height + vpad;
    float total_width = bounds.getWidth() - dropdown_hpad;
    float drop_x = bounds.getX() + dropdown_hpad / 2.0f;
    float drop_y = bounds.getY() + (bounds.getHeight() - total_height) / 2.0f;
    bounds = Rectangle<int>(
      static_cast<int>(drop_x), static_cast<int>(drop_y),
      static_cast<int>(total_width), static_cast<int>(total_height));
  }
  component()->setBounds(bounds);
}

Component* 
param_edit_element::build_core(juce::LookAndFeel const& lnf)
{
  switch (_type)
  {
  case edit_type::toggle: return build_toggle_core(lnf);
  case edit_type::dropdown: return build_dropdown_core(lnf);
  default: return build_slider_core(lnf);
  }
}

Component*
param_edit_element::build_toggle_core(LookAndFeel const& lnf)
{
  std::int32_t index = controller()->topology()->param_index(_part_id, _param_index);
  ToggleButton* result = new ToggleButton();
  _toggle_listener.reset(new toggle_param_listener(controller(), result, index));
  result->setToggleState(controller()->state()[index].discrete != 0, dontSendNotification);
  result->addListener(_toggle_listener.get());
  return result;
}

Component*
param_edit_element::build_dropdown_core(LookAndFeel const& lnf)
{
  std::int32_t index = controller()->topology()->param_index(_part_id, _param_index);
  auto const& desc = controller()->topology()->get_param_descriptor(_part_id, _param_index);
  inf_dropdown* result = new inf_dropdown(&desc);
  result->setJustificationType(Justification::centred);
  for(std::size_t i = 0; i < desc.data.discrete.items->size(); i++)
    result->addItem((*desc.data.discrete.items)[i].name, static_cast<std::int32_t>(i) + dropdown_id_offset);
  result->setSelectedItemIndex(controller()->state()[index].discrete, dontSendNotification);
  _dropdown_listener.reset(new dropdown_param_listener(controller(), result, index));
  result->addListener(_dropdown_listener.get());
  return result;
}

Component*
param_edit_element::build_slider_core(LookAndFeel const& lnf)
{
  std::int32_t index = controller()->topology()->param_index(_part_id, _param_index);
  auto const& desc = controller()->topology()->get_param_descriptor(_part_id, _param_index);
  inf_slider* result = new inf_slider(&desc, _type);
  result->setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
  if(desc.data.type == param_type::real)
  {
    result->setRange(desc.data.real.display.min, desc.data.real.display.max, 0.0);
    result->setValue(desc.data.real.display.to_range(controller()->state()[index].real), dontSendNotification);
  }
  else
  {
    result->setRange(desc.data.discrete.min, desc.data.discrete.max, 1.0);
    result->setValue(controller()->state()[index].discrete, dontSendNotification);
  }
  switch (_type)
  {
  case edit_type::knob:
  case edit_type::selector:
    result->setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    break;
  case edit_type::hslider:
    result->setSliderStyle(Slider::SliderStyle::LinearHorizontal);
    break;
  case edit_type::vslider:
    result->setSliderStyle(Slider::SliderStyle::LinearVertical);
    break;
  default:
    assert(false);
    break;
  }
  _slider_listener.reset(new slider_param_listener(controller(), result, index));
  result->addListener(_slider_listener.get());
  return result;
}

Component*
grid_element::build_core(LookAndFeel const& lnf)
{
  Component* result = new Component;
  for (std::size_t i = 0; i < _cell_contents.size(); i++)
    result->addChildComponent(_cell_contents[i]->build(lnf));
  return result;
}

// For root only.
std::int32_t 
grid_element::pixel_height()
{
  double rows = static_cast<double>(_row_distribution.size());
  double cols = static_cast<double>(_column_distribution.size());
  double col_width = controller()->editor_current_width() / cols;
  double row_height = col_width + get_param_label_total_height(controller());
  return static_cast<std::int32_t>(std::ceil(rows * row_height));
}

ui_element*
grid_element::add_cell(std::unique_ptr<ui_element>&& content,
  std::int32_t row, std::int32_t col, std::int32_t row_span, std::int32_t col_span)
{
  ui_element* result = content.get();
  _cell_bounds.push_back(Rectangle<std::int32_t>(col, row, col_span, row_span));
  _cell_contents.push_back(std::move(content));
  return result;
}

void
grid_element::layout()
{
  Grid grid;
  for(std::size_t row = 0; row < _row_distribution.size(); row++)
    grid.templateRows.add(_row_distribution[row]);
  for (std::size_t col = 0; col < _column_distribution.size(); col++)
    grid.templateColumns.add(_column_distribution[col]);
  for (std::size_t i = 0; i < _cell_contents.size(); i++)
  {
    GridItem item(_cell_contents[i]->component());
    Rectangle<std::int32_t> bounds = _cell_bounds[i];
    grid.items.add(item.withArea(bounds.getY() + 1, bounds.getX() + 1, GridItem::Span(bounds.getHeight()), GridItem::Span(bounds.getWidth())));
  }
  grid.performLayout(component()->getLocalBounds());
  for (std::size_t i = 0; i < _cell_contents.size(); i++)
    _cell_contents[i]->layout();
}

std::unique_ptr<grid_element>
create_grid_ui(
  plugin_controller* controller,
  std::vector<std::int32_t> const& row_distribution_relative, 
  std::vector<std::int32_t> const& column_distribution_relative)
{
  std::vector<Grid::TrackInfo> row_distribution;
  std::vector<Grid::TrackInfo> column_distribution;
  for(std::size_t i = 0; i < row_distribution_relative.size(); i++)
    row_distribution.push_back(Grid::TrackInfo(Grid::Fr(row_distribution_relative[i])));
  for (std::size_t i = 0; i < column_distribution_relative.size(); i++)
    column_distribution.push_back(Grid::TrackInfo(Grid::Fr(column_distribution_relative[i])));
  return create_grid_ui(controller, row_distribution, column_distribution);
}

std::unique_ptr<ui_element>
create_param_ui(
  plugin_controller* controller, std::int32_t part_type, std::int32_t part_index, 
  std::int32_t param_index, label_type label_type, edit_type edit_type)
{
  if (edit_type == edit_type::hslider)
  {
    auto auto_rest = Grid::TrackInfo(Grid::Fr(1));
    auto fixed_label_width = Grid::TrackInfo(Grid::Px(get_param_label_hslider_width(controller)));
    auto result = create_grid_ui(controller, { auto_rest }, { fixed_label_width, auto_rest });
    result->add_cell(create_param_label_ui(controller, part_type, part_index, param_index, label_type, juce::Justification::centredRight), 0, 0);
    result->add_cell(create_param_edit_ui(controller, part_type, part_index, param_index, edit_type), 0, 1);
    return result;
  } else
  {
    auto auto_rest = Grid::TrackInfo(Grid::Fr(1));
    auto fixed_label_height = Grid::TrackInfo(Grid::Px(get_param_label_total_height(controller)));
    auto result = create_grid_ui(controller, { auto_rest, fixed_label_height }, { auto_rest });
    result->add_cell(create_param_edit_ui(controller, part_type, part_index, param_index, edit_type), 0, 0);
    result->add_cell(create_param_label_ui(controller, part_type, part_index, param_index, label_type), 1, 0);
    return result;
  }
}

std::unique_ptr<ui_element>
create_group_ui(plugin_controller* controller, std::unique_ptr<group_label_element>&& label, std::unique_ptr<ui_element>&& content)
{
  std::vector<Grid::TrackInfo> rows;
  std::vector<Grid::TrackInfo> cols;
  auto label_height = get_group_label_total_height(controller);
  if (label->vertical())
  {
    rows.push_back(Grid::TrackInfo(Grid::Fr(1)));
    cols.push_back(Grid::TrackInfo(Grid::Px(label_height + group_label_vpad)));
    cols.push_back(Grid::TrackInfo(Grid::Fr(1)));
  }
  else
  {
    rows.push_back(Grid::TrackInfo(Grid::Px(label_height)));
    rows.push_back(Grid::TrackInfo(Grid::Fr(1)));
    cols.push_back(Grid::TrackInfo(Grid::Fr(1)));
  }
  auto result = create_grid_ui(controller, rows, cols);
  if (label->vertical())
  {
    result->add_cell(std::move(label), 0, 0);
    result->add_cell(std::move(content), 0, 1);
  }
  else
  {
    result->add_cell(std::move(label), 0, 0);
    result->add_cell(std::move(content), 1, 0);
  }
  return result;
}

} // namespace inf::base::ui