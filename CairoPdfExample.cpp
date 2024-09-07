#include <cairo.h>
#include <cairo-pdf.h>
#include <iostream>

#include <cairo.h>
#include <cairo-pdf.h>
#include <iostream>

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
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 5.0);
    cairo_move_to(cr, 0, page_height / 2);
    cairo_line_to(cr, page_width, page_height / 2);
    cairo_move_to(cr, page_width / 2, 0);
    cairo_line_to(cr, page_width / 2, page_height);
    cairo_stroke(cr);

    // Choose font and font size
    auto utf8_font_name = u8"ＭＳ 明朝";
    cairo_select_font_face(cr, utf8_font_name, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 40);

    // Draw text
    auto utf8_text = u8"日本語";
    draw_centering_text(cr, utf8_text, 0, 0, page_width, page_height);

    // Clean up
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    return 0;
}
