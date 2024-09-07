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


// A function that draws text centered within a rectangular region (x0, y0) - (x1, y1).
void draw_centering_text(cairo_t *cr, const char *utf8_text, double x0, double y0, double x1, double y1)
{
    // Calculate center position
    double rect_width = x1 - x0, rect_height = y1 - y0;
    double center_x = x0 + rect_width / 2, center_y = y0 + rect_height / 2;

    // Adjust the scale
    cairo_text_extents_t extents;
    double scale_x = 1, scale_y = 1;
    for (;;)
    {
        cairo_text_extents(cr, utf8_text, &extents);

        if (extents.width * scale_x < rect_width * 0.9)
            scale_x *= 1.1;
        else if (extents.width * scale_x > rect_width * 1.1)
            scale_x *= 0.9;
        else if (extents.height * scale_y < rect_height * 0.9)
            scale_y *= 1.1;
        else if (extents.height * scale_y > rect_height * 1.1)
            scale_y *= 0.9;
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

int main(void)
{
    // Initialize cairo
    double page_width = 595.0, page_height = 842.0; // A4 paper size in points
    cairo_surface_t *surface = cairo_pdf_surface_create("output.pdf", page_width, page_height);
    cairo_t *cr = cairo_create(surface);

    // Draw lines
    int rows = 2, cols = 3;
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 3.0);
    for (int irow = 1; irow < rows; ++irow)
    {
        cairo_move_to(cr, 0, page_height * irow / rows);
        cairo_line_to(cr, page_width, page_height * irow / rows);
    }
    for (int icol = 1; icol < cols; ++icol)
    {
        cairo_move_to(cr, page_width * icol / cols, 0);
        cairo_line_to(cr, page_width * icol / cols, page_height);
    }
    cairo_stroke(cr);

    // Choose font and font size
    auto utf8_font_name = u8"ＭＳ 明朝";
    cairo_select_font_face(cr, utf8_font_name, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 40);

    // Split characeters
    auto utf8_text = u8"テスト森鷗外";
    std::vector<std::string> vec;
    u8_split_each_char(vec, utf8_text);

    // Draw characeters
    int ich = 0;
    int cell_width = page_width / cols;
    int cell_height = page_height / rows;
    for (int irow = 0; irow < rows; ++irow)
    {
        for (int icol = 0; icol < cols; ++icol)
        {
            draw_centering_text(cr, vec[ich].c_str(),
                icol * cell_width, irow * cell_height,
                (icol + 1) * cell_width, (irow + 1) * cell_height);
            ++ich;
        }
    }

    // Clean up
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    return 0;
}
