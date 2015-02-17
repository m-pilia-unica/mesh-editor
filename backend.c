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
 * \file backend.c
 */

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "backend.h"

/*!
 * Open file in OSX, using a path relative to the executable location.
 */
FILE* osx_open_file(char *fn, int dim_file, char *exe_path, char *mode)
{
    int path_len = strlen(exe_path);
    char new_path[path_len + dim_file + 1];
    int i = path_len;

    /* delete executable name, leaving only its containing directory */
    while (exe_path[i] != '/' && i > 0)
        i--;
    exe_path[i] = '\0';
    
    strcpy(new_path, exe_path);
    strcat(new_path, fn);
    
    return fopen(new_path, mode);
}

/*!
 * Get model data from file.
 */
int parse_model_data(Model3D *m)
{
    int line;
    char s[STR_LEN + 1];
    int v_counter = 0, f_counter = 0;
    
    /* parse file header */
    while (
            !feof(m->file_model) 
            && strcmp(s, "end_header\n") != 0)
    {
        fgets(s, STR_LEN, m->file_model);

        if (strstr(s, "element vertex"))
            sscanf(s, "%*s %*s %d", &m->n_vertices);

        if (strstr(s, "element face"))
            sscanf(s, "%*s %*s %d", &m->n_faces);

    }

    /* check if file content is useful */
    if (m->n_faces == 0 || m->n_vertices == 0 )
        return PARSER_ERR_NO_DATA; 

    /* init hash table for vertices */
    line = __LINE__ + 2;
    m->vertices_array = 
        (Vertex**) malloc(m->n_vertices * sizeof (Vertex*));

    if (m->vertices_array == NULL)
        error_handler("malloc",__func__,  __FILE__, line);

    /* read vertices */
    while (v_counter < m->n_vertices)
    {
        /* read line from file */
        if(!fgets(s, STR_LEN, m->file_model))
            break; /* EOF */

        #ifdef __DEBUG__
        printf("%s", s);
        #endif // __DEBUG__

        Vertex v; /* temp variable for the current vertex */
        Vertex *v_p; /* temp variable, will refer to the actual
                        allocation of current vertex */

        v.index = v_counter; /* assign v index */

        /* get components of position and normal */
        sscanf(s, "%f %f %f %f %f %f",
                &v.vertexCoordinates.x,
                &v.vertexCoordinates.y,
                &v.vertexCoordinates.z,
                &v.vertexNormals.x,
                &v.vertexNormals.y,
                &v.vertexNormals.z
              );

        /* add vertex v to the vertex list of the model */
        v_p = vertex_add(m, v, v_counter);

        v_counter++;
    }

    /* read faces */
    while (f_counter < m->n_faces)
    {
        /* read one line from file */
        if(!fgets(s, STR_LEN, m->file_model))
            break; /* EOF */

        #ifdef __DEBUG__
        printf("%s", s);
        #endif // __DEBUG__

        Face f; /* temp variable for current face */
        Face *f_p; /* temp variable for a pointer 
                      to allocation of current face */

        f.index = f_counter; /* set index */

        /* get face vertex indices */
        sscanf(s, "%*d %d %d %d",
                &f.v1,
                &f.v2,
                &f.v3
              );

        /* 
         * Set pointers to vertices, use array to have faster access than list's
         */
        f.v1p = m->vertices_array[f.v1];
        f.v2p = m->vertices_array[f.v2];
        f.v3p = m->vertices_array[f.v3];

        /* add face to list */
        f_p = face_add(m, f);

        f_counter++;
    }

    printf("\n%d %d\n", m->n_faces, f_counter);

    /* read data does not match with header file declaration (there are 
     * missing or extra data lines in the file) */
    if (f_counter != m->n_faces)
        return PARSER_ERR_INCOHERENT_DATA;

    /* if one of these two is still NULL something terrible happened, 
     * and I don't wanna know what */
    assert(m->faces_list != NULL && m->vertices_list != NULL);

    #ifdef __DEBUG__
    printf("\nVertices: %d \nFaces: %d\n",
            m->n_vertices,
            m->n_faces);
    #endif // __DEBUG__

    return 0;
}

/*!
 * Set up to default value some model parameters. Initial setup with default 
 * values is needed for the next model processing.
 */
void init_model(Model3D *m)
{
    m->faces_list = NULL;
    m->vertices_list = NULL;
    m->n_vertices = 0;
    m->n_faces = 0;
    m->info.max_x = NULL;
    m->info.min_x = NULL;
    m->info.max_y = NULL;
    m->info.min_y = NULL;
    m->info.max_z = NULL;
    m->info.min_z = NULL;
    m->info.biggest_face = NULL;
    m->info.smallest_face = NULL;
    m->info.tot_surface = 0;
}

/*!
 * All'interno di questa funzione sara' possibile salvare il modello
 * opportunatamente modificato.
 */
