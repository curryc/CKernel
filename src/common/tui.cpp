#include "tui.h"

pos_t::pos_t(const uint8_t _col, const uint8_t _row)
    : col(_col)
    , row(_row)
{
}

col_t::col_t(const COLOR::color_t _fore, const COLOR::color_t _back)
    : back(_back)
    , fore(_fore)
{
}

char_t::char_t(const uint8_t _c, const col_t _color)
    : c(_c)
    , color(_color)
{
}

TUI::TUI(){
    set_color(COLOR::WHITE, COLOR::BLACK);
    set_pos(pos_t(0, 0));
}

void TUI::set_color(const COLOR::color_t _fore, const COLOR::color_t _back = COLOR::BLACK){
    this->color = col_t(_fore, _back);
}

COLOR::color_t TUI::get_color(void) const{
    return (COLOR::color_t)(this->color.back | this->color.fore);
}
void TUI::set_pos(const pos_t _pos){
    this->pos = _pos;
}

void TUI::set_pos_col(const size_t _col){
    this->pos.col = _col;
}

void TUI::set_pos_row(const size_t _row){
    this->pos.row = _row;
}

pos_t TUI::get_pos(void) const{
    return this->pos;
}

void TUI::write(const size_t _idx, const char_t _data){
    
}