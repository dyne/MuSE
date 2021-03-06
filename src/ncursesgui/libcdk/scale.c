#include "cdk.h"

/*
 * Declare file local prototypes.
 */
static void drawCDKScaleField (CDKSCALE *scale);

DeclareCDKObjects(my_funcs,Scale);

/*
 * This function creates a scale widget.
 */
CDKSCALE *newCDKScale (CDKSCREEN *cdkscreen, int xplace, int yplace, char *title, char *label, chtype fieldAttr, int fieldWidth, int start, int low, int high, int inc, int fastinc, boolean Box, boolean shadow)
{
   /* Declare local variables. */
   CDKSCALE *scale	= newCDKObject(CDKSCALE, &my_funcs);
   chtype *holder	= 0;
   int parentWidth	= getmaxx(cdkscreen->window) - 1;
   int parentHeight	= getmaxy(cdkscreen->window) - 1;
   int boxHeight	= 3;
   int boxWidth		= fieldWidth + 2;
   int maxWidth		= INT_MIN;
   int horizontalAdjust = 0;
   int xpos		= xplace;
   int ypos		= yplace;
   char **temp		= 0;
   int x, len, junk, junk2;

   /* Set some basic values of the scale field. */
   scale->label		= 0;
   scale->labelLen	= 0;
   scale->labelWin	= 0;
   scale->titleLines	= 0;

  /*
   * If the fieldWidth is a negative value, the fieldWidth will
   * be COLS-fieldWidth, otherwise, the fieldWidth will be the
   * given width.
   */
   fieldWidth = setWidgetDimension (parentWidth, fieldWidth, 0);
   boxWidth = fieldWidth + 2;

   /* Translate the label char *pointer to a chtype pointer. */
   if (label != 0)
   {
      scale->label	= char2Chtype (label, &scale->labelLen, &junk);
      boxWidth		= scale->labelLen + fieldWidth + 2;
   }

   /* Translate the char * items to chtype * */
   if (title != 0)
   {
      temp = CDKsplitString (title, '\n');
      scale->titleLines = CDKcountStrings (temp);

      /* We need to determine the widest title line. */
      for (x=0; x < scale->titleLines; x++)
      {
	 holder = char2Chtype (temp[x], &len, &junk2);
	 maxWidth = MAXIMUM (maxWidth, len);
	 freeChtype (holder);
      }

      /*
       * If one of the title lines is wider than the field and the label,
       * the box width will expand to accomodate.
       */
       if (maxWidth > boxWidth)
       {
	  horizontalAdjust = (int)((maxWidth - boxWidth) / 2) + 1;
	  boxWidth = maxWidth + 2;
       }

      /* For each line in the title, convert from char * to chtype * */
      for (x=0; x < scale->titleLines; x++)
      {
	 scale->title[x]	= char2Chtype (temp[x], &scale->titleLen[x], &scale->titlePos[x]);
	 scale->titlePos[x]	= justifyString (boxWidth, scale->titleLen[x], scale->titlePos[x]);
      }
      CDKfreeStrings(temp);
   }
   else
   {
      /* No title? Set the required variables. */
      scale->titleLines = 0;
   }
   boxHeight += scale->titleLines;

  /*
   * Make sure we didn't extend beyond the dimensions of the window.
   */
   boxWidth = (boxWidth > parentWidth ? parentWidth : boxWidth);
   boxHeight = (boxHeight > parentHeight ? parentHeight : boxHeight);
   fieldWidth = (fieldWidth > (boxWidth - scale->labelLen - 2) ? (boxWidth - scale->labelLen - 2) : fieldWidth);

   /* Rejustify the x and y positions if we need to. */
   alignxy (cdkscreen->window, &xpos, &ypos, boxWidth, boxHeight);

   /* Make the scale window. */
   scale->win = newwin (boxHeight, boxWidth, ypos, xpos);

   /* Is the main window null??? */
   if (scale->win == 0)
   {
      freeChtype (scale->label);
      free (scale);

      /* Return a null pointer. */
      return (0);
   }

   /* Create the scale label window. */
   if (scale->label != 0)
   {
      scale->labelWin = subwin (scale->win, 1,
				scale->labelLen + 2,
				ypos + scale->titleLines + 1,
				xpos + horizontalAdjust + 1);
   }

   /* Create the scale field window. */
   scale->fieldWin = subwin (scale->win, 1, fieldWidth,
				ypos + scale->titleLines + 1,
				xpos + scale->labelLen + horizontalAdjust + 1);
   keypad (scale->fieldWin, TRUE);
   keypad (scale->win, TRUE);

   /* Create the scale field. */
   ScreenOf(scale)		= cdkscreen;
   ObjOf(scale)->box		= Box;
   scale->parent		= cdkscreen->window;
   scale->shadowWin		= 0;
   scale->boxWidth		= boxWidth;
   scale->boxHeight		= boxHeight;
   scale->fieldWidth		= fieldWidth;
   scale->fieldAttr		= (chtype)fieldAttr;
   scale->current		= low;
   scale->low			= low;
   scale->high			= high;
   scale->current		= start;
   scale->inc			= inc;
   scale->fastinc		= fastinc;
   scale->exitType		= vNEVER_ACTIVATED;
   scale->shadow		= shadow;
   scale->preProcessFunction	= 0;
   scale->preProcessData	= 0;
   scale->postProcessFunction	= 0;
   scale->postProcessData	= 0;
   scale->ULChar		= ACS_ULCORNER;
   scale->URChar		= ACS_URCORNER;
   scale->LLChar		= ACS_LLCORNER;
   scale->LRChar		= ACS_LRCORNER;
   scale->HChar			= ACS_HLINE;
   scale->VChar			= ACS_VLINE;
   scale->BoxAttrib		= A_NORMAL;

   /* Do we want a shadow??? */
   if (shadow)
   {
      scale->shadowWin	= newwin (boxHeight, boxWidth, ypos + 1, xpos + 1);
   }

   /* Clean the key bindings. */
   cleanCDKObjectBindings (vSCALE, scale);

   /* Register this baby. */
   registerCDKObject (cdkscreen, vSCALE, scale);

   /* Return the pointer. */
   return (scale);
}