void save_model(FILE *newfile, Model3D m)
{
    Vertex *v = m.vertices_list;
    Face *f = m.faces_list;

    /* print header on file */
    fprintf(newfile,
            "ply\n"
            "format ascii 1.0\n"
            "comment %s\n"
            "element vertex %d\n"
            "property float x\n"
            "property float y\n"
            "property float z\n"
            "property float nx\n"
            "property float ny\n"
            "property float nz\n"
            "property uchar red\n"
            "property uchar green\n"
            "property uchar blue\n"
            "element face %d\n"
            "property list uchar int vertex_indices\n"
            "end_header\n",
            PLY_OUTPUT_COMMENT,
            m.n_vertices,
            m.n_faces
            );

    /* print vertices on file */
    while (v != NULL)
    {
        /* print components of coords and normals for the current vertex */
        fprintf(newfile,
                "%g %g %g %g %g %g %d %d %d\n",
                v->vertexCoordinates.x,
                v->vertexCoordinates.y,
                v->vertexCoordinates.z,
                v->vertexNormals.x,
                v->vertexNormals.y,
                v->vertexNormals.z,
                v->vertexColor.r,
                v->vertexColor.g,
                v->vertexColor.b
                );

        v = v->next;

        #ifdef __DEBUG__
        fflush(newfile);
        #endif // __DEBUG__
    }

    /* print faces on file */
    f = m.faces_list;
    while (f != NULL)
    {
        fprintf(newfile,
                "%d %d %d %d\n",
                3,
                f->v1,
                f->v2,
                f->v3
                );

        f = f->next;

        #ifdef __DEBUG__
        fflush(newfile);
        #endif // __DEBUG__
    }

    fflush(newfile);
}

/*!
 * Apply a flat coloration to the input Model3D object, using the provided
 * Color. The model will have the same color, passed as parameter, applied to
 * each of its vertexes. If coloration is done succesfully, the function shows 
 * a verbose message.
 */
int color_flat(Model3D model, ColorRGB c)
{
    Vertex *v = model.vertices_list;

    while (v != NULL)
    {
        v->vertexColor = c;
        v = v->next;
    }

    return 0;
}

/*!
 * Apply a random coloration to the Model3D object provided in input. A random
 * generated color is applied to each vertex of the input model. If coloration 
 * is done succesfully, the function shows a verbose message.
 */
int color_random(Model3D model)
{
    Vertex *v = model.vertices_list;

    while (v != NULL)
    {
        /* generate random coordinates 
         * srand(unsigned int) must be called before using this function */
        v->vertexColor.r = rand() % 256;
        v->vertexColor.g = rand() % 256;
        v->vertexColor.b = rand() % 256;

        v = v->next;
    }

    return 0;
}

/*!
 * Apply a distance-based coloration to the input Model3D object, referred 
 * to the input Point3D and ColorRGB objects. The model vertex nearest to the
 * input point is associated with the input color, the farthest vertex from it
 * is associated to white (255, 255, 255), and remaining vertices are colored 
 * with a linear combinations of the two colors, proportional to theyr distance
 * from the input point, according with the following formula:
 * 
 * \f[ p = q + \frac{d_v - d_{min}}{d_{max} - d_{min}} \cdot (255 - q) \f]
 * 
 * where \f$ d_v \f$ is the distance of the vertex from the input point,
 * \f$ d_{min} \f$ is the distance between the nearest model vertex and such 
 * point, \f$ d_{max} \f$ is the distance between the farthest model vertex and
 * it, \f$ p \f$ is a generic color component of the vertex and \f$ q \f$ is 
 * the respective component from the input color.
 * 
 * If all the model's vertices are equidistant from the chosen point, the 
 * previous formula is meaningless. In fact, in a such situation, this kind of
 * coloration is undefined because all vertices are at the same time the most 
 * and least distant, so they should assume two colors at the same time. In
 * this situation, the coloration is aborted and a message is shown to
 * the user, returning to the caller a nonzero value (which permits to the 
 * caller to ask the user another action to do).
 */
int color_distance(Model3D m, ColorRGB c, Point3D p)
{
    Vertex *v = m.vertices_list;
    double distance[m.n_vertices]; /* store values because are used twice */
    double distance_range; /* d_max - d_min */
    int max = 0, min = 0;  /* d_max and d_min indexes */
    float coef; /* coefficient for coloration */
    int i = 0;

    /* compute distance between each model point and p */
    while (v != NULL)
    {
        distance[i] = euclidean_distance(p, v->vertexCoordinates);

        if (distance[i] > distance[max])
            max = i;
        if (distance[i] < distance[min])
            min = i;

        v = v->next;
        i++;
    }

    /* check if all vertexes have equal distance from the chosen point;
     * if so, abort coloration with error */
    if (distance[max] == distance[min])
        return -1;

    /* compute d_max - d_min, a value reused a lot */
    distance_range = distance[max] - distance[min];

    /* color the vertexes
     * cannot do this in previous cycle, because depends of the
     * values of farthest and nearest */
    i = 0;
    v = m.vertices_list;
    while (v != NULL)
    {
        coef = (distance[i] - distance[min]) / distance_range;

        v->vertexColor.r = c.r + coef * (255 - c.r);
        v->vertexColor.g = c.g + coef * (255 - c.g);
        v->vertexColor.b = c.b + coef * (255 - c.b);
        
        v = v->next;
        i++;
    }

    return 0;
}

