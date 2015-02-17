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

/*!
 * \file frontend.c
 * @author Martino Pilia
 * @date 2015-01-24
 */

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "frontend.h"

/*!
 * This procedure shows a menu in the console, asking the user to chose one
 * of the default files or to insert another filename manually. It gets the
 * user input and writes the corresponding filename in its parameter (including
 * the name of the folder containing the file).
 */
void get_input_filename(char *fileName)
{
    int flag, choice, fname_len;
    char input[STR_LEN];
    char specifier[STR_LEN + 1];

    /* show menu with avaible files
     * note: files in menu have a one-based numeration */
    printf( "\nThe following model files are avaible:\n"
            "  1: buddha_n.ply\n"
            "  2: bunny_n.ply\n"
            "  3: cube_n.ply\n"
            "  4: heli_n.ply\n"
            "  5: swirl_n.ply\n"
            "  6: insert another filename manually\n");

    /* get input filename from user */
    do /* while (flag) */
    {
        flag = 0; /* set condition for exit: will be immutated if input
                     is valid */

        printf(STR_ATT(
                    "Chose a file to edit, then press Return [1-6]: ",
                    ANSI_ATT_BOLD));
    
        /* get desired file number to import */
        scanf("%d", &choice);
        clear_stdin();

        /* input validation */
        if (choice < 1 || choice > 6)
        {
            printf(STR_COL_ATT(
                    "\nInvalid file number. Please retry.\n",
                    ANSI_COL_RED,
                    ANSI_ATT_BOLD));
            flag = 1;
        }
    } while (flag);

    /* copy actual filename */
    switch (choice)
    {
        case 1:
            strncpy(fileName, MODEL_DIR "buddha_n.ply", STR_LEN);
            break;
            
        case 2:
            strncpy(fileName, MODEL_DIR "bunny_n.ply", STR_LEN);
            break;
            
        case 3:
            strncpy(fileName, MODEL_DIR "cube_n.ply", STR_LEN);
            break;
            
        case 4:
            strncpy(fileName, MODEL_DIR "heli_n.ply", STR_LEN);
            break;
            
        case 5:
            strncpy(fileName, MODEL_DIR "swirl_n.ply", STR_LEN);
            break;
            
        case 6:
            /* custom filename */
            fname_len = STR_LEN - strlen(MODEL_DIR); // filename length allowed
            printf(STR_ATT(
                        "Insert filename (max. %d chars): ", 
                        ANSI_ATT_BOLD),
                    fname_len);
            sprintf(specifier, "%%%ds", fname_len);
            scanf(specifier, &input);
            sprintf(fileName, "%s%s", MODEL_DIR, input);
            break;
            
        default:
            /* should be unreachable */
            perror("Something went seriously wrong");
            exit(EXIT_FAILURE);
            break;
    }
}

/*!
 * This procedure asks the user for a string containing the filename, then
 * writes it in its parameter string (including the name of the folder 
 * containing the file).
 */
void get_output_filename(char *outFile)
{
    int flag;
    char s[STR_LEN];

    /* set maximum length for filename (without path), -4 is reserved
     * for the extension */
    const int fname_len = STR_LEN - strlen(MODEL_DIR) - 4;

    /* get desired filename for export */
    do /* while (flag) */
    {
        flag = 0; /* set condition for exit: will be immutated if input
                     is valid */

        /* get input */
        char specifier[STR_LEN]; /* tmp string for lenth and specifier */
        sprintf(specifier, "%d", fname_len);
        printf(STR_ATT(
                    "\nInsert a filename for the modified model, without "
                    "extension\n(max. %s chars): ",
                    ANSI_ATT_BOLD),
                specifier);

        sprintf(specifier, "%%%ds", fname_len);
        scanf(specifier, s); /* get filename */
        clear_stdin();

        /* input validation: check for forbidden chars in filename */
        #if defined(__APPLE__) || defined(__linux__)
        if (strchr(s, '/') != NULL)
        {
            printf(STR_COL_ATT(
                        "\nFilename cannot contain any '/' character. "
                        "Please retry.\n",
                        ANSI_COL_RED,
                        ANSI_ATT_BOLD));
            flag = 1;
            continue;
        }
        #else // _WIN32
        if (
                strchr(s, '\\') != NULL ||
                strchr(s, '/') != NULL ||
                strchr(s, '>') != NULL ||
                strchr(s, '<') != NULL ||
                strchr(s, ':') != NULL ||
                strchr(s, '"') != NULL ||
                strchr(s, '|') != NULL ||
                strchr(s, '?') != NULL ||
                strchr(s, '*') != NULL 
                )
        {
            printf(STR_COL_ATT(
                        "\nFilename cannot contain any of the following " 
                        "characters: \\ / > < : \" | ? *\n"
                        "Please retry.\n", 
                        ANSI_COL_RED,
                        ANSI_ATT_BOLD));
            flag = 1;
            continue;
        }
        #endif // defined(__APPLE__) || defined(__linux__)

        /* add extension */
        strcat(s, ".ply");
    } while (flag);
    
    /* copy output filename for caller */
    strcpy(outFile, MODEL_DIR);
    strcat(outFile, s);
}