/*
 * This allows the person to use the scale field.
 */
int activateCDKScale (CDKSCALE *scale, chtype *actions)
{
   /* Declare local variables. */
   int ret;

   /* Draw the scale widget. */
   drawCDKScale (scale, ObjOf(scale)->box);

   /* Check if actions is null. */
   if (actions == 0)
   {
      chtype input = 0;
      for (;;)
      {
	 /* Get the input. */
	 input = wgetch (scale->fieldWin);

	 /* Inject the character into the widget. */
	 ret = injectCDKScale (scale, input);
	 if (scale->exitType != vEARLY_EXIT)
	 {
	    return ret;
	 }
      }
   }
   else
   {
      int length = chlen (actions);
      int x = 0;

      /* Inject each character one at a time. */
      for (x=0; x < length; x++)
      {
	 ret = injectCDKScale (scale, actions[x]);
	 if (scale->exitType != vEARLY_EXIT)
	 {
	    return ret;
	 }
      }
   }

   /* Set the exit type and return. */
   scale->exitType = vEARLY_EXIT;
   return -1;
}

/*
 * This function injects a single character into the widget.
 */
int injectCDKScale (CDKSCALE *scale, chtype input)
{
   /* Declare some local variables. */
   int ppReturn = 1;

   /* Set the exit type. */
   scale->exitType = vEARLY_EXIT;

   /* Draw the field. */
   drawCDKScaleField (scale);

   /* Check if there is a pre-process function to be called. */
   if (scale->preProcessFunction != 0)
   {
      /* Call the pre-process function. */
      ppReturn = scale->preProcessFunction (vSCALE, scale, scale->preProcessData, input);
   }

   /* Should we continue? */
   if (ppReturn != 0)
   {
      /* Check for a key binding. */
      if (checkCDKObjectBind(vSCALE, scale, input) != 0)
      {
	 scale->exitType = vESCAPE_HIT;
	 return 0;
      }
      else
      {
	 switch (input)
	 {
	    case KEY_LEFT : case 'd' : case '-' : case KEY_DOWN :
		 if (scale->current > scale->low)
		 {
		    scale->current -= scale->inc;
		 }
		 else
		 {
		    Beep();
		 }
		 break;

	    case KEY_RIGHT : case 'u' : case '+' : case KEY_UP :
		 if (scale->current < scale->high)
		 {
		    scale->current += scale->inc;
		 }
		 else
		 {
		    Beep();
		 }
		 break;

	    case KEY_PPAGE : case 'U' : case CONTROL('B') :
		 if ((scale->current + scale->fastinc) <= scale->high)
		 {
		    scale->current += scale->fastinc;
		 }
		 else
		 {
		    Beep();
		 }
		 break;

	       case KEY_NPAGE : case 'D' : case CONTROL('F') :
		 if ((scale->current - scale->fastinc) >= scale->low)
		 {
		    scale->current -= scale->fastinc;
		 }
		 else
		 {
		    Beep();
		 }
		 break;

	    case KEY_HOME : case 'g' : case '0' :
		 scale->current = scale->low;
		 break;

	    case KEY_END : case 'G' : case '$' :
		 scale->current = scale->high;
		 break;

	    case KEY_RETURN : case TAB : case KEY_ENTER :
		 scale->exitType = vNORMAL;
		 return (scale->current);

	    case KEY_ESC :
		 scale->exitType = vESCAPE_HIT;
		 return (scale->current);

	    case CDK_REFRESH :
		eraseCDKScreen (ScreenOf(scale));
		refreshCDKScreen (ScreenOf(scale));
		break;

	    default :
		 break;
	 }
      }

      /* Should we call a post-process? */
      if (scale->postProcessFunction != 0)
      {
	 scale->postProcessFunction (vSCALE, scale, scale->postProcessData, input);
      }
   }

   /* Draw the field window. */
   drawCDKScaleField (scale);

   /* Set the exit type and return. */
   scale->exitType = vEARLY_EXIT;
   return 0;
}