/*!
 * Apply a gradient coloration to the input Model3D object, referred 
 * to the input Direction and ColorRGB objects. The model vertex with lowest
 * coordinate component along the (oriented) direction in input assumes the
 * input color, while the vertex with highest coordinate component assumes the
 * white color (255, 255, 255). Other vertices are colored according to the
 * following formula:
 * 
 * \f[ p = q + \frac{x - x_{min}}{x_{max} - x_{min}} \cdot (255 - q) \f]
 * 
 * where \f$ x \f$ is the coordinate's component of the vertex along the 
 * (oriented) direction of fading, \f$ x_{min} \f$ is minimum position 
 * component in such direction among all vertices, \f$ x_{max} \f$ similarly
 * is the maximum component, \f$ p \f$ is a generic color component of the 
 * vertex and \f$ q \f$ is the respective component from the input color.
 * 
 * If the model is flat along the input direction, i.e. all points have the 
 * same component in that direction, so this coloration is undefined, because
 * all vertices have at the same time the highest and lowest value of
 * such directional component, so they should assume two different colors at
 * the same time. In this situation, the coloration is aborted and a message is 
 * shown to the user, returning to the caller a nonzero value (which permits to 
 * the caller to ask the user another action to do).
 */
int color_gradient(Model3D m, ColorRGB c, Direction d)
{
    Vertex *v = m.vertices_list;
    float coef;
    int flag = 0;

    /* check if the model is flat respect to the axis chosen for gradient
     * coloration */
    switch (d)
    {
        /* gradient along x axis */
        case RL:
        case LR:
            if ((m.info.max_x->vertexCoordinates.x 
                    - m.info.min_x->vertexCoordinates.x) < NUM_TOL)
                flag = 1;
            break;

        /* gradient along y axis */
        case TB:
        case BT:
            if ((m.info.max_y->vertexCoordinates.y 
                    - m.info.min_y->vertexCoordinates.y) < NUM_TOL)
                flag = 1;
            break;

        /* gradient along z axis */
        case FB:
        case BF:
            if ((m.info.max_z->vertexCoordinates.z 
                    - m.info.min_z->vertexCoordinates.z) < NUM_TOL)
                flag = 1;
            break;

        default:
            /* should be unreachable if input is valid */
            perror("color_gradient(): invalid value for direction");
            exit(EXIT_FAILURE);
            break;
    }

    /* if so, abort coloration with error */
    if (flag)
        return -1;

    /* color vertices */
    while (v != NULL)
    {
        coef = distance_coefficient(m, v, d);
        v->vertexColor.r = c.r + coef * (255 - c.r);
        v->vertexColor.g = c.g + coef * (255 - c.g);
        v->vertexColor.b = c.b + coef * (255 - c.b);

        v = v->next;
    }

    return 0;
}

/*!
 * Compute the coefficient defining color value in a vertex, defined as:
 *
 * \f[ \frac{x - x_{min}}{x_{max} - x_{min}} \f]
 * 
 * where \f$ x \f$ is the coordinate's component of the vertex along the 
 * (oriented) direction of fading, \f$ x_{min} \f$ is minimum position 
 * component in such direction among all vertices, \f$ x_{max} \f$ similarly
 * is the maximum component.
 */
float distance_coefficient(Model3D m, Vertex *v, Direction d)
{
    float coef;

    switch (d)
    {
        /* color right to left */
        case RL:
            coef = 1 -
                (v->vertexCoordinates.x 
                 - m.info.min_x->vertexCoordinates.x) 
                / (m.info.max_x->vertexCoordinates.x 
                        - m.info.min_x->vertexCoordinates.x);
            break;

            /* color left to right */
        case LR:
            coef = 
                (v->vertexCoordinates.x 
                 - m.info.min_x->vertexCoordinates.x) 
                / (m.info.max_x->vertexCoordinates.x 
                        - m.info.min_x->vertexCoordinates.x);
            break;

            /* color up to down */
        case TB:
            coef = 1 -
                (v->vertexCoordinates.y 
                 - m.info.min_y->vertexCoordinates.y) 
                / (m.info.max_y->vertexCoordinates.y 
                        - m.info.min_y->vertexCoordinates.y);
            break;

            /* color down to up */
        case BT:
            coef = 
                (v->vertexCoordinates.y 
                 - m.info.min_y->vertexCoordinates.y) 
                / (m.info.max_y->vertexCoordinates.y 
                        - m.info.min_y->vertexCoordinates.y);
            break;

            /* color front to back */
        case FB:
            coef = 1 -
                (v->vertexCoordinates.z 
                 - m.info.min_z->vertexCoordinates.z) 
                / (m.info.max_z->vertexCoordinates.z 
                        - m.info.min_z->vertexCoordinates.z);
            break;

            /* color back to front */
        case BF:
            coef = 
                (v->vertexCoordinates.z 
                 - m.info.min_z->vertexCoordinates.z) 
                / (m.info.max_z->vertexCoordinates.z 
                        - m.info.min_z->vertexCoordinates.z);
            break;

        default:
            /* unreachable */
            break;
    }

    return coef;
}

