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

#include "opentx.h"
#include "widgets_container_impl.h"

#define SOURCE_OPT_IDX 0
#define VALUE_OPT_IDX 1
#define COLOR_OPT_IDX 2
#define SHADOW_OPT_IDX 3
#define LABEL_ALIGN_OPT_IDX 4
#define VALUE_ALIGN_OPT_IDX 5

const coord_t NUMBERS_PADDING = 4;

class ValueDivWidget: public Widget
{
  public:
   ValueDivWidget(const WidgetFactory* factory, Window* parent,
               const rect_t& rect, Widget::PersistentData* persistentData) :
       Widget(factory, parent, rect, persistentData)
   {
   }

    void refresh(BitmapBuffer * dc) override
    {
      // get source from options[0]
      mixsrc_t field = persistentData->options[SOURCE_OPT_IDX].value.unsignedValue;

      // get divisor from options[1]
      uint32_t div = persistentData->options[VALUE_OPT_IDX].value.unsignedValue;

      // get color from options[2]
      LcdFlags color = COLOR2FLAGS(persistentData->options[COLOR_OPT_IDX].value.unsignedValue);

      // get shadow from options[3]
      // *** NO ACTION TAKEN HERE, SEE BELOW ***

      // get label alignment from options[4]
      LcdFlags label_alignment = persistentData->options[LABEL_ALIGN_OPT_IDX].value.unsignedValue;

      // get value alignment from options[5]
      LcdFlags value_alignment = persistentData->options[VALUE_ALIGN_OPT_IDX].value.unsignedValue;

      coord_t xValue, yValue, xLabel, yLabel;
      LcdFlags attrValue, attrLabel = 0;

      if (width() < 120 && height() < 50) {
        switch (label_alignment) {
          case ALIGN_RIGHT:
            xLabel = width() - NUMBERS_PADDING;
            attrLabel = RIGHT;
            break;
          case ALIGN_CENTER:
            xLabel = width()/2;
            attrLabel = CENTERED;
            break;
          default: // ALIGN_LEFT:
            xLabel = 0;
            attrLabel = LEFT;
        }
        yLabel = 0;
        switch (value_alignment) {
          case ALIGN_RIGHT:
            xValue = width() - NUMBERS_PADDING;
            attrValue = RIGHT | NO_UNIT | FONT(L);
            break;
          case ALIGN_CENTER:
            xValue = width()/2 - NUMBERS_PADDING;
            attrValue = CENTERED | NO_UNIT | FONT(L);
            break;
          default: // ALIGN_LEFT:
            xValue = 0;
            attrValue = LEFT | NO_UNIT | FONT(L);
        }
        yValue = 14;
      }
      else if (height() < 50) {
        xValue = width() - NUMBERS_PADDING;
        yValue = -2;
        xLabel = NUMBERS_PADDING;
        yLabel = +2;
        attrValue = RIGHT | NO_UNIT | FONT(L);
      }
      else {
        switch (label_alignment) {
          case ALIGN_RIGHT:
            xLabel = width() - NUMBERS_PADDING;
            attrLabel = RIGHT;
            break;
          case ALIGN_CENTER:
            xLabel = width()/2 - NUMBERS_PADDING;
            attrLabel = CENTERED;
            break;
          default: // ALIGN_LEFT:
            xLabel = NUMBERS_PADDING;
            attrLabel = LEFT;
        }
        yLabel = 2;
        switch (value_alignment) {
          case ALIGN_RIGHT:
            xValue = width() - NUMBERS_PADDING;
            break;
          case ALIGN_CENTER:
            xValue = width()/2;
            break;
          default: // ALIGN_LEFT:
            xValue = NUMBERS_PADDING;
        }
        yValue = 18;
        LcdFlags valalign = LEFT;
        switch (value_alignment) {
          case ALIGN_RIGHT:
            valalign = RIGHT;
            break;
          case ALIGN_CENTER:
            valalign = CENTERED;
            break;
          default: // ALIGN_LEFT:
            valalign = LEFT;
        }
        if (field >= MIXSRC_FIRST_TELEM) {
          if (isGPSSensor(1 + (field - MIXSRC_FIRST_TELEM) / 3)) {
            attrValue = valalign | FONT(L) | PREC1;
          }
          else {
            attrValue = valalign | FONT(XL);
          }
        }
#if defined(INTERNAL_GPS)
        else if (field == MIXSRC_TX_GPS) {
          attrValue = valalign | FONT(L) | PREC1;
        }
#endif
        else {
          attrValue = valalign | FONT(XL);
        }
      }

      if (field >= MIXSRC_FIRST_TIMER && field <= MIXSRC_LAST_TIMER) {
        TimerState & timerState = timersStates[field - MIXSRC_FIRST_TIMER];
        if (timerState.val < 0) {
          color = COLOR_THEME_WARNING;
        }
        if (persistentData->options[SHADOW_OPT_IDX].value.boolValue) {
          drawSource(dc, xLabel + 1, yLabel + 1, field, attrLabel | COLOR2FLAGS(BLACK));
          drawTimer(dc, xValue + 1, yValue + 1, abs(timerState.val),
                    attrValue | FONT(STD) | COLOR2FLAGS(BLACK));
        }
        drawSource(dc, xLabel, yLabel, field, attrLabel | color);
        drawTimer(dc, xValue, yValue, abs(timerState.val),
                    attrValue | FONT(STD) | color);

      } else if (field == MIXSRC_TX_TIME) {
        int32_t tme = getValue(MIXSRC_TX_TIME);
        if (persistentData->options[SHADOW_OPT_IDX].value.boolValue) {
          drawSource(dc, xLabel + 1, yLabel + 1, field, COLOR2FLAGS(BLACK));
          drawTimer(dc, xValue + 1, yValue + 1, tme,
                    attrValue | FONT(STD) | COLOR2FLAGS(BLACK) | TIMEHOUR);
        }
        drawSource(dc, xLabel, yLabel, field, attrLabel | color);
        drawTimer(dc, xValue, yValue, tme,
                  attrValue | FONT(STD) | color | TIMEHOUR);
      } else {

        if (field >= MIXSRC_FIRST_TELEM) {
          TelemetryItem& telemetryItem =
              telemetryItems[(field - MIXSRC_FIRST_TELEM) / 3];
          if (!telemetryItem.isAvailable() || telemetryItem.isOld()) {
            color = COLOR_THEME_DISABLED;
          }
        }

        if (0 != div) {
          uint32_t divValue = ((int)(getValue(field)) + ((int)div/2)) / (int)div;

          if (persistentData->options[SHADOW_OPT_IDX].value.boolValue) {
            drawSource(dc, xLabel + 1, yLabel + 1, field, attrLabel | COLOR2FLAGS(BLACK));
            // drawSourceValue(dc, xValue + 1, yValue + 1, field, attrValue | COLOR2FLAGS(BLACK));
            drawSourceCustomValue(dc, xValue + 1, yValue + 1, field, divValue, attrValue | COLOR2FLAGS(BLACK));
          }

          drawSource(dc, xLabel, yLabel, field, attrLabel | color);
          // drawSourceValue(dc, xValue, yValue, field, attrValue | color);
          drawSourceCustomValue(dc, xValue, yValue, field, divValue, attrValue | color);
        } else {
          if (persistentData->options[SHADOW_OPT_IDX].value.boolValue) {
            drawSource(dc, xLabel + 1, yLabel + 1, field, attrLabel | COLOR2FLAGS(BLACK));
            drawSourceValue(dc, xValue + 1, yValue + 1, field, attrValue | COLOR2FLAGS(BLACK));
          }

          drawSource(dc, xLabel, yLabel, field, attrLabel | color);
          drawSourceValue(dc, xValue, yValue, field, attrValue | color);
        }
      }
    }

