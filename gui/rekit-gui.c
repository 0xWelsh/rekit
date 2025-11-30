#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *notebook;
    GtkWidget *output_view;
    GtkTextBuffer *output_buffer;
    GtkWidget *file_entry;
    GtkWidget *addr_entry;
    GtkWidget *func_entry;
} AppWidgets;

void append_output(AppWidgets *app, const char *text) {
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(app->output_buffer, &end);
    gtk_text_buffer_insert(app->output_buffer, &end, text, -1);
    gtk_text_buffer_insert(app->output_buffer, &end, "\n", -1);
}

void clear_output(AppWidgets *app) {
    gtk_text_buffer_set_text(app->output_buffer, "", -1);
}

char* run_command(const char *cmd) {
    FILE *fp = popen(cmd, "r");
    if (!fp) return NULL;
    
    char *output = malloc(65536);
    size_t len = 0;
    char line[1024];
    
    while (fgets(line, sizeof(line), fp)) {
        len += strlen(line);
        if (len < 65536) strcat(output, line);
    }
    
    pclose(fp);
    return output;
}

void on_syscall_trace(GtkWidget *widget, gpointer data) {
    AppWidgets *app = (AppWidgets*)data;
    const char *file = gtk_entry_get_text(GTK_ENTRY(app->file_entry));
    
    if (strlen(file) == 0) {
        append_output(app, "Error: No file specified");
        return;
    }
    
    clear_output(app);
    append_output(app, "=== System Call Trace ===\n");
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "timeout 3 ../bin/syscall-tracer %s 2>&1", file);
    
    char *output = run_command(cmd);
    if (output) {
        append_output(app, output);
        free(output);
    }
}

void on_hook_functions(GtkWidget *widget, gpointer data) {
    AppWidgets *app = (AppWidgets*)data;
    const char *file = gtk_entry_get_text(GTK_ENTRY(app->file_entry));
    const char *funcs = gtk_entry_get_text(GTK_ENTRY(app->func_entry));
    
    if (strlen(file) == 0 || strlen(funcs) == 0) {
        append_output(app, "Error: Specify file and functions");
        return;
    }
    
    clear_output(app);
    append_output(app, "=== Function Hooks ===\n");
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "timeout 3 ../bin/dbi-advanced %s %s 2>&1", file, funcs);
    
    char *output = run_command(cmd);
    if (output) {
        append_output(app, output);
        free(output);
    }
}

void on_extract_strings(GtkWidget *widget, gpointer data) {
    AppWidgets *app = (AppWidgets*)data;
    const char *file = gtk_entry_get_text(GTK_ENTRY(app->file_entry));
    
    if (strlen(file) == 0) {
        append_output(app, "Error: No file specified");
        return;
    }
    
    clear_output(app);
    append_output(app, "=== Extracted Strings ===\n");
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "../bin/strings %s 4 2>&1 | head -100", file);
    
    char *output = run_command(cmd);
    if (output) {
        append_output(app, output);
        free(output);
    }
}

void on_parse_pe(GtkWidget *widget, gpointer data) {
    AppWidgets *app = (AppWidgets*)data;
    const char *file = gtk_entry_get_text(GTK_ENTRY(app->file_entry));
    
    if (strlen(file) == 0) {
        append_output(app, "Error: No file specified");
        return;
    }
    
    clear_output(app);
    append_output(app, "=== PE Structure ===\n");
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "../bin/pe-parser %s 2>&1", file);
    
    char *output = run_command(cmd);
    if (output) {
        append_output(app, output);
        free(output);
    }
}

void on_dump_memory(GtkWidget *widget, gpointer data) {
    AppWidgets *app = (AppWidgets*)data;
    const char *file = gtk_entry_get_text(GTK_ENTRY(app->file_entry));
    const char *addr = gtk_entry_get_text(GTK_ENTRY(app->addr_entry));
    
    if (strlen(file) == 0 || strlen(addr) == 0) {
        append_output(app, "Error: Specify PID and address");
        return;
    }
    
    clear_output(app);
    append_output(app, "=== Memory Dump ===\n");
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "../bin/memdump %s %s 0x1000 2>&1", file, addr);
    
    char *output = run_command(cmd);
    if (output) {
        append_output(app, output);
        free(output);
    }
}