/*!
 * This procedure asks the <code>stdin</code> one character a time, until it 
 * reaches <code>EOF</code> or it gets an <code>endline</code> character.
 * This subroutine is blocking, hanging for input, if it is called while 
 * <code>stdin</code> is already empty.
 */
void clear_stdin(void)
{
    char c;
    while ((c = getchar()) != EOF && c != '\n') {}
}

/*!
 * This procedure computes a matrix row by column product between a 3x3 square 
 * matrix and a column vector, saving the result on another vector.
 */
void apply_transformation_matrix(float y[3], float a[3][3], float x[3])
{
    int i, j;
    
    /* row by column product */
    for (i = 0; i < 3; ++i)
    {
        y[i] = 0;
        for (j = 0; j < 3; ++j)
            y[i] += a[i][j] * x[j];
    }
}

/*!
 * This function applies an arbitrary rotation to the input model. The 
 * rotation is defined by a rotation axis (given by a point \f$ \mathbf{o} \f$ 
 * belonging to it and a vector \f$ \mathbf{u} = (u_x,\, u_y,\, u_z) \f$ 
 * representing its direction) and an angle \f$ \theta \f$ (with the usual 
 * convention for the sign: positive for counterclockwise rotation, 
 * according to the right hand rule, negative for clockwise rotation).
 * 
 * The model is first translated by \f$ -\mathbf{o} \f$, bringing the rotation
 * axis on the origin. Then, if the rotation axis coincides with the \f$ x \f$
 * axis, an elementary rotation is performed around it, according to the
 * following rotation matrix \f$ \mathbf{R}_\theta \f$:
 * 
 * \f[
 *    \mathbf{R}_\theta =
 *      \begin{pmatrix}
 *          1    & 0                 & 0              \\
 *          0    & \cos(\theta)      & -\sin(\theta)  \\
 *          0    & \sin(\theta)      & \cos(\theta)
 *      \end{pmatrix}
 * \f]
 * 
 * Otherwise, a couple of elementary rotations \f$ \mathbf{R}_x \f$ and
 * \f$ \mathbf{R}_y \f$ is performed (where \f$ q = \sqrt{u_y^2 + u_z^2} \f$):
 * 
 * \f[
 *    \mathbf{R}_x =
 *      \begin{pmatrix}
 *          1    & 0                 & 0              \\
 *          0    & \frac{u_z}{q}     & -\frac{u_y}{q} \\
 *          0    & \frac{u_y}{q}     & \frac{u_z}{q}
 *      \end{pmatrix}
 * \f]
 * 
 * \f[
 *    \mathbf{R}_y =
 *      \begin{pmatrix}
 *          u_x      & 0       & q     \\
 *          0        & 1       & 0     \\
 *          -q       & 0       & u_x
 *      \end{pmatrix}
 * \f]
 * 
 * The former brings the rotation axis into the \f$ xz \f$ plane, and then
 * the latter superimposes it to the \f$ x \f$ axis. At this point, the 
 * elementary rotation \f$ \mathbf{R}_\theta \f$ is performed, then the 
 * rotation axis is restored to its original position, multiplying in order 
 * for \f$ \mathbf{R}_y^{-1} \f$ and \f$ \mathbf{R}_x^{-1} \f$.
 * 
 * Finally, the model is translated back by \f$ \mathbf{o} \f$.
 */
