/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "view_options.h"

#include "opentx.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

ViewOptions::ViewOptions() : Page(ICON_MODEL_SETUP)
{
  header.setTitle(STR_MENU_MODEL_SETUP);
  header.setTitle2(STR_VIEW_OPTIONS);

  body.padAll(8);

  auto form = new FormWindow(&body, rect_t{});
  form->setFlexLayout();
  FlexGridLayout grid(line_col_dsc, line_row_dsc, 2);

  auto line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_MENU_TAB, 0, COLOR_THEME_PRIMARY1);
  new StaticText(line, rect_t{}, STR_VISIBILITY, 0, COLOR_THEME_PRIMARY1);
  line = form->newLine(&grid);

#if defined(HELI)
  new StaticText(line, rect_t{}, STR_MENUHELISETUP, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_ADCFILTERVALUES, 0, 2,
             GET_SET_DEFAULT(g_model.jitterFilter));
  line = form->newLine(&grid);
#endif

#if defined(FLIGHT_MODES)
  new StaticText(line, rect_t{}, STR_MENUFLIGHTMODES, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_ADCFILTERVALUES, 0, 2,
             GET_SET_DEFAULT(g_model.jitterFilter));
  line = form->newLine(&grid);
#endif

  new StaticText(line, rect_t{}, STR_MENUCURVES, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_ADCFILTERVALUES, 0, 2,
             GET_SET_DEFAULT(g_model.jitterFilter));
  line = form->newLine(&grid);

#if defined(GVARS)
  new StaticText(line, rect_t{}, STR_MENU_GLOBAL_VARS, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_ADCFILTERVALUES, 0, 2,
             GET_SET_DEFAULT(g_model.jitterFilter));
  line = form->newLine(&grid);
#endif

  new StaticText(line, rect_t{}, STR_MENULOGICALSWITCHES, 0,
                 COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_ADCFILTERVALUES, 0, 2,
             GET_SET_DEFAULT(g_model.jitterFilter));
  line = form->newLine(&grid);

  new StaticText(line, rect_t{}, STR_MENUSPECIALFUNCS, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_ADCFILTERVALUES, 0, 2,
             GET_SET_DEFAULT(g_model.jitterFilter));
  line = form->newLine(&grid);

#if defined(LUA_MODEL_SCRIPTS)
  new StaticText(line, rect_t{}, STR_MENUCUSTOMSCRIPTS, 0,
                 COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_ADCFILTERVALUES, 0, 2,
             GET_SET_DEFAULT(g_model.jitterFilter));
  line = form->newLine(&grid);
#endif
}
