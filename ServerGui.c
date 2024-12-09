#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <glib.h>

// Define a structure to hold the widget references
typedef struct {
    GtkWidget *window;  // the main window 
    GtkButton *btnstartssh;  // buttons that will call functions 
    GtkButton *btnstopssh;
    GtkButton *btnstatussh;
    GtkButton *btnstartapach;
    GtkButton *btnstopapach;
    GtkButton *btnstatusapache;
    
    GtkTextView *txtview1; // Add GtkTextView for output
} AppWidgets;

static char *sudo_password = NULL;


void prompt_password(GtkWindow *parent_window) {
    GtkWidget *dialog, *content_area, *entry;
    GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
    gint response;

    // Create a new dialog
    dialog = gtk_dialog_new_with_buttons("Enter Password",
                                         parent_window,
                                         flags,
                                         "_OK",
                                         GTK_RESPONSE_OK,
                                         "_Cancel",
                                         GTK_RESPONSE_CANCEL,
                                         NULL);

    // Set the dialog's default width
    gtk_window_set_default_size(GTK_WINDOW(dialog), 250, -1); // 400px wide, height is automatic (-1)

    // Get the content area and create the entry
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE); // Hide password input
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter your sudo password");

    // Add the entry to the dialog
    gtk_container_add(GTK_CONTAINER(content_area), entry);

    // Show all widgets in the dialog
    gtk_widget_show_all(dialog);

    // Run the dialog and capture the response
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_OK) {
        const char *password = gtk_entry_get_text(GTK_ENTRY(entry));
        if (password && strlen(password) > 0) {
            sudo_password = g_strdup(password); // Duplicate password to store it
        } else {
            fprintf(stderr, "Password cannot be empty. Exiting.\n");
            gtk_widget_destroy(dialog);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "Password input canceled. Exiting.\n");
        gtk_widget_destroy(dialog);
        exit(EXIT_FAILURE);
    }

    // Destroy the dialog after use
    gtk_widget_destroy(dialog);
}



// Function to execute a command using sudo with the password provided
gboolean execute_command(const char *command, GtkTextView *text_view) {
    char full_command[512];
    FILE *pipe;

    snprintf(full_command, sizeof(full_command), "echo '%s' | sudo -S %s 2>&1", sudo_password, command);

    pipe = popen(full_command, "r");
    if (!pipe) {
        perror("popen");
        return FALSE;
    }

    // Get the GtkTextBuffer associated with the GtkTextView
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);
    gtk_text_buffer_set_text(buffer, "", -1); // Clear any existing text

    char buffer_line[256];
    while (fgets(buffer_line, sizeof(buffer_line), pipe)) {
        // Append each line to the GtkTextView
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(buffer, &end);
        gtk_text_buffer_insert(buffer, &end, buffer_line, -1);
    }

    int ret_code = pclose(pipe);
    if (WEXITSTATUS(ret_code) != 0) {
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(buffer, &end);
        gtk_text_buffer_insert(buffer, &end, "\nCommand failed.\n", -1);
        return FALSE;
    }

    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, "\nCommand executed successfully.\n", -1);

    return TRUE;
}


// Signal handlers for the buttons
void on_btnstartssh_clicked(GtkButton *button, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data; // Cast user_data to AppWidgets
   
    execute_command("systemctl start ssh", widgets->txtview1);
}

void on_btnstopssh_clicked(GtkButton *button, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data;
    
    execute_command("systemctl stop ssh", widgets->txtview1);
}

void on_btnstatussh_clicked(GtkButton *button, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data;
    
    execute_command("systemctl status ssh", widgets->txtview1);
}

void on_btnstartapach_clicked(GtkButton *button, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data;

    execute_command("systemctl start apache2", widgets->txtview1);
}

void on_btnstopapach_clicked(GtkButton *button, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data;
    
    execute_command("systemctl stop apache2", widgets->txtview1);
}

void on_btnstatusapache_clicked(GtkButton *button, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data;
    
    execute_command("systemctl status apache2", widgets->txtview1);
}


int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Load the Glade file and set up the UI
    GtkBuilder *builder = gtk_builder_new_from_file("servernewgui.glade");

    // Create an AppWidgets structure and populate it
    AppWidgets widgets;
    
    
    // get the window we are working with 
    widgets.window = GTK_WIDGET(gtk_builder_get_object(builder, "mainwindow"));
    
    // Prompt for the password using a dialog
    prompt_password(GTK_WINDOW(widgets.window));
     
    // get the textView 
    widgets.txtview1 = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "txtview1"));
   
    // set the style  for the textiew widget
   
    GtkCssProvider *cssProviderLog = gtk_css_provider_new();
    GtkStyleContext *context;

    // Set the CSS for the textview
    gtk_css_provider_load_from_data(cssProviderLog,
                                    "textview {"
                                    "font-family: serif;"
                                    "font-size: 12px;"                                    
                                    "color: green;"
                                    "}", -1, NULL);

    // Add the CSS provider to the context of the GtkTextView
    context = gtk_widget_get_style_context(GTK_WIDGET(widgets.txtview1));
    gtk_style_context_add_provider(context,
                                   GTK_STYLE_PROVIDER(cssProviderLog),
                                   GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Unreference the CSS provider to avoid memory leaks
    g_object_unref(cssProviderLog);


   // assign  buttons
   
	widgets.btnstartssh = GTK_BUTTON(gtk_builder_get_object(builder, "btnstartssh"));
	widgets.btnstopssh = GTK_BUTTON(gtk_builder_get_object(builder, "btnstopssh"));
	widgets.btnstatussh = GTK_BUTTON(gtk_builder_get_object(builder, "btnstatussh"));
	widgets.btnstartapach = GTK_BUTTON(gtk_builder_get_object(builder, "btnstartapach"));
	widgets.btnstopapach = GTK_BUTTON(gtk_builder_get_object(builder, "btnstopapach"));
	widgets.btnstatusapache = GTK_BUTTON(gtk_builder_get_object(builder, "btnstatusapache"));

       
    // buttons  signals  ssh
    g_signal_connect(widgets.btnstartssh, "clicked", G_CALLBACK(on_btnstartssh_clicked), &widgets);
	g_signal_connect(widgets.btnstopssh, "clicked", G_CALLBACK(on_btnstopssh_clicked), &widgets);
	g_signal_connect(widgets.btnstatussh, "clicked", G_CALLBACK(on_btnstatussh_clicked), &widgets);


   // buttons  signals apache2 
	g_signal_connect(widgets.btnstartapach, "clicked", G_CALLBACK(on_btnstartapach_clicked), &widgets);
	g_signal_connect(widgets.btnstopapach, "clicked", G_CALLBACK(on_btnstopapach_clicked), &widgets);
	g_signal_connect(widgets.btnstatusapache, "clicked", G_CALLBACK(on_btnstatusapache_clicked), &widgets);

    // window close  signal 
    
    // Connect the "destroy" signal to quit the application
    g_signal_connect(widgets.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);


    g_object_unref(builder);

    // Show the window and start the GTK main loop
    gtk_widget_show(widgets.window);
    gtk_main();

    // Clean up the password from memory
    g_free(sudo_password);

    return 0;
    
}


    