/*!
 * Show in the console a menu, asking the user for the desired action. Validate
 * input and return the choice to the caller.
 */
Action main_menu(Model3D m)
{
    Action choice;

    /* show a menu and get user choice */
    printf( "\nModel: %s\n"
            "Avaible actions:\n"
            "  1: flat coloration\n"
            "  2: gradient coloration\n"
            "  3: distance-based coloration\n"
            "  4: random coloration\n"
            "  5: model info\n"
            "  6: apply a rotation\n"
            "  7: pick another model\n"
            "  8: save modified model\n"
            "  9: exit without actions\n"
            STR_ATT("Chose an action [1-9]: ", ANSI_ATT_BOLD),
            m.filename);

    /* get the user choice */
    do
    {
        scanf("%d", &choice);
        clear_stdin();
        
        /* show message on invalid choice */
        if (choice < COL_FLAT || choice > EXIT)
            printf(STR_ATT(
                    "Invalid choice. Please retry [1-9]: ",
                    ANSI_ATT_BOLD
                    ));

    } while (choice < COL_FLAT || choice > EXIT);

    return choice;
}

/*!
 * Ask the user if he/she wants to exit or to process another model, then
 * get the response and return zero for an exit choiche, nonzero otherwise.
 */
int ask_exit(void)
{
    int flag;

    /* ask the user */
    printf( "\nWhat do you want to do?\n"
            "  1: pick another model\n"
            "  2: exit\n");

    /* get user choice */
    do
    { 
        printf(STR_ATT("Choose an action: ", ANSI_ATT_BOLD));
        scanf("%d", &flag);
        clear_stdin();
        if (flag < 1 || flag > 2)
            printf(STR_COL_ATT(
                        "Invalid choice. Please retry.\n",
                        ANSI_COL_RED,
                        ANSI_ATT_BOLD));
    } while (flag < 1 || flag > 2);

    return flag - 2; /* 0 if user wants to exit */
}

/*!
 * This procedure shows on the console some info related to a 
 * Model3D object:       
 * <ul>
 *   <li> vertices number; </li>
 *   <li> faces number; </li>
 *   <li> left extreme index and coordinates; </li>
 *   <li> right extreme index and coordinates; </li>
 *   <li> up extreme index and coordinates; </li>
 *   <li> down extreme index and coordinates; </li>
 *   <li> front extreme index and coordinates; </li>
 *   <li> back extreme index and coordinates; </li>
 *   <li> largest face vertices indexes and surface; </li>
 *   <li> smallest face vertices indexes and surface; </li>
 *   <li> total surface; </li>
 *   <li> total volume. </li>
 * </ul>
 */           
