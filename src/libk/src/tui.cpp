#include "tui.h"
#include "boot_info.h"
#include "multiboot2.h"
#include "vgaprint.h"
#include "resource.h"
#include "vmm.h"
#include "common.h"
#include "cstdio"

pos_t::pos_t(const uint8_t _col, const uint8_t _row)
    : col(_col)
    , row(_row)
{
    return;
}

pos_t::~pos_t(void) {
    return;
}

col_t::col_t(const COLOR::color_t _fore, const COLOR::color_t _back)
    : back(_back)
    , fore(_fore)
{
    return;
}


col_t::~col_t(void) {
    return;
}

char_t::char_t(void) 
    : c(0)
    , color(col_t(COLOR::WHITE, COLOR::BLACK))
{
    return;
}
char_t::char_t(const uint8_t _c, const col_t _color)
    : c(_c)
    , color(_color)
{
    return;
}

char_t::~char_t(void) {
    return;
}

TUI::TUI(){
    set_color(COLOR::WHITE, COLOR::BLACK);
    set_pos(pos_t(0, 0));
    // init();
}

void TUI::init(void)
{
    // 从boot_info中获取地址
    resource_t fb = BOOT_INFO::get_framebuffer();
    TUI_MEM_BASE = fb.fb_info_t.base;
    TUI_MEM_SIZE = fb.fb_info_t.size;
    TUI_WIDTH = fb.fb_info_t.width;
    TUI_HEIGHT = fb.fb_info_t.height;
    buffer = (char_t *)TUI_MEM_BASE;
    return;
}
TUI::~TUI(void)
{
    return;
}

void TUI::set_color(const COLOR::color_t _fore, const COLOR::color_t _back){
    this->color = col_t(_fore, _back);
}

COLOR::color_t TUI::get_color(void) const{
    return (COLOR::color_t)(this->color.back | this->color.fore);
}


// 当前位置
pos_t TUI::pos(0, 0);
// 当前命令行颜色
col_t TUI::color(COLOR::WHITE, COLOR::BLACK);
void TUI::set_pos(const pos_t _pos) {
    const uint16_t index = _pos.row * this->TUI_WIDTH + _pos.col;
    this->pos            = _pos;
    // 光标的设置，见参考资料
    // 告诉 TUI 我们要设置光标的高字节
    PORT::port_outb(TUI_ADDR, TUI_CURSOR_H);
    // 发送高 8 位
    PORT::port_outb(TUI_DATA, index >> 8);
    // 告诉 TUI 我们要设置光标的低字节
    PORT::port_outb(TUI_ADDR, TUI_CURSOR_L);
    // 发送低 8 位
    PORT::port_outb(TUI_DATA, index);
    return;
}

void TUI::set_pos_row(const size_t _row) {
    pos.row = _row;
    set_pos(pos);
    return;
}

void TUI::set_pos_col(const size_t _col) {
    pos.col = _col;
    set_pos(pos);
    return;
}

void TUI::put_entry_at(const char _c, const col_t _color, const size_t _x,
                       const size_t _y) {
    // 计算索引
    const size_t index = _y * TUI_WIDTH + _x;
    write(index, char_t(_c, _color));
    return;
}

bool TUI::escapeconv(const char _c) {
    switch (_c) {
        // 如果是 \n
        case '\n': {
            // 行+1
            pos.row++;
            // 列归零
            pos.col = 0;
            // 串口换行
            serial_putc(_c);
            return true;
        }
        // 如果是 \t
        case '\t': {
            // 取整对齐
            pos.col += (pos.col % 4 == 0) ? 4 : 4 - (pos.col % 4);
            return true;
        }
        // 如果是 \b
        case '\b': {
            if (pos.col > 0) {
                // 删除一个字符
                write(pos.row * TUI_WIDTH + --pos.col, char_t(' ', color));
            }
            return true;
        }
    }
    return false;
}

void TUI::scroll(void) {
    if (pos.row >= TUI_HEIGHT) {
        // 将所有行的显示数据复制到上一行
        for (size_t i = 0; i < (TUI_HEIGHT - 1) * TUI_WIDTH; i++) {
            write(i, read(i + TUI_WIDTH));
        }
        // 最后的一行数据现在填充空格，不显示任何字符
        for (size_t i = (TUI_HEIGHT - 1) * TUI_WIDTH; i < TUI_HEIGHT * TUI_WIDTH; i++) {
            write(i, char_t(' ', color));
        }
        // 向上移动了一行
        pos.row = TUI_HEIGHT - 1;
    }
    return;
}



pos_t TUI::get_pos(void) const {
    // 通过读写相应寄存器获取光标位置
    PORT::port_outb(TUI_ADDR, TUI_CURSOR_H);
    size_t cursor_pos_h = PORT::port_inb(TUI_DATA);
    PORT::port_outb(TUI_ADDR, TUI_CURSOR_L);
    size_t cursor_pos_l = PORT::port_inb(TUI_DATA);
    // 返回光标位置
    return pos_t(cursor_pos_l, cursor_pos_h);
}

void TUI::write(const size_t _idx, const char_t _data) {
    // 向缓冲区写数据
    this->buffer[_idx] = _data;
    return;
}

char_t TUI::read(const size_t _idx) const {
    // 从缓冲区读数据
    return this->buffer[_idx];
}

void TUI::put_char(const char _c) {
    // 转义字符处理
    if (escapeconv(_c) == false) {
        // 在指定位置输出指定字符
        put_entry_at(_c, color, pos.col, pos.row);
        // 输出到端口
        serial_putc(_c);
        // 如果到达最后一列则换行
        if (++pos.col >= TUI_WIDTH) {
            pos.col = 0;
            pos.row++;
        }
    }
    // 屏幕滚动
    scroll();
    // 更新光标位置
    set_pos(pos);
    return;
}

uint8_t TUI::get_char(void) const {
    return 0;
}

void TUI::write_string(const char* _s) {
    write(_s, strlen(_s));
    return;
}

void TUI::write(const char* _s, const size_t _len) {
    for (size_t i = 0; i < _len; i++) {
        put_char(_s[i]);
    }
    return;
}

void TUI::clear(void) {
    // 从左上角开始
    for (size_t y = 0; y < TUI_HEIGHT; y++) {
        for (size_t x = 0; x < TUI_WIDTH; x++) {
            const size_t index = y * TUI_WIDTH + x;
            // 用 ' ' 填满屏幕
            // 字体为灰色，背景为黑色
            write(index, char_t(' ', color));
        }
    }
    // 将光标位置设为屏幕左上角
    set_pos(pos_t(0, 0));
    return;
}
