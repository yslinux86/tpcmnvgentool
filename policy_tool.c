#include <gtk/gtk.h>
#include <stdlib.h>
#include <ctype.h>

#include "policy.h"
#include "common.h"

/*
gcc policy_tool.c -o policy_tool `pkg-config --cflags --libs gtk+-3.0`
*/

static GtkWidget *check_button[8];
static GtkWidget *start_addr_entry[8];
static GtkWidget *length_entry[8];
static GtkWidget *hash_entry[8];
static GtkWidget *alg_comboboxtext[8];
static GtkWidget *policy_comboboxtext[8];

static const gchar title[5][16] =
{
    "StartAdd(Hex)",
    "Length  (Hex)",
    "Hash",
    "Algorithm",
    "Policy"
};
static uint8_t area_id[8] = {0,1,2,3,4,5,6,7};

static void set_widiget_sensitive(GtkWidget *start_addr_entry, GtkWidget *length_entry, GtkWidget *hash_entry, 
    GtkWidget *alg_comboboxtext, GtkWidget *policy_comboboxtext, gboolean sensitive)
{
    gtk_widget_set_sensitive(start_addr_entry, sensitive);
    gtk_widget_set_sensitive(length_entry, sensitive);
    gtk_widget_set_sensitive(hash_entry, sensitive);
    gtk_widget_set_sensitive(alg_comboboxtext, sensitive);
    gtk_widget_set_sensitive(policy_comboboxtext, sensitive);
}

static void write_default_tpm_policy(policy_tpm_t* policy)
{
    BYTE hash_len[2] = {0x20,0x00};
    BYTE alg[2] = {0x12,0x00};

    memset(policy, 0xff, sizeof(policy_tpm_t));
    memset(policy->start_addr, 0x00, 4);
    memset(policy->length, 0x00, 4);
    memcpy(policy->hash_len, hash_len, 2);
    memcpy(policy->hashAlg, alg, 2);
    policy->policy = 0x00;
}

static void write_default_tcm_policy(policy_tcm_t* policy)
{
    BYTE hash_len[2] = {0x20,0x00};
    BYTE alg[2] = {0x12,0x00};

    memset(policy, 0xff, sizeof(policy_tcm_t));
    memset(policy->start_addr, 0x00, 4);
    memset(policy->length, 0x00, 4);
    memcpy(policy->hash_len, hash_len, 2);
    memcpy(policy->hashAlg, alg, 2);
    policy->policy = 0x00;
}

static void check_button_toggled_cb(GtkCheckButton *checkbutton, gpointer data)
{
    uint8_t i = *(uint8_t*)data;

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton)))
        set_widiget_sensitive(start_addr_entry[i], length_entry[i], hash_entry[i], alg_comboboxtext[i], policy_comboboxtext[i], TRUE);
    else
        set_widiget_sensitive(start_addr_entry[i], length_entry[i], hash_entry[i], alg_comboboxtext[i], policy_comboboxtext[i], FALSE);
}

