#include <gtk/gtk.h>
#include <glib.h>
#include <math.h>

#define LENGTH 60
#define MIN_VALUE 0
#define MAX_VALUE 100
#define CORES 4

gint64 last_data_time;

static GList *data[4];
cairo_surface_t *offscreen = NULL;

gboolean on_chart_draw (GtkWidget *widget,
                        cairo_t   *crx,
                        gpointer  user_data) 
{ 
  gint i,j;
  gdouble v_step, h_step;
  GtkAllocation allocation;
  gtk_widget_get_allocation(widget, &allocation);
  cairo_t *cr = NULL;
  if (offscreen == NULL) {
    printf ("Creating offscreen surface\n");
    cairo_pattern_t * pattern = cairo_get_source (crx);
    printf ("Created pattern\n");
    cairo_surface_t *surface;
    cairo_pattern_get_surface (pattern, &surface);
    printf ("Getting surface from pattern\n");
    offscreen = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, allocation.width, allocation.height);
    printf ("Creating offscreen\n");
    cr = cairo_create (offscreen);
    printf ("Created offscreen context %d, %p\n", cairo_status (cr), cr);
  }
  v_step = (double)allocation.height / (MAX_VALUE - MIN_VALUE);
  h_step = (double)allocation.width / (LENGTH -1);
  cairo_set_line_width (cr, 1.0);
  gint64 between = g_get_real_time () - last_data_time;
  gdouble shift = (double)between/1000000 * h_step;
  //printf ("Partial redraw at %li, after %li, shiftin with %.2f out of %.2f\n", g_get_real_time(), between, shift, h_step);
  for (j=0;j<CORES;j++) {
    GList *current = data[j];
    cairo_set_source_rgb (cr, j==0 || j==4 ? 1.0 :0.0, j==1 || j==4 ? 1.0 :0.0, j==2 || j==4 ? 1.0 :0.0);
    cairo_move_to (cr, 0, allocation.height - v_step * GPOINTER_TO_INT(current->data));
    for (i = 1;i<LENGTH-1;i++) {
      cairo_line_to (cr, i*h_step-shift, allocation.height - v_step * GPOINTER_TO_INT ((current = g_list_next (current))->data));
    }
    cairo_line_to (cr, allocation.width, allocation.height - v_step *  GPOINTER_TO_INT ((current = g_list_next (current))->data));
    cairo_stroke (cr);
  }
  //cairo_set_source_surface (crx, offscreen, 0, 0);
  //cairo_paint (crx);
  return TRUE;
}

static void data_refreshed(GtkWidget * chart)
{
  gint j;
  for (j=0;j<CORES;j++) {
    data[j] = g_list_delete_link (data[j], g_list_first (data[j]));
    data[j] = g_list_append (data[j], GINT_TO_POINTER(
                                        CLAMP(
                                          GPOINTER_TO_INT( g_list_last(data[j])->data) 
                                          + g_random_int_range(-10, 10), MIN_VALUE, MAX_VALUE
                                        )
                                      ));
  }
  /*gint i;
  for (i=0;i<LENGTH;i++) {
    printf ("%d, ", GPOINTER_TO_INT(g_list_nth_data (data, i)));
  }*/
  last_data_time = g_get_real_time ();
  printf(" Refreshed at %li\n", last_data_time);
}

gint
main (int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *chart;
  gtk_init (&argc, &argv);
  gint i, j;
  for (j=0;j<CORES;j++) {
    data[j] = NULL;
    for (i=0;i<LENGTH;i++) {
      data[j] = g_list_append (data[j], GINT_TO_POINTER(g_random_int_range(MIN_VALUE, MAX_VALUE)));
    }
  }
  
  
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_size_request (window, 100, 100);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
  g_signal_connect (window, "delete-event", G_CALLBACK (gtk_main_quit), NULL);
  gtk_widget_show (window);
  
  chart = gtk_drawing_area_new ();
  g_signal_connect (chart, "draw", G_CALLBACK (on_chart_draw), NULL);
  gtk_container_add (GTK_CONTAINER (window), chart);
  g_timeout_add_seconds (1, (GSourceFunc)data_refreshed, chart);
  g_timeout_add (1000/60, (GSourceFunc)gtk_widget_queue_draw, chart);
  
  gtk_widget_show (chart);
  
  
  gtk_main ();
  
  return 0;
}