void on_file_chooser(GtkWidget *widget, gpointer data) {
    AppWidgets *app = (AppWidgets*)data;
    
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Select File",
        GTK_WINDOW(app->window),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT,
        NULL);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_entry_set_text(GTK_ENTRY(app->file_entry), filename);
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

GtkWidget* create_dbi_tab(AppWidgets *app) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    
    GtkWidget *hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *label1 = gtk_label_new("Target:");
    app->file_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->file_entry), "/bin/cat");
    GtkWidget *browse = gtk_button_new_with_label("Browse");
    g_signal_connect(browse, "clicked", G_CALLBACK(on_file_chooser), app);
    
    gtk_box_pack_start(GTK_BOX(hbox1), label1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox1), app->file_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox1), browse, FALSE, FALSE, 0);
    
    GtkWidget *hbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *label2 = gtk_label_new("Functions:");
    app->func_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->func_entry), "open read write close");
    
    gtk_box_pack_start(GTK_BOX(hbox2), label2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), app->func_entry, TRUE, TRUE, 0);
    
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *btn1 = gtk_button_new_with_label("Trace Syscalls");
    GtkWidget *btn2 = gtk_button_new_with_label("Hook Functions");
    
    g_signal_connect(btn1, "clicked", G_CALLBACK(on_syscall_trace), app);
    g_signal_connect(btn2, "clicked", G_CALLBACK(on_hook_functions), app);
    
    gtk_box_pack_start(GTK_BOX(btn_box), btn1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), btn2, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(vbox), hbox1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), btn_box, FALSE, FALSE, 0);
    
    return vbox;
}

GtkWidget* create_analysis_tab(AppWidgets *app) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *btn1 = gtk_button_new_with_label("Extract Strings");
    GtkWidget *btn2 = gtk_button_new_with_label("Parse PE");
    
    g_signal_connect(btn1, "clicked", G_CALLBACK(on_extract_strings), app);
    g_signal_connect(btn2, "clicked", G_CALLBACK(on_parse_pe), app);
    
    gtk_box_pack_start(GTK_BOX(btn_box), btn1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), btn2, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(vbox), btn_box, FALSE, FALSE, 0);
    
    return vbox;
}

GtkWidget* create_memory_tab(AppWidgets *app) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *label = gtk_label_new("Address:");
    app->addr_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->addr_entry), "0x400000");
    
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), app->addr_entry, TRUE, TRUE, 0);
    
    GtkWidget *btn = gtk_button_new_with_label("Dump Memory");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_dump_memory), app);
    
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 0);
    
    return vbox;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    AppWidgets app;
    
    app.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app.window), "REKit - Reverse Engineering Toolkit");
    gtk_window_set_default_size(GTK_WINDOW(app.window), 900, 600);
    g_signal_connect(app.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(app.window), main_vbox);
    
    app.notebook = gtk_notebook_new();
    
    GtkWidget *dbi_tab = create_dbi_tab(&app);
    GtkWidget *analysis_tab = create_analysis_tab(&app);
    GtkWidget *memory_tab = create_memory_tab(&app);
    
    gtk_notebook_append_page(GTK_NOTEBOOK(app.notebook), dbi_tab, gtk_label_new("Dynamic Analysis"));
    gtk_notebook_append_page(GTK_NOTEBOOK(app.notebook), analysis_tab, gtk_label_new("Static Analysis"));
    gtk_notebook_append_page(GTK_NOTEBOOK(app.notebook), memory_tab, gtk_label_new("Memory"));
    
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    app.output_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app.output_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(app.output_view), TRUE);
    app.output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app.output_view));
    
    gtk_container_add(GTK_CONTAINER(scroll), app.output_view);
    
    gtk_box_pack_start(GTK_BOX(main_vbox), app.notebook, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), scroll, TRUE, TRUE, 0);
    
    gtk_widget_show_all(app.window);
    gtk_main();
    
    return 0;
}
