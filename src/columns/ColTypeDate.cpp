////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <ColTypeDate.h>
#include <Context.h>
#include <ISO8601.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnTypeDate::ColumnTypeDate ()
{
  _name      = "";
  _type      = "date";
  _style     = "formatted";
  _label     = "";
  _styles    = {"formatted",
                "julian",
                "epoch", 
                "iso",
                "age",
                "relative",
                "remaining",
                "countdown"};

  ISO8601d now;
  now -= 125; // So that "age" is non-zero.
  _examples = {now.toString (context.config.get ("dateformat")),
               format (now.toJulian (), 13, 12),
               now.toEpochString (),
               now.toISO (),
               ISO8601p (ISO8601d () - now).formatVague (),
               "-" + ISO8601p (ISO8601d () - now).formatVague (),
               "",
               ISO8601p (ISO8601d () - now).format ()};
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnTypeDate::measure (Task& task, unsigned int& minimum, unsigned int& maximum)
{
  minimum = maximum = 0;

  if (task.has (_name))
  {
    ISO8601d date (task.get_date (_name));

    if (_style == "default" ||
        _style == "formatted")
    {
      // Determine the output date format, which uses a hierarchy of definitions.
      //   rc.report.<report>.dateformat
      //   rc.dateformat.report
      //   rc.dateformat.
      std::string format = context.config.get ("report." + _report + ".dateformat");
      if (format == "")
        format = context.config.get ("dateformat.report");
      if (format == "")
        format = context.config.get ("dateformat");

      minimum = maximum = ISO8601d::length (format);
    }
    else if (_style == "countdown")
    {
      ISO8601d now;
      if (now > date)
        minimum = maximum = ISO8601p (now - date).formatVague ().length ();
    }
    else if (_style == "julian")
    {
      minimum = maximum = format (date.toJulian (), 13, 12).length ();
    }
    else if (_style == "epoch")
    {
      minimum = maximum = date.toEpochString ().length ();
    }
    else if (_style == "iso")
    {
      minimum = maximum = date.toISO ().length ();
    }
    else if (_style == "age")
    {
      ISO8601d now;
      if (now > date)
        minimum = maximum = ISO8601p (now - date).formatVague ().length ();
      else
        minimum = maximum = ISO8601p (date - now).formatVague ().length () + 1;
    }
    else if (_style == "relative")
    {
      ISO8601d now;
      if (now < date)
        minimum = maximum = ISO8601p (date - now).formatVague ().length ();
      else
        minimum = maximum = ISO8601p (now - date).formatVague ().length () + 1;
    }
    else if (_style == "remaining")
    {
      ISO8601d now;
      if (date > now)
        minimum = maximum = ISO8601p (date - now).formatVague ().length ();
    }
    else
      throw format (STRING_COLUMN_BAD_FORMAT, _name, _style);
  }
}

////////////////////////////////////////////////////////////////////////////////
void ColumnTypeDate::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  if (task.has (_name))
  {
    ISO8601d date (task.get_date (_name));

    if (_style == "default" ||
        _style == "formatted")
    {
      // Determine the output date format, which uses a hierarchy of definitions.
      //   rc.report.<report>.dateformat
      //   rc.dateformat.report
      //   rc.dateformat
      std::string format = context.config.get ("report." + _report + ".dateformat");
      if (format == "")
      {
        format = context.config.get ("dateformat.report");
        if (format == "")
          format = context.config.get ("dateformat");
      }

      renderStringLeft (lines, width, color, date.toString (format));
    }
    else if (_style == "countdown")
    {
      ISO8601d now;
      if (now > date)
        renderStringRight (lines, width, color, ISO8601p (now - date).formatVague ());
    }
    else if (_style == "julian")
      renderStringRight (lines, width, color, format (date.toJulian (), 13, 12));

    else if (_style == "epoch")
      renderStringRight (lines, width, color, date.toEpochString ());

    else if (_style == "iso")
      renderStringLeft (lines, width, color, date.toISO ());

    else if (_style == "age")
    {
      ISO8601d now;
      if (now > date)
        renderStringLeft (lines, width, color, ISO8601p (now - date).formatVague ());
      else
        renderStringLeft (lines, width, color, "-" + ISO8601p (date - now).formatVague ());
    }
    else if (_style == "relative")
    {
      ISO8601d now;
      if (now < date)
        renderStringLeft (lines, width, color, ISO8601p (date - now).formatVague ());
      else
        renderStringLeft (lines, width, color, "-" + ISO8601p (now - date).formatVague ());
    }

    else if (_style == "remaining")
    {
      ISO8601d now;
      if (date > now)
        renderStringRight (lines, width, color, ISO8601p (date - now).formatVague ());
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