int rotation(Model3D m, Point3D o, Vector3D u, float theta)
{
    /* norm of the input vector */
    float norm = sqrt(u.x * u.x + u.y * u.y + u.z * u.z);
    
    /* normalized vector components */
    float x = u.x / norm;
    float y = u.y / norm;
    float z = u.z / norm;
    
    /* useful coefficients */
    float q2 = y*y + z*z;
    float s = sin(theta);
    float c = cos(theta);
    
    /* rotation matrix */
    float rot_x[3][3] = /* around x axis: elementary rotation matrix R_th */
    {
        {1, 0,  0},
        {0, c, -s},
        {0, s,  c}
    };
    float rot[3][3] = /* around any other axis: (Ry*Rx)^-1 * R_th * (Ry*Rx) */
    {
        {
            c * q2 + x * x,
            (1 - c) * x*y - s*z,
            (1 - c) * x*z + s*y
        },
        {
            (1 - c) * x*y + s*z,
            y*y + (x*y * (c*x*y - s*z) + z * (s*x*y + c*z)) / q2,
            y*z + (x*z * (c*x*y - s*z) - y * (s*x*y + c*z)) / q2,
        },
        {
            (1 - c) * x*z - s*y,
            y*z + (z * (s*x*z - c*y) + x*y * (s*y + c*x*z)) / q2,
            z*z + (x*z * (s*y + c*x*z) - y * (s*x*z - c*y)) / q2,
        }
    };
    float input[3], result[3];
    Vertex *v = m.vertices_list;
    
    while (v != NULL)
    {
        /* rotate vertex */
        input[0] = v->vertexCoordinates.x - o.x;
        input[1] = v->vertexCoordinates.y - o.y;
        input[2] = v->vertexCoordinates.z - o.z;
        if (fabs(y) < NUM_TOL && fabs(z) < NUM_TOL)
            apply_transformation_matrix(result, rot_x, input);
        else
            apply_transformation_matrix(result, rot, input);
        v->vertexCoordinates.x = result[0] + o.x;
        v->vertexCoordinates.y = result[1] + o.y;
        v->vertexCoordinates.z = result[2] + o.z;
        
        /* rotate normal */
        input[0] = v->vertexNormals.x - o.x;
        input[1] = v->vertexNormals.y - o.y;
        input[2] = v->vertexNormals.z - o.z;
        if (fabs(y) < NUM_TOL && fabs(z) < NUM_TOL)
            apply_transformation_matrix(result, rot_x, input);
        else
            apply_transformation_matrix(result, rot, input);
        v->vertexNormals.x = result[0] + o.x;
        v->vertexNormals.y = result[1] + o.y;
        v->vertexNormals.z = result[2] + o.z;
        
        v = v->next;
    }

    return 0;
}

/*!
 * This procedure performs a new search of the vertexes with minimum and
 * maximum value for each coordinate. A first search is performed directly
 * inside the parser, but this procedure is useful when the model has been
 * modified, i.e. with a rotation. Note that there's no need to search for
 * the maximum and minimum surfaces, because surfaces are not altered by 
 * isometric transformations.
 */
void rescan_vertices_info(Model3D *m)
{
    Vertex *v = m->vertices_list;

    /* reset vertices info */
    m->info.min_x = NULL;
    m->info.max_x = NULL;
    m->info.min_y = NULL;
    m->info.max_y = NULL;
    m->info.min_z = NULL;
    m->info.max_z = NULL;

    /* rescan */
    while (v != NULL)
    {
        update_vertices_info(m, v);
        v = v->next;
    }
}


/*!
 * This function adds a vertex in the vertex list of the input Model3D object,
 * allocating it in a dynamical memory region. List items are also indexed 
 * with an associative array (indexed by their index number), in order 
 * to provide faster nonsequential access to the list items.
 *
 * The list tail, i.e. the last added element, is also memorized in the
 * Model3D object, in order to eliminate the needing for list flow when adding
 * a new item.
 */
Vertex* vertex_add(Model3D *model, Vertex item, int index)
{
    int line;
    Vertex *v;

    if (model == NULL)
    {
        /* should be unreachable if code is ok */
        perror("vertex_add(): NULL pointer to Model3D object");
        exit(EXIT_FAILURE);
    }

    line = __LINE__ + 1;
    v = (Vertex*) malloc(sizeof (Vertex));
    
    if (v == NULL)
        error_handler("malloc", __func__, __FILE__, line);

    *v = item;
    v->next = NULL;
    model->vertices_array[index] = v; /* add to vertex indexing array */

    if (model->vertices_list == NULL) /* empty list, this is the first item */
    {
        model->vertices_list = v;
        v->prev = NULL;
        model->last_vertex = v;
        return v;
    }

    /* non empty list */
    v->prev = model->last_vertex;
    model->last_vertex->next = v;
    model->last_vertex = v;

    return v;
}

/*!
 * Flow the list until the desired position, and return a pointer to the
 * corresponding vector.
 */
Vertex* vertex_get(Vertex *l, int n)
{
    int i = 0;
    Vertex *v = l;

    if (n < 0)
    {
        /* should be unreachable if caller code is correct */
        perror("vertex_get: invalid negative index");
        return NULL;
    }

    while (i < n)
    {
        if (v == NULL)
        {
            /* should be unreachable if caller code is correct */
            perror("vertex_get: index exceedes list length");
            return NULL;
        }
        v = v->next;
        i++;
    }

    return v;
}

/*!
 * This function adds a face in the faces list of the input Model3D object,
 * allocating it in a dynamical memory region.
 *
 * The list tail, i.e. the last added element, is also memorized in the
 * Model3D object, in order to eliminate the needing for list flow when adding
 * a new item.
 */

Face* face_add(Model3D *model, Face item)
{
    int line;
    Face *f;

    if (model == NULL)
    {
        /* should be unreachable if code is ok */
        perror("vertex_add(): NULL pointer to Model3D object");
        exit(EXIT_FAILURE);
    }

    line = __LINE__ + 1;
    f = (Face*) malloc(sizeof (Face));
    
    if (f == NULL)
        error_handler("malloc",__func__, __FILE__, line);

    *f = item;
    f->next = NULL;

    if (model->faces_list == NULL) /* empty list, this is the first item */
    {
        model->faces_list = f;
        f->prev = NULL;
        model->last_face = f;
        return f;
    }

    /* non empty list */
    f->prev = model->last_face;
    model->last_face->next = f;
    model->last_face = f;

    return f;
}