static void show_error(const gchar *message)
{
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new(NULL,
                                    GTK_DIALOG_MODAL,
                                    GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_OK,
                                    message, "Error");
    gtk_window_set_title(GTK_WINDOW(dialog), "Error");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

typedef void (*save_file_cb)(void*, char*);

static void save_to_tpm_file(void *policy, char* filename)
{
    FILE *pd = NULL;
    pd = fopen(filename, "wb");

    fwrite(policy, sizeof(policy_tpm_file_t), 1, pd);
    fclose(pd);
}

static void save_to_tcm_file(void *policy, char* filename)
{
    FILE *pd = NULL;
    pd = fopen(filename, "wb");

    fwrite(policy, sizeof(policy_tcm_file_t), 1, pd);
    fclose(pd);
}

static void write_policy_file(char* title, void *policy, save_file_cb save_file)
{
    GtkWidget *dialog;
    GtkFileChooser *chooser;
    GtkFileFilter * filter;
    gint res;

    dialog = gtk_file_chooser_dialog_new(title,
                                         NULL,
                                         GTK_FILE_CHOOSER_ACTION_SAVE,
                                         "_Cancel",
                                          GTK_RESPONSE_CANCEL,
                                         "_Save",
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "*.bin");
    gtk_file_filter_add_pattern(filter,"*.bin");

    chooser = GTK_FILE_CHOOSER(dialog);
    gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);
    gtk_file_chooser_add_filter(chooser, filter);

    res = gtk_dialog_run(GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;

        filename = gtk_file_chooser_get_filename(chooser);
        save_file(policy, filename);
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

void swap_data(BYTE* data)
{
    BYTE tmp;

    tmp = data[0];
    data[0] = data[3];
    data[3] = tmp;

    tmp = data[1];
    data[1] = data[2];
    data[2] = tmp;
}

uint8_t check_data_valid(uint8_t i)
{
    uint8_t j;
    const gchar* text;

    if (gtk_entry_get_text_length(GTK_ENTRY(start_addr_entry[i])) != 8)
    {
        show_error("Start Address length Not Correct");
        return 1;
    }
    text = gtk_entry_get_text(GTK_ENTRY(start_addr_entry[i]));
    for (j=0; j<8; j++)
    {
        if (isxdigit(text[j]) == 0)
        {
            show_error("Start Address Not Valid Hex");
            return 1;
        }
    }

    if (gtk_entry_get_text_length(GTK_ENTRY(length_entry[i])) != 8)
    {
        show_error("Length data length Not Correct");
        return 1;
    }
    text = gtk_entry_get_text(GTK_ENTRY(length_entry[i]));
    for (j=0; j<8; j++)
    {
        if (isxdigit(text[j]) == 0)
        {
            show_error("Length Not Valid Hex");
            return 1;
        }
    }

    if (gtk_entry_get_text_length(GTK_ENTRY(hash_entry[i])) != 64)
    {
        show_error("Hash data length Not Correct");
        return 1;
    }
    text = gtk_entry_get_text(GTK_ENTRY(hash_entry[i]));
    for (j=0; j<64; j++)
    {
        if (isxdigit(text[j]) == 0)
        {
            show_error("Hash data Not Valid Hex");
            return 1;
        }
    }

    return 0;
}

static void on_about_button_clicked(GtkWidget *button, gpointer userdata)
{
    GtkWidget *dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "This is a tpcm policy file genertor tool.\nCopyleft@shunyou\n"
            "email: youshun@cgprintech.com\n"
            "github: https://github.com/youshun22/tpcmnvgentool");
    gtk_window_set_title(GTK_WINDOW(dialog), "Information");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void on_tpm_button_clicked(GtkWidget *button, gpointer userdata)
{
    uint8_t i, k;
    policy_tpm_file_t policy;
    char buffer[64];
    BYTE startaddr[4];
    BYTE length[4];
    BYTE hash[32];
    BYTE hash_len[2]={0x20,0x00};
    BYTE alg[2]={0x12,0x00};
    BYTE status[1]={0x01};
    unsigned int outlen;
    uint8_t flag[2]={0,0};

    memset(&policy, 0x00, sizeof(policy_tpm_t));

    for (i=0; i<8; i++)
    {
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button[i])))
        {
            if (check_data_valid(i))
                return;

            if (i>=4)
            {
                k=i-4;
                policy.area_valid2[k] = 0x01;
                memset(&policy.policy2[k], 0xff, sizeof(policy_tpm_t));

                StringToHex((char*)gtk_entry_get_text(GTK_ENTRY(start_addr_entry[i])), (unsigned char*)&policy.policy2[k].start_addr, &outlen);
                swap_data((BYTE*)&policy.policy2[k].start_addr);
                StringToHex((char*)gtk_entry_get_text(GTK_ENTRY(length_entry[i])), (unsigned char*)&policy.policy2[k].length, &outlen);
                swap_data((BYTE*)&policy.policy2[k].length);

                memcpy(&policy.policy2[k].hash_len, hash_len, 2);
                StringToHex((char*)gtk_entry_get_text(GTK_ENTRY(hash_entry[i])), (unsigned char*)&policy.policy2[k].sm3_hash, &outlen);
                memcpy(&policy.policy2[k].hashAlg, alg, 2);
                if (gtk_combo_box_get_active(GTK_COMBO_BOX(policy_comboboxtext[i])) == 1)
                    policy.policy2[k].policy = 0x01;
                else
                    policy.policy2[k].policy = 0x00;

                if (flag[1] == 0)
                {
                    flag[1] = 1;
                    policy.flashrom_valid[1] = 0x01;
                }
            }
            else
            {
                policy.area_valid1[i] = 0x01;
                memset(&policy.policy1[i], 0xff, sizeof(policy_tpm_t));

                StringToHex((char*)gtk_entry_get_text(GTK_ENTRY(start_addr_entry[i])), (unsigned char*)&policy.policy1[i].start_addr, &outlen);
                swap_data((BYTE*)&policy.policy1[i].start_addr);
                StringToHex((char*)gtk_entry_get_text(GTK_ENTRY(length_entry[i])), (unsigned char*)&policy.policy1[i].length, &outlen);
                swap_data((BYTE*)&policy.policy1[i].length);

                memcpy(&policy.policy1[i].hash_len, hash_len, 2);
                StringToHex((char*)gtk_entry_get_text(GTK_ENTRY(hash_entry[i])), (unsigned char*)&policy.policy1[i].sm3_hash, &outlen);
                memcpy(&policy.policy1[i].hashAlg, alg, 2);
                if (gtk_combo_box_get_active(GTK_COMBO_BOX(policy_comboboxtext[i])) == 1)
                    policy.policy1[i].policy = 0x01;
                else
                    policy.policy1[i].policy = 0x00;

                if (flag[0] == 0)
                {
                    flag[0] = 1;
                    policy.flashrom_valid[0] = 0x01;
                }
            }
        }
        else
        {
            if (i>=4)
            {
                policy.area_valid2[i-4]=0x00;
                write_default_tpm_policy(&policy.policy2[i-4]);
            }
            else
            {
                policy.area_valid1[i]=0x00;
                write_default_tpm_policy(&policy.policy1[i]);
            }
        }
    }

    // tcmPrintf("policy", sizeof(policy_tpm_file_t), (const BYTE*)&policy);
    write_policy_file("Save TPM File", (void*)&policy, save_to_tpm_file);
}