void show_info(Model3D model)
{
    char left[STR_LEN];
    char right[STR_LEN];
    char up[STR_LEN];
    char down[STR_LEN];
    char front[STR_LEN];
    char back[STR_LEN];
    char largest[STR_LEN];
    char smallest[STR_LEN];
    
    if (       model.info.min_x == NULL
            || model.info.max_x == NULL
            || model.info.max_y == NULL
            || model.info.min_y == NULL
            || model.info.max_z == NULL
            || model.info.min_z == NULL
            || model.info.biggest_face == NULL
            || model.info.smallest_face == NULL
            )
    {
        /* should be unrechable if the code is ok and nothing crazy happens */
        perror("show_info(): model must be initialized correctly in order to "
                "show info");
        return; /* return to the caller, user can chose another model/action */
    }
        

    vertex_to_string(left, model.info.min_x);
    vertex_to_string(right, model.info.max_x);
    vertex_to_string(up, model.info.max_y);
    vertex_to_string(down, model.info.min_y);
    vertex_to_string(front, model.info.max_z);
    vertex_to_string(back, model.info.min_z);

    face_to_string(largest, model.info.biggest_face);
    face_to_string(smallest, model.info.smallest_face);

    printf(
            STR_COL_ATT(
                "\nInfo on current model: %s\n",
                ANSI_COL_GRE,
                ANSI_ATT_BOLD)
            STR_COL(
                "  vertices number: %d\n"
                "  faces number:    %d\n"
                "  total surface:   %g\n"
                "  total volume:    %g\n"
                "\n"
                "  left extreme:    %s\n"
                "  right extreme:   %s\n"
                "  up extreme:      %s\n"
                "  down extreme:    %s\n"
                "  front extreme:   %s\n"
                "  back extreme:    %s\n"
                "\n"
                "  largest face:    %s\n"
                "  smallest face:   %s\n",
                ANSI_COL_GRE),
            model.filename,
            model.n_vertices,
            model.n_faces,
            model.info.tot_surface,
            model.info.volume,
            left,
            right,
            up,
            down,
            front,
            back,
            largest,
            smallest
            );
}

/*!
 * Check if the value passed as a parameter is a valid RGB component, i.e. if
 * it is inside the range 0-255. Return zero if so, or print a message on 
 * screen and return nonzero to the caller otherwise.
 */
int check_component(int c)
{
    if (c < 0 || c > 255)
    {
        printf(STR_COL_ATT(
                    "Invalid value. Component must be inside [0-255]. "
                    "Please retry.\n",
                    ANSI_COL_RED,
                    ANSI_ATT_BOLD));
        return 1;
    }

    return 0;
}

/*!
 * Ask the user for the insertion of the three components of a RGB color,
 * validating input for each component with the function check_component(int).
 */
ColorRGB ask_color(void)
{
    int flag;
    ColorRGB c;

    printf("\nInsert three color components (integer values only)\n");

    /* red */
    flag = 0;
    do
    {
        printf(STR_ATT("red [0-255]: ", ANSI_ATT_BOLD));
        scanf("%d", &c.r);
        clear_stdin();
        flag = check_component(c.r); /* validate input */
    } while (flag);

    /* green */
    do
    {
        printf(STR_ATT("green [0-255]: ", ANSI_ATT_BOLD));
        scanf("%d", &c.g);
        clear_stdin();
        flag = check_component(c.g);
    } while (flag);

    /* blue */
    do
    {
        printf(STR_ATT("blue [0-255]: ", ANSI_ATT_BOLD));
        scanf("%d", &c.b);
        clear_stdin();
        flag = check_component(c.b);
    } while (flag);

    return c;
}

/*!
 * Ask the user for the insertion of the three coordinates (float values) of
 * a point in tridimensional space.
 */
Point3D ask_point(void)
{
    Point3D p;

    printf("\nInsert three point coordinates\n");

    /* x */
    printf(STR_ATT("x (to right): ", ANSI_ATT_BOLD));
    scanf("%f", &p.x);
    clear_stdin();

    /* y */
    printf(STR_ATT("y (to up): ", ANSI_ATT_BOLD));
    scanf("%f", &p.y);
    clear_stdin();

    /* x */
    printf(STR_ATT("z (to front): ", ANSI_ATT_BOLD));
    scanf("%f", &p.z);
    clear_stdin();

    return p;
}

/*!
 * Ask the user for the insertion of the three components (float values) of
 * a vector in tridimensional space. Vector components cannot be all equal to
 * zero.
 */
