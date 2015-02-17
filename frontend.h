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
 * \file frontend.h
 * @author Martino Pilia
 * @date 2015-01-24
 */

#include "backend.h"

/*!
 * \brief Ask the user for the name of the file to be opened.
 * @param commandPath Value of argv[0] from the caller.
 * @param f String, will be filled with the filename.
 * @note The filename has a maximum length of STR_LEN, path included.
 */
void get_input_filename(char *f);

/*!
 * \brief Ask the user for the name of the file in which save the edited model.
 * @param commandPath Value of argv[0] from the caller.
 * @param f String, will be filled with the filename.
 * @note The filename has a maximum length of STR_LEN, path included.
 */
void get_output_filename(char *f);

/*!
 * \brief Show the main menu and get the desired action from the user.
 * @param m Model currently loaded.
 * @return The action chosen by the user.
 */
Action main_menu(Model3D m);

/*!
 * \brief Ask the user for exit or another model processing.
 * @return Zero for exit, nonzero otherwise.
 */
int ask_exit(void);

/*!
 * \brief Show informations about the current model.
 * @param m Model from which show informations.
 */
void show_info(Model3D m);

/*!
 * \brief Check if a RGB color component is valid or not.
 * @param c Value of the component to be checked..
 * @return Zero if component is inside [0-255], nonzero otherwise.
 */
int check_component(int c);

/*!
 * \brief Ask the user for a color.
 * @return A ColorRGB structure containing the color chosen by the user.
 */
ColorRGB ask_color(void);

/*!
 * \brief Ask the user for a point in space.
 * @return A Point3D structure containing the point chosen by the user.
 */
Point3D ask_point(void);

/*!
 * \brief Ask the user for a non-null vector in space.
 * @return A Point3D structure containing the coordinates chosen by the user.
 */
Vector3D ask_vector(void);

/*!
 * \brief Ask the user for a angle in degrees.
 * @return The value chosen by the user, converted in radians.
 */
float ask_angle(void);

/*!
 * \brief Ask the user for a direction.
 * @return The Direction value chosen by the user.
 */
Direction ask_direction(void);

/*!
 * \brief Apply the desired coloration to the model.
 * @param m Model to be colored.
 * @param a The desired kind of coloration.
 * @return Zero if coloration was fine, nonzero otherwise.
 * @note Before calling this function, srand(unsigned int) must be called 
 * at least once in order to inizialize the random number generator, if the
 * color_random(Model3D) coloration is required.
 */
int color_model(Model3D m, Action a);

/*!
 * \brief Apply a rotation to the model, asking details to the user.
 * @param m Model to be rotated.
 * @return Zero if rotation was fine, nonzero otherwise.
 */
int rotate_model(Model3D m);

/*!
 * \brief Ask for confirmation for the chosen action.
 * @param a Chosen action.
 * @return NULL_ACTION if user does not confirm, the input parameter 
 * value otherwise.
 */
Action confirm(Action a);