static void on_tcm_button_clicked(GtkWidget *button, gpointer userdata)
{
    uint8_t i, k;
    policy_tcm_file_t policy;
    char buffer[64];
    BYTE startaddr[4];
    BYTE length[4];
    BYTE hash[32];
    BYTE hash_len[2]={0x20,0x00};
    BYTE alg[2]={0x12,0x00};
    BYTE status[1]={0x01};
    unsigned int outlen;
    uint8_t flag[2]={0,0};

    memset(&policy, 0x00, sizeof(policy_tcm_t));

    for (i=0; i<8; i++)
    {
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button[i])))
        {
            if (check_data_valid(i))
                return;

            if (i>=4)
            {
                k=i-4;
                policy.area_valid2[k] = 0x01;
                memset(&policy.policy2[k], 0xff, sizeof(policy_tcm_t));

                StringToHex((char*)gtk_entry_get_text(GTK_ENTRY(start_addr_entry[i])), (unsigned char*)&policy.policy2[k].start_addr, &outlen);
                swap_data((BYTE*)&policy.policy2[k].start_addr);
                StringToHex((char*)gtk_entry_get_text(GTK_ENTRY(length_entry[i])), (unsigned char*)&policy.policy2[k].length, &outlen);
                swap_data((BYTE*)&policy.policy2[k].length);

                memcpy(&policy.policy2[k].hash_len, hash_len, 2);
                StringToHex((char*)gtk_entry_get_text(GTK_ENTRY(hash_entry[i])), (unsigned char*)&policy.policy2[k].sm3_hash, &outlen);
                memcpy(&policy.policy2[k].hashAlg, alg, 2);
                if (gtk_combo_box_get_active(GTK_COMBO_BOX(policy_comboboxtext[i])) == 1)
                    policy.policy2[k].policy = 0x01;
                else
                    policy.policy2[k].policy = 0x00;

                if (flag[1] == 0)
                {
                    flag[1] = 1;
                    policy.flashrom_valid[1] = 0x01;
                }
            }
            else
            {
                policy.area_valid1[i] = 0x01;
                memset(&policy.policy1[i], 0xff, sizeof(policy_tcm_t));

                StringToHex((char*)gtk_entry_get_text(GTK_ENTRY(start_addr_entry[i])), (unsigned char*)&policy.policy1[i].start_addr, &outlen);
                swap_data((BYTE*)&policy.policy1[i].start_addr);
                StringToHex((char*)gtk_entry_get_text(GTK_ENTRY(length_entry[i])), (unsigned char*)&policy.policy1[i].length, &outlen);
                swap_data((BYTE*)&policy.policy1[i].length);

                memcpy(&policy.policy1[i].hash_len, hash_len, 2);
                StringToHex((char*)gtk_entry_get_text(GTK_ENTRY(hash_entry[i])), (unsigned char*)&policy.policy1[i].sm3_hash, &outlen);
                memcpy(&policy.policy1[i].hashAlg, alg, 2);
                if (gtk_combo_box_get_active(GTK_COMBO_BOX(policy_comboboxtext[i])) == 1)
                    policy.policy1[i].policy = 0x01;
                else
                    policy.policy1[i].policy = 0x00;

                if (flag[0] == 0)
                {
                    flag[0] = 1;
                    policy.flashrom_valid[0] = 0x01;
                }
            }
        }
        else
        {
            if (i>=4)
            {
                policy.area_valid2[i-4]=0x00;
                write_default_tcm_policy(&policy.policy2[i-4]);
            }
            else
            {
                policy.area_valid1[i]=0x00;
                write_default_tcm_policy(&policy.policy1[i]);
            }
        }
    }

    // tcmPrintf("policy", sizeof(policy_tcm_file_t), (const BYTE*)&policy);
    write_policy_file("Save TCM File", (void*)&policy, save_to_tcm_file);
}