/*!
 * This procedure checks if the input vertex has some peculiar property, 
 * i.e. it has the highest/lowest value of a coordinate, and in such case the
 * vertex is referenced in the info field of the Model3D object which 
 * belongs to.
 */
void update_vertices_info(Model3D *model, Vertex *v)
{
    /* NULL check relies on standardized evaluation order in C logical 
     * expressions */
    if (model->info.max_x == NULL ||
            v->vertexCoordinates.x > model->info.max_x->vertexCoordinates.x)
        model->info.max_x = v;

    if (model->info.min_x == NULL ||
            v->vertexCoordinates.x < model->info.min_x->vertexCoordinates.x)
        model->info.min_x = v;

    if (model->info.max_y == NULL ||
            v->vertexCoordinates.y > model->info.max_y->vertexCoordinates.y)
        model->info.max_y = v;

    if (model->info.min_y == NULL ||
            v->vertexCoordinates.y < model->info.min_y->vertexCoordinates.y)
        model->info.min_y = v;

    if (model->info.max_z == NULL ||
            v->vertexCoordinates.z > model->info.max_z->vertexCoordinates.z)
        model->info.max_z = v;

    if (model->info.min_z == NULL ||
            v->vertexCoordinates.z < model->info.min_z->vertexCoordinates.z)
        model->info.min_z = v;
}


/*!
 * This procedure checks if the input face has the biggest or the smallest 
 * surface among model faces, and in such case it adds it to the info field 
 * of the Model3D onject. 
 *
 * This procedure updates the total surface of the model too, adding the 
 * surface of the current input face.
 */
void update_faces_info(Model3D *model, Face *f)
{
    triangle_surface(f); /* compute face surface and assign it 
                            to relative f field */

    /* assert f->surface is not NaN */
    assert(f->surface == f->surface);

    model->info.tot_surface += f->surface; /* update total surface */

    if (model->info.biggest_face == NULL || 
            f->surface > model->info.biggest_face->surface)
        model->info.biggest_face = f;

    if (model->info.smallest_face == NULL ||
            f->surface < model->info.smallest_face->surface)
        model->info.smallest_face = f;

}

/*!
 * Compute the euclidean distance between two points in tridimensional
 * space. Euclidean distance is a symmetric, positive definite bilinear
 * for defined as:
 * \f[ 
 *      d_e( \mathbf{x} ,\, \mathbf{y} ) 
 *          = \sqrt{ \sum_{i=i}^3 \left( x_i - y_i \right)^2 }
 * \f]
 */
double euclidean_distance(Point3D p1, Point3D p2)
{
    /* NOTE: squares are computed with product and not with pow()
     * function, for efficiency purpose. */
    double dist, sum;;

    sum = (p2.x - p1.x) * (p2.x - p1.x)
            + (p2.y - p1.y) * (p2.y - p1.y)
            + (p2.z - p1.z) * (p2.z - p1.z);

    /* Ensure that the operand of sqrt is not negative. Despite the 
     * expression of sum cannot assume a negative value analytically,
     * the numerical result may assume a small negative value instead of 
     * zero, due to floating point approximation, causing an EDOM error 
     * in the sqrt(double) function, which returns an implementation defined
     * value (usually NaN). */
    if (sum < 0)
        sum = 0;

    dist = sqrt(sum);

    return dist;
}

/*!
 * Check if three points 
 * \f$ \mathbf{p_i} = (p_{i,1} ,\, p_{i,2} ,\, p_{i,3}) \f$ 
 * belong to the same straight line, computing the 
 * following determinant
 * \f[
 *      \begin{vmatrix}
 *      p_{1,1} & p_{1,2} & p_{1,3} \\
 *      p_{2,1} & p_{2,2} & p_{2,3} \\
 *      p_{3,1} & p_{3,2} & p_{3,3} \\
 *      \end{vmatrix}
 * \f]
 * whose value is zero if the points are linealry dipendent, i.e. aligned,
 * nonzero otherwise.
 */
int aligned(Point3D p1, Point3D p2, Point3D p3)
{
    /* use numerical comparison */
    return fabs(
          p1.x * (p2.y * p3.z - p2.z * p3.y)
        - p1.y * (p2.x * p3.z - p2.z * p3.x)
        + p1.z * (p2.x * p3.y - p3.x * p2.y)
        ) < NUM_TOL;
}

/*!
 * This function computes the area of the input Face object, using Heron's
 * formula. Let  \f$ l_1 ,\, l_2 \; \text{and} \; l_3 \f$ be the three
 * sides of a triangle, and \f$ s = \frac{l_1 + l_2 + l_3}{2} \f$ its
 * semiperimeter; so the area of the triangle is given by:
 * \f[ 
 *      A = \sqrt{s \cdot (s - l_1) \cdot (s - l_2) \cdot (s - l_3) }
 * \f]
 */