    void checkEvents() override
    {
      Widget::checkEvents();

      mixsrc_t field = persistentData->options[SOURCE_OPT_IDX].value.unsignedValue;

      // if value changed
      auto newValue = getValue(field);
      if (lastValue != newValue) {
        lastValue = newValue;
        invalidate();
      }

      // if telemetry value, and telemetry offline or old data
      if (field >= MIXSRC_FIRST_TELEM) {
        TelemetryItem& telemetryItem =
            telemetryItems[(field - MIXSRC_FIRST_TELEM) / 3];
        if (!telemetryItem.isAvailable() || telemetryItem.isOld()) invalidate();
      }
    }

    static const ZoneOption options[];
    int32_t lastValue = 0;
};

const ZoneOption ValueDivWidget::options[] = {
  { STR_SOURCE, ZoneOption::Source, OPTION_VALUE_UNSIGNED(MIXSRC_FIRST_STICK) },
  { STR_VALUE, ZoneOption::Integer, OPTION_VALUE_UNSIGNED(1), OPTION_VALUE_UNSIGNED(1), OPTION_VALUE_UNSIGNED(12) },
  { STR_COLOR, ZoneOption::Color, OPTION_VALUE_UNSIGNED(COLOR_THEME_PRIMARY2 >> 16) },
  { STR_SHADOW, ZoneOption::Bool, OPTION_VALUE_BOOL(false)  },
  { STR_ALIGN_LABEL, ZoneOption::Align, OPTION_VALUE_UNSIGNED(ALIGN_LEFT) },
  { STR_ALIGN_VALUE, ZoneOption::Align, OPTION_VALUE_UNSIGNED(ALIGN_LEFT) },
  { nullptr, ZoneOption::Bool }
};

BaseWidgetFactory<ValueDivWidget> ValueDivWidget("ValueDiv", ValueDivWidget::options);