Point3D ask_vector(void)
{
    Point3D p;
    int is_null;

    printf("\nInsert three components of a vector\n");

    do /* while (is_null) */
    {
        /* x */
        printf(STR_ATT("x (to right): ", ANSI_ATT_BOLD));
        scanf("%f", &p.x);
        clear_stdin();

        /* y */
        printf(STR_ATT("y (to up): ", ANSI_ATT_BOLD));
        scanf("%f", &p.y);
        clear_stdin();

        /* x */
        printf(STR_ATT("z (to front): ", ANSI_ATT_BOLD));
        scanf("%f", &p.z);
        clear_stdin();

        /* check if vector is null (save expression value used twice) */
        is_null =      fabs(p.x) < NUM_TOL // numerical comparison
                    && fabs(p.y) < NUM_TOL 
                    && fabs(p.z) < NUM_TOL;
                    
        if (is_null)
            printf(STR_COL_ATT(
                        "\nError: vector cannot be null. Please retry.\n",
                        ANSI_COL_RED,
                        ANSI_ATT_BOLD));

    } while (is_null);

    return p;
}
/*!
 * Ask the user for the insertion of an angle in degrees, then return the 
 * value converted into radians.
 */
float ask_angle(void)
{
    float angle;

    printf(STR_ATT("\nAngle (degrees): ", ANSI_ATT_BOLD));
    scanf("%f", &angle);
    clear_stdin();

    /* convert into radians */
    angle *= PI / 180.0f;

    return angle;
}

/*!
 * Show a menu and ask the user a direction.
 */
Direction ask_direction(void)
{
    Direction choice;

    printf( "\nDirection for coloration\n"
            "  1: from right to left\n"
            "  2: from left to right\n"
            "  3: from top to bottom\n"
            "  4: from bottom to top\n"
            "  5: from back to front\n"
            "  6: from front to back\n"
            STR_ATT("Chose a direction [1-6]: ", ANSI_ATT_BOLD));
    do
    {
        scanf("%d", &choice);
        clear_stdin();
    } while (choice < 1 || choice > 6);

    return choice;
}
    
/*!
 * This function takes in input a Model3D object and an Action, then it applies
 * the desired coloration. It returns zero if all operations were fine,
 * nonzero otherwise.
 *
 * This function handles messages and interaction with user, in order to
 * mantain independent from the interface, and then more reusable, the functions 
 * which actually computes the coloration.
 */                
