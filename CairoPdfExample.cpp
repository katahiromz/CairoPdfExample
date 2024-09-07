#include <cairo.h>
#include <cairo-pdf.h>
#include <cstdio>
#include <cassert>
#include <vector>
#include <string>

constexpr bool u8_is_lead(unsigned char ch)
{
    return (ch & 0xC0) != 0x80;
}
constexpr size_t u8_len(const char *str)
{
    size_t len = 0;
    while (*str)
    {
        if (u8_is_lead(*str))
            ++len;
        ++str;
    }
    return len;
}
static_assert(u8_len(u8"abあいう漢字") == 7, "");
static_assert(u8_len(u8"𠮷") == 1, "");
static_assert(u8_len(u8"😃😃") == 2, "");

void u8_split_each_char(std::vector<std::string>& vec, const char *str)
{
    std::string s;
    for (const char *pch = str; *pch; ++pch)
    {
        if (u8_is_lead(*pch))
        {
            if (s.size())
            {
                vec.push_back(s);
                s.clear();
            }
        }
        s += *pch;
    }
    if (s.size())
        vec.push_back(s);
}

template <typename T_STR>
constexpr bool
mstr_replace_all(T_STR& str, const T_STR& from, const T_STR& to)
{
    bool ret = false;
    size_t i = 0;
    for (;;) {
        i = str.find(from, i);
        if (i == T_STR::npos)
            break;
        ret = true;
        str.replace(i, from.size(), to);
        i += to.size();
    }
    return ret;
}
template <typename T_STR>
constexpr bool
mstr_replace_all(T_STR& str,
                 const typename T_STR::value_type *from,
                 const typename T_STR::value_type *to)
{
    return mstr_replace_all(str, T_STR(from), T_STR(to));
}

template <typename T_STR_CONTAINER>
constexpr void
mstr_split(T_STR_CONTAINER& container,
           const typename T_STR_CONTAINER::value_type& str,
           const typename T_STR_CONTAINER::value_type& chars)
{
    container.clear();
    size_t i = 0, k = str.find_first_of(chars);
    while (k != T_STR_CONTAINER::value_type::npos)
    {
        container.push_back(str.substr(i, k - i));
        i = k + 1;
        k = str.find_first_of(chars, i);
    }
    container.push_back(str.substr(i));
}

void u8_split_lines(std::vector<std::string>& lines, const char *str)
{
    std::string s = str;
    mstr_replace_all(s, "\r\n", "\n");
    mstr_replace_all(s, "\r", "\n");
    mstr_split(lines, s, "\n");
}

// A function that draws text centered within a rectangular region (x0, y0) - (x1, y1).
void draw_centering_text(cairo_t *cr, const char *utf8_text, double x0, double y0, double x1, double y1)
{
    // Calculate center position
    double rect_width = x1 - x0, rect_height = y1 - y0;
    double center_x = x0 + rect_width / 2, center_y = y0 + rect_height / 2;
    double font_size = 10;

    // Adjust the font size and scale
    cairo_text_extents_t extents;
    double scale_x = 1, scale_y = 1;
    for (;;)
    {
        cairo_set_font_size(cr, font_size);
        cairo_text_extents(cr, utf8_text, &extents);

        if (extents.width * scale_x < rect_width * 0.9 && extents.height * scale_y < rect_height * 0.9)
            font_size *= 1.1;
        else if (extents.width * scale_x < rect_width * 0.9)
            scale_x *= 1.1;
        else if (extents.height * scale_y < rect_height * 0.9)
            scale_y *= 1.1;
        else
            break;
    }

    // The text extent
    double text_width = extents.width, text_height = extents.height;

    cairo_save(cr);
    {
        // Move to center
        cairo_translate(cr, center_x, center_y);

        // Scaling
        cairo_scale(cr, scale_x, scale_y);

        // Set the text position
        cairo_move_to(cr, -text_width / 2.0 - extents.x_bearing, -text_height / 2.0 - extents.y_bearing);

        // Draw the text
        cairo_show_text(cr, utf8_text);
    }
    cairo_restore(cr);
}

#define DRAW_LINES

int main(void)
{
    // Initialize cairo
    double page_width = 595.0, page_height = 842.0; // A4 paper size in points
    cairo_surface_t *surface = cairo_pdf_surface_create("output.pdf", page_width, page_height);
    cairo_t *cr = cairo_create(surface);

    // Draw lines
#ifdef DRAW_LINES
    int rows = 2, cols = 1;
#else
    int rows = 2, cols = 3;
#endif
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 3.0);
    for (int iRow = 1; iRow < rows; ++iRow)
    {
        cairo_move_to(cr, 0, page_height * iRow / rows);
        cairo_line_to(cr, page_width, page_height * iRow / rows);
    }
    for (int iCol = 1; iCol < cols; ++iCol)
    {
        cairo_move_to(cr, page_width * iCol / cols, 0);
        cairo_line_to(cr, page_width * iCol / cols, page_height);
    }
    cairo_stroke(cr);

    // Choose font and font size
    auto utf8_font_name = u8"ＭＳ 明朝";
    cairo_select_font_face(cr, utf8_font_name, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

#ifdef DRAW_LINES
    auto utf8_text = u8"てっと\n森鷗外";

    // Split lines
    std::vector<std::string> lines;
    u8_split_lines(lines, utf8_text);

    // Draw lines
    int iLine = 0;
    int cell_width = page_width / cols, cell_height = page_height / rows;
    for (int iRow = 0; iRow < rows; ++iRow)
    {
        int iCol = 0;
        draw_centering_text(cr, lines[iRow].c_str(),
            iCol * cell_width, iRow * cell_height,
            (iCol + 1) * cell_width, (iRow + 1) * cell_height);
    }
#else
    auto utf8_text = u8"てっと森鷗外";

    // Split characeters
    std::vector<std::string> vec;
    u8_split_each_char(vec, utf8_text);

    // Draw characeters
    int ich = 0;
    int cell_width = page_width / cols, cell_height = page_height / rows;
    for (int iRow = 0; iRow < rows; ++iRow)
    {
        for (int iCol = 0; iCol < cols; ++iCol)
        {
            draw_centering_text(cr, vec[ich].c_str(),
                iCol * cell_width, iRow * cell_height,
                (iCol + 1) * cell_width, (iRow + 1) * cell_height);
            ++ich;
        }
    }
#endif

    // Clean up
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    return 0;
}
