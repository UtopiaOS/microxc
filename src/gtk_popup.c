#include <gtk/gtk.h>



int init_gtk() {
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.utopia.internal.developerpopup", G_APPLICATION_FLAGS_NONE);
}