int color_model(Model3D model, Action action)
{
    int status = 0;
    char message[STR_LEN + 1];
    ColorRGB c;
    Direction d;
    Point3D p;
    
    switch (action) 
    {
        /* flat coloration */
        case COL_FLAT:
            printf( "\nFlat coloration\n"
                    "All model vertexes will be colored with the desired color\n"
                    );
            /* ask the user a color, then color the model */
            c = ask_color();
            status = color_flat(model, c);
            /* verbose */
            printf(STR_COL_ATT(
                    "\nApplied flat coloration.\n"
                    "Color: (%d, %d, %d)\n",
                    ANSI_COL_GRE,
                    ANSI_ATT_BOLD),
                c.r,
                c.g,
                c.b);
            return status;

        /* gradient coloration */
        case COL_GRAD:
            printf( "\nGradient coloration\n"
                    "Model vertexes will be colored with a color starting from "
                    "the desired value\nand fading to white along the "
                    "selected direction.\n");
            /* ask the user a color and a direction,
             * then color the model */
            c = ask_color();
            d = ask_direction();
            status = color_gradient(model, c, d);
            /* verbose */
            if (status)
            {
                /* failure */
                printf(STR_COL_ATT(
                        "Error: the model is flat respect to the chosen "
                        "direction, so this kind\nof coloration is not "
                        "applicable",
                        ANSI_COL_RED, 
                        ANSI_ATT_BOLD));
            }
            else
            {
                /* success */
                switch (d)
                {
                    case RL:
                        strcpy(message, "right to left");
                        break;
                    case LR:
                        strcpy(message, "left to right");
                        break;
                    case TB:
                        strcpy(message, "top to bottom");
                        break;
                    case BT:
                        strcpy(message, "bottom to top");
                        break;
                    case FB:
                        strcpy(message, "front to back");
                        break;
                    case BF:
                        strcpy(message, "back to front");
                        break;
                }
                printf(STR_COL_ATT(
                        "\nApplied %s gradient coloration.\n"
                        "Starting color: (%d, %d, %d)\n",
                        ANSI_COL_GRE,
                        ANSI_ATT_BOLD),
                       message,
                       c.r,
                       c.g,
                       c.b);
            }                
            return status;

        /* distance based coloration */
        case COL_DIST:
            printf( "\nDistance based coloration\n"
                    "Model vertexes will be colored with a fading color. "
                    "The desired color\nwill be applied to the vertex nearest "
                    "to the chosed point, the other\nvertexes are colored with "
                    "a hue which fades to white according to\n"
                    "the distance respect to such point.\n"
                    );
            /* ask the user a color and a point, 
             * then color the model */
            c = ask_color();
            p = ask_point();
            status = color_distance(model, c, p);
            if (status)
            {
                /* failure */
                printf(STR_COL_ATT(
                        "Error: all model vertexes have equal distance respect "
                        "to the chosen\npoint, so this kind of coloration is "
                        "undefined.\n",
                        ANSI_COL_RED,
                        ANSI_ATT_BOLD));
            }
            else
            {
                /* success */
                printf(STR_COL_ATT(
                        "\nApplied distance based coloration.\n"
                        "Starting point: (%g, %g, %g)\n"
                        "Starting color: (%d, %d, %d)\n",
                        ANSI_COL_GRE,
                        ANSI_ATT_BOLD),
                    p.x,
                    p.y,
                    p.z,
                    c.r,
                    c.g,
                    c.b);
            }
            return status;

        /* random coloration */
        case COL_RAND:
            printf( "\nRandom coloration\n"
                    "All model vertexes are colored with a random color.\n"
                    );
            status = color_random(model);
            /* verbose */
            printf(STR_COL_ATT(
                    "\nApplied random coloration.\n",
                    ANSI_COL_GRE,
                    ANSI_ATT_BOLD));
            return status;

        /* other values: invalid */
        default:
            /* should be unreachable if code is ok*/
            break;
    }

    /* should be unreachable if code is ok*/
    perror("color_model(): invalid action as parameter");
    exit(EXIT_FAILURE);
}

/*!
 * Ask the user for details (axis base point and direction, angle), then apply
 * the desired rotation to the model.
 */
int rotate_model(Model3D m)
{
    int status = 0;
    Point3D o;   /* rotation axis base point */
    Vector3D u;  /* rotation axis direction */
    float theta; /* rotation angle (in radians) */

    printf( "\nRotation\n"
            "The model will rotate by the desired angle, around an axis\n"
            "passing for the provided point and with the desired direction.\n");

    o = ask_point();
    u = ask_vector();
    theta = ask_angle();

    /* apply rotation */
    status = rotation(m, o, u, theta);

    /* update vertices info, modified due to rotation */
    rescan_vertices_info(&m);

    /* verbose */
    printf(STR_COL_ATT(
                "\nModel rotated by %g degrees around an axis passing for\n"
                "(%g, %g, %g) with direction (%g, %g, %g).\n",
                ANSI_COL_GRE,
                ANSI_ATT_BOLD),
            theta * 180.0f / PI,
            o.x,
            o.y,
            o.z,
            u.x,
            u.y,
            u.z);

    return status;
}

/*!
 * Ask the user to confirm an action, showing a warning message.
 */
Action confirm(Action a)
{
    char reply; /* variable for user reply */

    /* show warning */
    printf(STR_COL_ATT(
                "\nWarning: if you do so, the changes made "
                "to the current model will be lost.\n",
                ANSI_COL_YEL,
                ANSI_ATT_BOLD));

    /* get user response */
    do
    {
        printf(STR_COL_ATT(
                    "Are you sure? [y/n]: ",
                    ANSI_COL_YEL,
                    ANSI_ATT_BOLD));

        scanf("%c", &reply);
        clear_stdin();


        if (reply != 'y' && reply != 'n')
            printf(STR_COL_ATT(
                        "Invalid choice. Retry.\n",
                        ANSI_COL_RED,
                        ANSI_ATT_BOLD));

    } while (reply != 'y' && reply != 'n');

    if (reply == 'n')
        return NULL_ACTION;

    return a;
}