/*
 * This moves the scale field to the given location.
 */
static void _moveCDKScale (CDKOBJS *object, int xplace, int yplace, boolean relative, boolean refresh_flag)
{
   CDKSCALE *scale = (CDKSCALE *)object;
   /* Declare local variables. */
   int currentX = getbegx(scale->win);
   int currentY = getbegy(scale->win);
   int xpos	= xplace;
   int ypos	= yplace;
   int xdiff	= 0;
   int ydiff	= 0;

   /*
    * If this is a relative move, then we will adjust where we want
    * to move to.
    */
   if (relative)
   {
      xpos = getbegx(scale->win) + xplace;
      ypos = getbegy(scale->win) + yplace;
   }

   /* Adjust the window if we need to. */
   alignxy (WindowOf(scale), &xpos, &ypos, scale->boxWidth, scale->boxHeight);

   /* Get the difference. */
   xdiff = currentX - xpos;
   ydiff = currentY - ypos;

   /* Move the window to the new location. */
   moveCursesWindow(scale->win, -xdiff, -ydiff);
   if (scale->labelWin != 0)
   {
      moveCursesWindow(scale->labelWin, -xdiff, -ydiff);
   }
   moveCursesWindow(scale->fieldWin, -xdiff, -ydiff);

   /* If there is a shadow box we have to move it too. */
   if (scale->shadowWin != 0)
   {
      moveCursesWindow(scale->shadowWin, -xdiff, -ydiff);
   }

   /* Touch the windows so they 'move'. */
   touchwin (WindowOf(scale));
   wrefresh (WindowOf(scale));

   /* Redraw the window, if they asked for it. */
   if (refresh_flag)
   {
      drawCDKScale (scale, ObjOf(scale)->box);
   }
}

/*
 * This function draws the scale widget.
 */
static void _drawCDKScale (CDKOBJS *object, boolean Box)
{
   CDKSCALE *scale = (CDKSCALE *)object;
   int x;

   /* Draw the shadow. */
   if (scale->shadowWin != 0)
   {
      drawShadow (scale->shadowWin);
   }

   /* Box the widget if asked. */
   if (Box)
   {
      attrbox (scale->win,
		scale->ULChar, scale->URChar,
		scale->LLChar, scale->LRChar,
		scale->HChar,  scale->VChar,
		scale->BoxAttrib);
   }

   /* Draw in the title if there is one. */
   if (scale->titleLines != 0)
   {
      for (x=0; x < scale->titleLines; x++)
      {
	 writeChtype (scale->win,
			scale->titlePos[x],
			x + 1,
			scale->title[x],
			HORIZONTAL, 0,
			scale->titleLen[x]);
      }
   }

   /* Draw the label. */
   if (scale->labelWin != 0)
   {
      writeChtype (scale->labelWin, 0, 0,
			scale->label,
			HORIZONTAL, 0,
			scale->labelLen);
   }
   touchwin (scale->win);
   wrefresh (scale->win);

   /* Draw the field window. */
   drawCDKScaleField (scale);
}

/*
 * This draws the scale widget.
 */
static void drawCDKScaleField (CDKSCALE *scale)
{
   /* Declare the local variables. */
   char temp[256];

   /* Erase the field. */
   werase (scale->fieldWin);

   /* Draw the value in the field. */
   sprintf (temp, "%d", scale->current);
   writeCharAttrib (scale->fieldWin,
			scale->fieldWidth-(int)strlen(temp)-1, 0, temp,
			scale->fieldAttr,
			HORIZONTAL, 0,
			(int)strlen(temp));

   /* Refresh the field window. */
   touchwin (scale->fieldWin);
   wrefresh (scale->fieldWin);
}

