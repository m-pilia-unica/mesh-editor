/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) Martino Pilia, 2015
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "frontend.h"

int main(int argc, char *argv[])
{
    /* statements without any effect, only to avoid syntax-checker warnings */
    UNUSED(argc); /* really unused */
    UNUSED(argv); /* unused on Linux and Windows */

    int line;                      /* line number for error messages */
    char out_fname[STR_LEN + 1];   /* filename for the model to be exported */
    Model3D m;                     /* object for imported 3D model */
    FILE *out_file;                /* file in which export modified model */
    Action action = NULL_ACTION;   /* action selected by user in main menu */
    int flag = 1;                  /* flag to remain inside main loop */
    int parser_error;              /* flag for problems parsing the file */
    int color_error;               /* flag for missing/failed coloration */
    int rotation_error;            /* flag for missing/failed rotation */
    
    srand(time(NULL));

    /* show a brief introduction */
    printf( STR_ATT("\nWelcome.", ANSI_ATT_BOLD)
            "\nThis software is made to color a 3D model in .ply format.\n"
            "The software will load one of the .ply files located in the "
            "following folder:\n%s\n", MODEL_DIR);

    do /* while (flag) */
    {
        /* set flags for current execution
         * nonzero value indicates that no coloration/rotation was successfully 
         * done yet; note that this initialization is needed for each cycle */
        color_error = 1;
        rotation_error = 1;
    
        /* init model */
        init_model(&m);
    
        /* get the name of the file containing the model to import */
        get_input_filename(m.filename);
    
        /* open file */
        #ifdef __APPLE__ 
        m.file_model = openFile(
                m.fileName,
                strlen(m.fileName),
                argv[0],
                strlen(argv[0]),
                "r");
        #else /* Linux, Windows */
        m.file_model = fopen(m.filename, "r");
        #endif
    
        /* check if file has been opened succesfully */
        if (m.file_model == NULL)
        {
            printf(STR_COL_ATT(
                        "\nError: unable to open file %s. "
                        "Try with another file.\n",
                        ANSI_COL_RED,
                        ANSI_ATT_BOLD),
                    m.filename);
            continue; /* next do-while iteration */
        }
    
        /* read model data from file */
        parser_error = parse_model_data(&m);

        /* check if file content is useful, otherwise manage errors */
        switch (parser_error)
        {
            case PARSER_ERR_NO_DATA:
                printf(STR_COL_ATT(
                        "\nThe file does not contain any information on model "
                        "vertices or faces.\n"
                        "Please try to pick another model file. \n",
                        ANSI_COL_RED,
                        ANSI_ATT_BOLD));
                /* no clear_model() because there is nothing to dealloc yet */
                fclose(m.file_model);
                continue; /* skip to next do-while iteration */

            case PARSER_ERR_INCOHERENT_DATA:
                printf(STR_COL_ATT(
                        "\nThe number of vertices or faces read from file does"
                        " not match the declaration\nin the .ply file header. "
                        "File may be corrupted.\nPlease try to pick "
                        "another file.\n",
                        ANSI_COL_RED,
                        ANSI_ATT_BOLD));
                clear_model(m);
                fclose(m.file_model);
                continue; /* skip to next do-while iteration */
        }

        /* determine info about current model */
        model_info(&m);

        do /* while (action != SAVE || action != ANOTHER); */
        {

            /* show main menu */
            action = main_menu(m);

            /* do the action selected by the user */
            switch (action)
            {
                case NULL_ACTION:
                    break;

                /* pick another model */
                case ANOTHER:
                    /* if there are unsaved pending changes, 
                     * ask for confirmation */
                    if (!color_error || !rotation_error)
                        action = confirm(action);
                    break; /* action value determine if remain inside of loop */

                case SAVE:
                    if (color_error)
                    {
                        printf(STR_COL_ATT(
                                    "\nError: model must be colored before "
                                    "saving.\n",
                                    ANSI_COL_RED,
                                    ANSI_ATT_BOLD));
                        action = NULL_ACTION; /* remain inside do-while loop */
                    }
                    break;

                case INFO:
                    show_info(m);
                    break;

                case EXIT:
                    /* if there are unsaved pending changes, 
                     * ask for confirmation */
                    if (!color_error || !rotation_error)
                        if (confirm(action) == NULL_ACTION)
                            break;
                    fclose(m.file_model);
                    clear_model(m);
                    exit(EXIT_SUCCESS);

                case ROTATE:
                    /* apply rotation (asking detail to the user */
                    rotation_error = rotate_model(m);
                    break;

                case COL_FLAT:
                case COL_GRAD:
                case COL_DIST:
                case COL_RAND:
                    /* apply the desired coloration (asking user for details) */
                    color_error = color_model(m, action);
                    break;
            }
        } while (action != SAVE && action != ANOTHER);
                
        if (action == ANOTHER) /* pick another model */
        {
            fclose(m.file_model);
            clear_model(m); /* cleanup */
            continue; /* ask another file to import */
        }
    
        do /* while (out_file == NULL) */
        {
            /* get from user the name of the file in which export */
            get_output_filename(out_fname);

            /* open file for output */
            #ifdef __APPLE__
            line = __LINE__ + 1; /* mark next line for eventual error message */
            out_file = openFile(
                    out_fname,
                    strlen(out_fname),
                    argv[0],
                    strlen(argv[0]),
                    "w");
            #else
            line = __LINE__ + 1;
            out_file = fopen(out_fname, "w");
            #endif

            /* error message if file opening failed */
            if (out_file == NULL)
                printf(STR_COL_ATT(
                            "\nError: unable to open/create desired file in "
                            "write mode.\nCheck file permissions or try another"
                            "file name.\n",
                            ANSI_COL_RED,
                            ANSI_ATT_BOLD));

            /* ask another name if file opening failed */
        } while (out_file == NULL);
    
        /* save model in the output file */
        save_model(out_file, m);
    
        /* close files */
        fclose(out_file);
        fclose(m.file_model);
    
        /* resource cleanup */
        clear_model(m);

        /* ask for exit */
        flag = ask_exit();

    } while (flag);

    return EXIT_SUCCESS;
}