double triangle_surface(Face *f)
{
    /* compute sides and semiperimeter */
    double l1 = euclidean_distance(
            f->v1p->vertexCoordinates,
            f->v2p->vertexCoordinates);

    double l2 = euclidean_distance(
            f->v2p->vertexCoordinates,
            f->v3p->vertexCoordinates);

    double l3 = euclidean_distance(
            f->v3p->vertexCoordinates,
            f->v1p->vertexCoordinates);

    double sp = (l1 + l2 + l3) / 2;

    /* compute sqrt operand for Heron's formula */
    double prod = sp * (sp - l1) * (sp - l2) * (sp - l3);
    
    /* Ensure that the operand of sqrt is not negative. Despite the 
     * expression of prod cannot assume a negative value analytically,
     * when the three points are aligned or almost aligned the
     * numerical result may assume a small negative value instead of 
     * zero, due to floating point approximation, causing an EDOM error 
     * in the sqrt(double) function, which returns a NaN value. */
    if (prod < 0)
        f->surface = 0;
    else
        f->surface = sqrt(prod);

    return f->surface;
}

/*!
 * Write a human readable description of the vertex in the string given as 
 * parameter, with the following format specifier:
 * <code> "index: %*d;     coord: (% .*f, % .*f, % .*f)" </code>
 * where the number of decimal and floating point digits are given by
 * INT_DIGITS and FLO_DIGITS macros respectively.
 */
void vertex_to_string(char *s, Vertex *v)
{
    sprintf(s,
            "index: %*d;     coord: (% *.*f, % *.*f, % *.*f )",
            INT_DIGITS,
            v->index,
            FLO_DIGITS,
            FLO_DIGITS / 2,
            v->vertexCoordinates.x,
            FLO_DIGITS,
            FLO_DIGITS / 2,
            v->vertexCoordinates.y,
            FLO_DIGITS,
            FLO_DIGITS / 2,
            v->vertexCoordinates.z
           );
}

/*!
 * Write a human readable description of the vertex in the string given as 
 * parameter, with the following format specifier:
 * <code> "indexes: %*d, %*d, %*d;     area: % .g" </code>
 * where the number of decimal digits are given by the INT_DIGITS macro.
 */
void face_to_string(char *s, Face *f)
{
    sprintf(s,
            "indexes: %*d, %*d, %*d;     area: % .g",
            INT_DIGITS,
            f->v1,
            INT_DIGITS,
            f->v2,
            INT_DIGITS,
            f->v3,
            f->surface
           );
}

/*!
 * This procedure writes an adequately descriptive error message on 
 * <code>stdout</code>, descibing the error encountered, then it 
 * quits the program with <code>EXIT_FAILURE</code> status. 
 * This procedure is intended to be used to write a standardized message 
 * with few lines of code, after the check of the output from a function 
 * call shows a failure, e.g.
 * \code
 *      line_no = __LINE__ + 1;
 *      m = malloc(n * sizeof(int));
 *
 *      if (m == NULL)
 *          error_handler("malloc", __func__, __FILE__, line_no);
 * \endcode
 */
void error_handler(char *fun_name, const char *caller, char *file, int line)
{           
    char message[STR_LEN + 1];
    sprintf(message, "%s:%d: %s: %s() error", 
            file, 
            line,
            caller,
            fun_name);
    perror(message);
    exit(EXIT_FAILURE);
}

/*!
 * This procedure frees all dynamical resources allocated for the input 
 * model.
 */
void clear_model(Model3D m)
{
    Vertex *v1, *v2;
    Face *f1, *f2;

    /* dealloc all vertices */
    v1 = m.vertices_list;
    while (v1 != NULL)
    {
        v2 = v1->next;
        free(v1);
        v1 = v2;
    }

    /* dealloc all faces */
    f1 = m.faces_list;
    while (f1 != NULL)
    {
        f2 = f1->next;
        free(f1);
        f1 = f2;
    }

    /* dealloc vertex associative table */
    free(m.vertices_array);
}

/*!
 * This function computes the value of the mixed product (a.k.a. box product)
 * of three vectors 
 * \f$ \mathbf{a},\, \mathbf{b},\, \mathbf{c} \in \mathbb{R}^3 \f$,
 * defined as \f$ <\mathbf{a},\, \mathbf{b} \times \mathbf{c}> \f$.
 */
double mixed_product(Point3D a, Point3D b, Point3D c)
{
    return  a.x * (b.y*c.z - b.z*c.y)
          + a.y * (b.z*c.x - b.x*c.z)
          + a.z * (b.x*c.y - b.y*c.x);
}

