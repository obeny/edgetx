/*
 * Authors (alphabetical order)
 * - Andre Bernet <bernet.andre@gmail.com>
 * - Andreas Weitl
 * - Bertrand Songis <bsongis@gmail.com>
 * - Bryan J. Rentoul (Gruvin) <gruvin@gmail.com>
 * - Cameron Weeks <th9xer@gmail.com>
 * - Erez Raviv
 * - Gabriel Birkus
 * - Jean-Pierre Parisy
 * - Karl Szmutny
 * - Michael Blandford
 * - Michal Hlavinka
 * - Pat Mackenzie
 * - Philip Moss
 * - Rob Thomson
 * - Romolo Manfredini <romolo.manfredini@gmail.com>
 * - Thomas Husterer
 *
 * opentx is based on code named
 * gruvin9x by Bryan J. Rentoul: http://code.google.com/p/gruvin9x/,
 * er9x by Erez Raviv: http://code.google.com/p/er9x/,
 * and the original (and ongoing) project by
 * Thomas Husterer, th9x: http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "../../opentx.h"
#include <stdio.h>

#define COLUMN_HEADER_X 150

void displayHeader(const char *header)
{
  // TODO ? lcd_putsAtt(COLUMN_HEADER_X, MENU_FOOTER_TOP, header, HEADER_COLOR);
}

void displayColumnHeader(const char * const *headers, uint8_t index)
{
  // TODO ? displayHeader(headers[index]);
}

const char * STR_MONTHS[] = { "Jan", "Fev", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

#define DATETIME_SEPARATOR_X    425
#define DATETIME_LINE1          9
#define DATETIME_LINE2          23
#define DATETIME_LEFT(s)        (LCD_W+DATETIME_SEPARATOR_X+8-getTextWidth(s, SMLSIZE))/2

void lcdDrawTopmenuDatetime()
{
  lcdDrawVerticalLine(DATETIME_SEPARATOR_X, 7, 31, TEXT_INVERTED_COLOR);

  struct gtm t;
  gettime(&t);
  char str[10];
  sprintf(str, "%d %s", t.tm_mday, STR_MONTHS[t.tm_mon]);
  lcd_putsAtt(DATETIME_LEFT(str), DATETIME_LINE1, str, SMLSIZE|TEXT_INVERTED_COLOR);

  getTimerString(str, getValue(MIXSRC_TX_TIME));
  lcd_putsAtt(DATETIME_LEFT(str), DATETIME_LINE2, str, SMLSIZE|TEXT_INVERTED_COLOR);
}

void drawStick(coord_t centrex, int16_t xval, int16_t yval)
{
#define BOX_CENTERY   (220 - FH - BOX_WIDTH/2)
#define MARKER_WIDTH  5

  lcdDrawSquare(centrex-BOX_WIDTH/2, BOX_CENTERY-BOX_WIDTH/2, BOX_WIDTH, TEXT_COLOR);
  lcd_vlineStip(centrex, BOX_CENTERY-1, 3, SOLID, TEXT_COLOR);
  lcd_hlineStip(centrex-1, BOX_CENTERY, 3, SOLID, TEXT_COLOR);
  lcdDrawSquare(centrex + (xval/((2*RESX)/(BOX_WIDTH-MARKER_WIDTH))) - MARKER_WIDTH/2, BOX_CENTERY - (yval/((2*RESX)/(BOX_WIDTH-MARKER_WIDTH))) - MARKER_WIDTH/2, MARKER_WIDTH, ROUND|TEXT_COLOR);

#undef BOX_CENTERY
#undef MARKER_WIDTH
}

void lcdDrawCheckBox(coord_t x, coord_t y, uint8_t value, LcdFlags attr)
{
  if (attr) {
    lcdDrawFilledRect(x-1, y+2, 13, 13, TEXT_INVERTED_BGCOLOR);
    lcdDrawFilledRect(x+1, y+4, 9, 9, TEXT_BGCOLOR);
    if (value) {
      lcdDrawFilledRect(x+2, y+5, 7, 7, TEXT_INVERTED_BGCOLOR);
    }
  }
  else {
    if (value) {
      lcdDrawFilledRect(x+2, y+5, 7, 7, SCROLLBOX_COLOR);
      lcdDrawRect(x, y+3, 11, 11, LINE_COLOR);
    }
    else {
      lcdDrawRect(x, y+3, 11, 11, LINE_COLOR);
    }
  }
}

void lcdDrawScrollbar(coord_t x, coord_t y, coord_t h, uint16_t offset, uint16_t count, uint8_t visible)
{
  lcdDrawVerticalLine(x, y, h, LINE_COLOR);
  coord_t yofs = (h*offset + count/2) / count;
  coord_t yhgt = (h*visible + count/2) / count;
  if (yhgt + yofs > h)
    yhgt = h - yofs;
  lcdDrawFilledRect(x-1, y + yofs, 3, yhgt, SCROLLBOX_COLOR);
}

void displayProgressBar(const char *label)
{
  lcd_putsLeft(4*FH, label);
  lcd_rect(3, 6*FH+4, 204, 7);
  lcdRefresh();
}

void updateProgressBar(int num, int den)
{
  if (num > 0 && den > 0) {
    int width = (200*num)/den;
    lcd_hline(5, 6*FH+6, width);
    lcd_hline(5, 6*FH+7, width);
    lcd_hline(5, 6*FH+8, width);
    lcdRefresh();
  }
}

#define MENU_ICONS_SPACING 31

void drawMenuTemplate(const char * name, evt_t event)
{
  // clear the screen
  lcdDrawFilledRect(0, 0, LCD_W, MENU_HEADER_HEIGHT, HEADER_BGCOLOR);
  lcdDrawFilledRect(0, MENU_HEADER_HEIGHT, LCD_W, MENU_TITLE_TOP-MENU_HEADER_HEIGHT, TEXT_BGCOLOR);
  lcdDrawFilledRect(0, MENU_TITLE_TOP, LCD_W, MENU_TITLE_HEIGHT, TITLE_BGCOLOR);
  lcdDrawFilledRect(0, MENU_BODY_TOP, LCD_W, MENU_BODY_HEIGHT, TEXT_BGCOLOR);
  lcdDrawFilledRect(0, MENU_FOOTER_TOP, LCD_W, MENU_FOOTER_HEIGHT, HEADER_BGCOLOR);

  lcdDrawBitmapPattern(0, 0, LBM_TOPMENU_POLYGON, TITLE_BGCOLOR);

  if (m_posVert < 0) {
    lcdDrawBitmapPattern(58+menuPageIndex*MENU_ICONS_SPACING-10, 0, LBM_CURRENT_BG, TITLE_BGCOLOR);
  }
  else {
    lcdDrawFilledRect(58+menuPageIndex*MENU_ICONS_SPACING-9, 0, 32, MENU_HEADER_HEIGHT, TITLE_BGCOLOR);
    lcdDrawBitmapPattern(58+menuPageIndex*MENU_ICONS_SPACING, MENU_TITLE_TOP-9, LBM_DOT, MENU_TITLE_COLOR);
  }

  const uint8_t * const * icons = (g_menuPos[0] == 0 ? LBM_MODEL_ICONS : LBM_RADIO_ICONS);

  lcdDrawBitmapPattern(5, 7, icons[0], MENU_TITLE_COLOR);

  for (int i=0; i<menuPageCount; i++) {
    lcdDrawBitmapPattern(50+i*MENU_ICONS_SPACING, 7, icons[i+1], MENU_TITLE_COLOR);
  }

  if (m_posVert < 0) {
    lcdDrawBitmapPattern(58+menuPageIndex*MENU_ICONS_SPACING-10, 0, LBM_CURRENT_SHADOW, TEXT_COLOR);
    lcdDrawBitmapPattern(58+menuPageIndex*MENU_ICONS_SPACING, MENU_TITLE_TOP-9, LBM_CURRENT_DOT, MENU_TITLE_COLOR);
  }

  lcdDrawTopmenuDatetime();

  if (name) {
    // must be done at the end so that we can write something at the right of the menu title
    lcd_putsAtt(MENUS_MARGIN_LEFT, MENU_TITLE_TOP+2, name, MENU_TITLE_COLOR);
  }
}

select_menu_value_t selectMenuItem(coord_t x, coord_t y, const pm_char *label, const pm_char *values, select_menu_value_t value, select_menu_value_t min, select_menu_value_t max, LcdFlags attr, evt_t event)
{
  lcd_putsColumnLeft(x, y, label);
  if (values) lcd_putsiAtt(x, y, values, value-min, attr);
  if (attr) value = checkIncDec(event, value, min, max, (g_menuPos[0] == 0) ? EE_MODEL : EE_GENERAL);
  return value;
}

uint8_t onoffMenuItem(uint8_t value, coord_t x, coord_t y, const pm_char *label, LcdFlags attr, evt_t event )
{
  lcdDrawCheckBox(x, y, value, attr);
  return selectMenuItem(x, y, label, NULL, value, 0, 1, attr, event);
}

int8_t switchMenuItem(coord_t x, coord_t y, int8_t value, LcdFlags attr, evt_t event)
{
  lcd_putsColumnLeft(x, y, STR_SWITCH);
  putsSwitches(x,  y, value, attr);
  if (attr) CHECK_INCDEC_MODELSWITCH(event, value, SWSRC_FIRST_IN_MIXES, SWSRC_LAST_IN_MIXES, isSwitchAvailableInMixes);
  return value;
}

void displaySlider(coord_t x, coord_t y, uint8_t value, uint8_t max, uint8_t attr)
{
  const int width = 50;
  lcd_hlineStip(x, y+5, width, SOLID, TEXT_COLOR);
  if (attr && (!(attr & BLINK) || !BLINK_ON_PHASE)) {
    lcdDrawFilledRect(x+value*(width-5)/max, y, 5, 11, TEXT_INVERTED_BGCOLOR);
  }
  else {
    lcdDrawFilledRect(x+value*(width-5)/max, y, 5, 11, LINE_COLOR);
  }
}

#if defined(GVARS)
bool noZero(int val)
{
  return val != 0;
}

int16_t gvarMenuItem(coord_t x, coord_t y, int16_t value, int16_t min, int16_t max, LcdFlags attr, uint8_t editflags, evt_t event)
{
  uint16_t delta = GV_GET_GV1_VALUE(max);
  bool invers = (attr & INVERS);

  // TRACE("gvarMenuItem(val=%d min=%d max=%d)", value, min, max);

  if (invers && event == EVT_KEY_LONG(KEY_ENTER)) {
    s_editMode = !s_editMode;
    if (attr & PREC1)
      value = (GV_IS_GV_VALUE(value, min, max) ? GET_GVAR(value, min, max, mixerCurrentFlightMode)*10 : delta);
    else
      value = (GV_IS_GV_VALUE(value, min, max) ? GET_GVAR(value, min, max, mixerCurrentFlightMode) : delta);
    eeDirty(EE_MODEL);
  }

  if (GV_IS_GV_VALUE(value, min, max)) {
    if (attr & LEFT)
      attr -= LEFT; /* because of ZCHAR */
    else
      x -= 20;

    attr &= ~PREC1;

    int8_t idx = (int16_t) GV_INDEX_CALC_DELTA(value, delta);
    if (idx >= 0) ++idx;    // transform form idx=0=GV1 to idx=1=GV1 in order to handle double keys invert
    if (invers) {
      CHECK_INCDEC_MODELVAR_CHECK(event, idx, -MAX_GVARS, MAX_GVARS, noZero);
      if (idx == 0) idx = 1;    // handle reset to zero, map to GV1
    }
    if (idx < 0) {
      value = (int16_t) GV_CALC_VALUE_IDX_NEG(idx, delta);
      idx = -idx;
      putsStrIdx(x, y, STR_GV, idx, attr, "-");
    }
    else {
      putsStrIdx(x, y, STR_GV, idx, attr);
      value = (int16_t) GV_CALC_VALUE_IDX_POS(idx-1, delta);
    }
  }
  else {
    lcd_outdezAtt(x, y, value, attr, "%");
    if (invers) value = checkIncDec(event, value, min, max, EE_MODEL | editflags);
  }
  return value;
}
#else
int16_t gvarMenuItem(coord_t x, coord_t y, int16_t value, int16_t min, int16_t max, LcdFlags attr, evt_t event)
{
  lcd_outdezAtt(x, y, value, attr, "%");
  if (attr&INVERS) value = checkIncDec(event, value, min, max, EE_MODEL);
  return value;
}
#endif