/*
 * These functions set the drawing characters of the widget.
 */
void setCDKScaleULChar (CDKSCALE *scale, chtype character)
{
   scale->ULChar = character;
}
void setCDKScaleURChar (CDKSCALE *scale, chtype character)
{
   scale->URChar = character;
}
void setCDKScaleLLChar (CDKSCALE *scale, chtype character)
{
   scale->LLChar = character;
}
void setCDKScaleLRChar (CDKSCALE *scale, chtype character)
{
   scale->LRChar = character;
}
void setCDKScaleVerticalChar (CDKSCALE *scale, chtype character)
{
   scale->VChar = character;
}
void setCDKScaleHorizontalChar (CDKSCALE *scale, chtype character)
{
   scale->HChar = character;
}
void setCDKScaleBoxAttribute (CDKSCALE *scale, chtype character)
{
   scale->BoxAttrib = character;
}

/*
 * This sets the background color of the widget.
 */
void setCDKScaleBackgroundColor (CDKSCALE *scale, char *color)
{
   chtype *holder = 0;
   int junk1, junk2;

   /* Make sure the color isn't null. */
   if (color == 0)
   {
      return;
   }

   /* Convert the value of the environment variable to a chtype. */
   holder = char2Chtype (color, &junk1, &junk2);

   /* Set the widgets background color. */
   wbkgd (scale->win, holder[0]);
   wbkgd (scale->fieldWin, holder[0]);
   if (scale->labelWin != 0)
   {
      wbkgd (scale->labelWin, holder[0]);
   }

   /* Clean up. */
   freeChtype (holder);
}

/*
 * This function destroys the scale widget.
 */
void destroyCDKScale (CDKSCALE *scale)
{
   int x;

   /* Erase the object. */
   eraseCDKScale (scale);

   /* Clean up the char pointers. */
   freeChtype (scale->label);
   for (x=0; x < scale->titleLines; x++)
   {
      freeChtype (scale->title[x]);
   }

   /* Clean up the windows. */
   deleteCursesWindow (scale->fieldWin);
   deleteCursesWindow (scale->labelWin);
   deleteCursesWindow (scale->shadowWin);
   deleteCursesWindow (scale->win);

   /* Unregister this object. */
   unregisterCDKObject (vSCALE, scale);

   /* Finish cleaning up. */
   free (scale);
}

/*
 * This function erases the scale widget from the screen.
 */
static void _eraseCDKScale (CDKOBJS *object)
{
   CDKSCALE *scale = (CDKSCALE *)object;

   eraseCursesWindow (scale->labelWin);
   eraseCursesWindow (scale->fieldWin);
   eraseCursesWindow (scale->win);
   eraseCursesWindow (scale->shadowWin);
}

/*
 * These functions set specific attributes of the widget.
 */
void setCDKScale (CDKSCALE *scale, int low, int high, int value, boolean Box)
{
   setCDKScaleLowHigh (scale, low, high);
   setCDKScaleValue (scale, value);
   setCDKScaleBox (scale, Box);
}

/*
 * This sets the low and high values of the scale widget.
 */
void setCDKScaleLowHigh (CDKSCALE *scale, int low, int high)
{
   /* Make sure the values aren't out of bounds. */
   if (low <= high)
   {
      scale->low	= low;
      scale->high	= high;
   }
   else if (low > high)
   {
      scale->low	= high;
      scale->high	= low;
   }
}
int getCDKScaleLowValue (CDKSCALE *scale)
{
   return scale->low;
}
int getCDKScaleHighValue (CDKSCALE *scale)
{
   return scale->high;
}

/*
 * This sets the scale value.
 */
void setCDKScaleValue (CDKSCALE *scale, int value)
{
   if ((value >= scale->low) && (value <= scale->high))
   {
      scale->current = value;
   }
}
int getCDKScaleValue (CDKSCALE *scale)
{
   return scale->current;
}

/*
 * This sets the scale box attribute.
 */
void setCDKScaleBox (CDKSCALE *scale, boolean Box)
{
   ObjOf(scale)->box = Box;
}
boolean getCDKScaleBox (CDKSCALE *scale)
{
   return ObjOf(scale)->box;
}

/*
 * This function sets the pre-process function.
 */
void setCDKScalePreProcess (CDKSCALE *scale, PROCESSFN callback, void *data)
{
   scale->preProcessFunction = callback;
   scale->preProcessData = data;
}

/*
 * This function sets the post-process function.
 */
void setCDKScalePostProcess (CDKSCALE *scale, PROCESSFN callback, void *data)
{
   scale->postProcessFunction = callback;
   scale->postProcessData = data;
}