/*!
 * This function computes the volume of a 3D model. It computes the sum of 
 * all signed volumes of the symplexes (thetrahedron) subtended by each face 
 * respect to the origin.
 * 
 * The sign of each symplex volume is positive if the normal of the face is
 * directed toward the opposite semispace respect to that containing the 
 * origin, negative otherwise. If the solid is convex and contains the origin,
 * all the volumes subtended by faces are positive and their sum equals the
 * model volume. Otherwise, the extra positive volume subtended by the 
 * faces farthest from the origin is compensated by the negative volume
 * subtended by the nearest faces whose normal points in the same semispace
 * containing the origin.
 * 
 * The resulting value is obviously meaningless if the model does not represent
 * a closed surface or if the surface is not orientable, or if vertices have
 * not a consistent order with the face normal according to the right hand rule.
 */
double model_volume(Model3D m)
{
    double volume = 0;
    Face *f = m.faces_list;

    while (f != NULL)
    {
        /* the box product is 6 times the volume of the tetrahedron
         * defined by the three vertices */
        volume += mixed_product(
                f->v1p->vertexCoordinates,
                f->v2p->vertexCoordinates,
                f->v3p->vertexCoordinates) / 6.0;
        f = f->next;
    }

    return fabs(volume);
}

/*!
 * This function provides an approximation of the model volume, based on 
 * analytical considerations. It relies on the divergence theorem, which 
 * states that, given a compact set \f$ V \subset \mathbb{R}^3 \f$ with 
 * piecewise smooth boundary \f$ S = \partial V \f$ and a vector field 
 * \f$ \mathbf{F} \f$, defined on a neighborhood of \f$ V \f$ and continuously 
 * differentiable, the following equality holds:
 * \f[
 *      \int_V \text{div} F\; dV = \oint_S <\mathbf{F},\, \mathbf{n}> dS 
 * \f]
 * where \f$ \mathbf{n} \f$ is the surface normal unitary vector field with
 * positive orientation (i.e. pointing outside the surface).
 * 
 * Given a vector field with constant divergence, e.g. 
 * \f$ \mathbf{F} = (x, y, z)\f$ (whose divergence has the constant value of 3),
 * the volume of the set \f$ V \f$ is given as
 * \f[
 *      |V| = \int_V dV 
 *          = \frac{1}{\text{div} F} \oint_S <\mathbf{F},\,\mathbf{n}> dS
 * \f]
 * so an approximation of the right member surface integral gives an 
 * approximation of the model volume.
 * 
 * The resulting value is obviously meaningless if the model does not represent 
 * a closed surface, and the approximation is quite rough if faces are not very
 * small.
 */
double model_volume_approx(Model3D m)
{
    double volume = 0;
    double norm2;
    double norm;
    double dot_prod;
    Vector3D n;
    Point3D g;
    Face *f = m.faces_list;

    /* iterate on model faces */
    while (f != NULL)
    {
        /* compute face centroid g */
        g.x = (  f->v1p->vertexCoordinates.x
               + f->v2p->vertexCoordinates.x
               + f->v3p->vertexCoordinates.x) / 3.0;

        g.y = (  f->v1p->vertexCoordinates.y
               + f->v2p->vertexCoordinates.y
               + f->v3p->vertexCoordinates.y) / 3.0;

        g.z = (  f->v1p->vertexCoordinates.x
               + f->v2p->vertexCoordinates.z
               + f->v3p->vertexCoordinates.z) / 3.0;

        /* compute an approximation n of the face normal, as the 
         * arithmetical mean of its vertices normals 
         * (this is quick but *very rough*) */
        n.x = (  f->v1p->vertexNormals.x 
               + f->v2p->vertexNormals.x 
               + f->v3p->vertexNormals.x );
        
        n.y = (  f->v1p->vertexNormals.y 
               + f->v2p->vertexNormals.y 
               + f->v3p->vertexNormals.y );

        n.z = (  f->v1p->vertexNormals.z 
               + f->v2p->vertexNormals.z 
               + f->v3p->vertexNormals.z );

        /* normalize n */
        norm2 = n.x*n.x + n.y*n.y + n.z*n.z;
        norm = (norm2 < 0 ? 0 : sqrt(norm2));
        n.x /= (norm == 0 ? 1 : norm);
        n.y /= (norm == 0 ? 1 : norm);
        n.z /= (norm == 0 ? 1 : norm);

        /* compute dot product between F(g) and n */
        dot_prod = g.x*n.x + g.y*n.y + g.z*n.z;

        /* multiply dot_prod for the face area, then add the term to the sum */
        volume += dot_prod * f->surface;

        f = f->next;
    }

    /* divide surface integral approximation for the F's divergence value */
    return fabs(volume) / 3.0;
}

/*!
 * This subroutine computes the informations about a 3D model, saving all in
 * its struct Info field.
 */
void model_info(Model3D *m)
{
    Vertex *v = m->vertices_list;
    Face *f = m->faces_list;

    /* determine informations about vertices */
    while (v != NULL)
    {
        update_vertices_info(m, v);
        v = v->next;
    }

    /* determine informations about faces and area */
    while (f != NULL)
    {
        update_faces_info(m, f);
        f = f->next;
    }

    /* determine volume */
    m->info.volume = model_volume(*m);
}