static void activate_window(GtkApplication *app, gpointer userdata)
{
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *grid[2];
    GtkWidget *frame[2];
    GtkWidget *label[2][5];
    GtkWidget *buttonbox;
    GtkWidget *button[3];
    uint8_t i = 0;
    char name[8];

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "TPCM NV Generation Tool");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    buttonbox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(buttonbox), GTK_BUTTONBOX_END);
    button[0] = gtk_button_new_with_label("About");
    button[1] = gtk_button_new_with_label("Generate(TPM)");
    button[2] = gtk_button_new_with_label("Generate(TCM)");

    g_signal_connect(G_OBJECT(button[0]), "clicked", G_CALLBACK(on_about_button_clicked), NULL);
    g_signal_connect(G_OBJECT(button[1]), "clicked", G_CALLBACK(on_tpm_button_clicked), NULL);
    g_signal_connect(G_OBJECT(button[2]), "clicked", G_CALLBACK(on_tcm_button_clicked), NULL);

    for (i=0; i<2; i++)
    {
        grid[i] = gtk_grid_new();
        gtk_grid_set_row_spacing(GTK_GRID(grid[i]), 6);
        gtk_grid_set_column_homogeneous(GTK_GRID(grid[i]), FALSE);
        gtk_grid_set_column_spacing(GTK_GRID(grid[i]), 6);
    }

    for (i=0; i<3; i++)
    {
        gtk_container_set_border_width(GTK_CONTAINER(button[i]), 2);
        gtk_container_add(GTK_CONTAINER(buttonbox), button[i]);
    }

    frame[0] = gtk_frame_new("SPI Flash Rom 1");
    frame[1] = gtk_frame_new("SPI Flash Rom 2");  //not already in used

    for (i=0; i<5; i++)
    {
        label[0][i] = gtk_label_new(title[i]);
        label[1][i] = gtk_label_new(title[i]);
    }

    for (i=0; i<8; i++)
    {
        start_addr_entry[i] = gtk_entry_new();
        gtk_entry_set_width_chars(GTK_ENTRY(start_addr_entry[i]), 8);
        gtk_entry_set_max_length(GTK_ENTRY(start_addr_entry[i]), 8);
        gtk_entry_set_text(GTK_ENTRY(start_addr_entry[i]), "00000000");

        length_entry[i] = gtk_entry_new();
        gtk_entry_set_width_chars(GTK_ENTRY(length_entry[i]), 8);
        gtk_entry_set_max_length(GTK_ENTRY(length_entry[i]), 8);
        gtk_entry_set_text(GTK_ENTRY(length_entry[i]), "00000000");

        hash_entry[i] = gtk_entry_new();
        gtk_entry_set_width_chars(GTK_ENTRY(hash_entry[i]), 64);
        gtk_entry_set_max_length(GTK_ENTRY(hash_entry[i]), 64);

        alg_comboboxtext[i] = gtk_combo_box_text_new();
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(alg_comboboxtext[i]), "SM3_256", "SM3_256");  // only support sm3
        gtk_combo_box_set_active(GTK_COMBO_BOX(alg_comboboxtext[i]), 0);

        policy_comboboxtext[i] = gtk_combo_box_text_new();
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(policy_comboboxtext[i]), "Hang", "Hang");
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(policy_comboboxtext[i]), "Goon", "Goon");
        gtk_combo_box_set_active(GTK_COMBO_BOX(policy_comboboxtext[i]), 0);
    }

    for (i=0; i<8; i++)
    {
        if (i>=4)
            sprintf(name, "Area %d", i-3);
        else
            sprintf(name, "Area %d", i+1);
        check_button[i] = gtk_check_button_new_with_label(name);

        set_widiget_sensitive(start_addr_entry[i], length_entry[i], hash_entry[i], alg_comboboxtext[i], policy_comboboxtext[i], FALSE);
        g_signal_connect(check_button[i], "toggled", G_CALLBACK(check_button_toggled_cb), (gpointer)&area_id[i]);
    }

    for (i=0; i<5; i++)
    {
        gtk_grid_attach(GTK_GRID(grid[0]), label[0][i], i+1,0,1,1);
        gtk_grid_attach(GTK_GRID(grid[1]), label[1][i], i+1,0,1,1);
    }

    for (i=0; i<4; i++)
    {
        gtk_grid_attach(GTK_GRID(grid[0]), check_button[i],     0,i+1,1,1);
        gtk_grid_attach(GTK_GRID(grid[0]), start_addr_entry[i], 1,i+1,1,1);
        gtk_grid_attach(GTK_GRID(grid[0]), length_entry[i],     2,i+1,1,1);
        gtk_grid_attach(GTK_GRID(grid[0]), hash_entry[i],       3,i+1,1,1);
        gtk_grid_attach(GTK_GRID(grid[0]), alg_comboboxtext[i],    4,i+1,1,1);
        gtk_grid_attach(GTK_GRID(grid[0]), policy_comboboxtext[i], 5,i+1,1,1);
    }

    for (i=0; i<4; i++)
    {
        gtk_grid_attach(GTK_GRID(grid[1]), check_button[i+4],     0,i+1,1,1);
        gtk_grid_attach(GTK_GRID(grid[1]), start_addr_entry[i+4], 1,i+1,1,1);
        gtk_grid_attach(GTK_GRID(grid[1]), length_entry[i+4],     2,i+1,1,1);
        gtk_grid_attach(GTK_GRID(grid[1]), hash_entry[i+4],       3,i+1,1,1);
        gtk_grid_attach(GTK_GRID(grid[1]), alg_comboboxtext[i+4],    4,i+1,1,1);
        gtk_grid_attach(GTK_GRID(grid[1]), policy_comboboxtext[i+4], 5,i+1,1,1);
    }

    gtk_container_set_border_width(GTK_CONTAINER(buttonbox), 8);
    for (i=0; i<2; i++)
    {
        gtk_container_set_border_width(GTK_CONTAINER(grid[i]), 8);
        gtk_container_set_border_width(GTK_CONTAINER(frame[i]), 8);
        gtk_container_add(GTK_CONTAINER(frame[i]), grid[i]);
        gtk_container_add(GTK_CONTAINER(vbox), frame[i]);
    }

    gtk_container_add(GTK_CONTAINER(vbox), buttonbox);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    gtk_widget_show_all(window);
}

int main(int argc,char **argv)
{
    GtkApplication *app;
    int status;

    app = gtk_application_new("cn.cgprintech.www", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate_window), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
