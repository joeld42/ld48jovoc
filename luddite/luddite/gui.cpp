#include <stdio.h>
#include <string.h>

#include "gui.h"

void Jgui_initContext( Jgui_UIContext *ctx )
{
	ctx->m_widgetHot = 0;
	ctx->m_widgetActive = 0;	
	ctx->m_keyPress = JguiK_NONE;

}
void Jgui_frameDone( Jgui_UIContext *ctx )
{
	ctx->m_widgetHot = 0;
	ctx->m_lastButtonState = ctx->m_buttonState;
}

void Jgui_ctxSetActive( Jgui_UIContext *ctx, GLuint widget )
{
	// set active if nothing else is already active
	if (!ctx->m_widgetActive)
	{
		ctx->m_widgetActive = widget;
	}
}

void Jgui_ctxSetNotActive( Jgui_UIContext *ctx, GLuint widget )
{
	// make not active if this is who is active
	if (ctx->m_widgetActive == widget)
	{
		ctx->m_widgetActive = 0;
	}
}


void Jgui_ctxSetHot( Jgui_UIContext *ctx, GLuint widget )
{
	// right now, just set hot widget
	ctx->m_widgetHot = widget;
}

bool Jgui_doButton( GLuint self,
				    Jgui_UIContext *ctx,
				    const char *label,
				    GLuint fontId, 
					GLuint fontSize,
				    GLuint x, GLuint y,
				    GLuint w, GLuint h )
{	
	bool result = false;
	bool mouseOver = false;

	// Expand the button to fit the text if needed
	GLuint textWidth;
	gfEnableFont( fontId, fontSize );	
	textWidth = gfGetStringWidth( label ) + 24;
	if (textWidth > w)
	{
		x -= (textWidth - w)/2; // shift to remain centered
		w = textWidth;
	}

	// Is the mouse over us?
	if ( (ctx->m_mouseX >= x) && (ctx->m_mouseX <= x+w) &&
		 (ctx->m_mouseY >= y) && (ctx->m_mouseY <= y+h) )
	{
		mouseOver = true;
		Jgui_ctxSetHot( ctx, self );		
	}
	
	// Are we active?
	if (ctx->m_widgetActive == self)
	{		
		// Button 1 released -- do action
		if ((ctx->m_lastButtonState & 0x1) &&
			(!ctx->m_buttonState & 0x1))
		{
			if (ctx->m_widgetHot == self )
			{
				result = true;
			}

			// clear active
			Jgui_ctxSetNotActive( ctx, self );
		}
	}
	else if (ctx->m_widgetHot == self )
	{
		// Button 1 pressed -- make active
		if ((!ctx->m_lastButtonState & 0x1) &&
			(ctx->m_buttonState & 0x1))
		{			
			Jgui_ctxSetActive( ctx, self );
		}		
	}	

	// Now, draw the widget.. first the outline
	// TODO: make not-fugly	
	glDisable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
		
	// draw the fill
	float color[4];
	glGetFloatv( GL_CURRENT_COLOR, color );

	if (ctx->m_widgetActive == self)
	{		
		glColor4f( 226.0f/255.0f, 162.0f/255.0f, 26.0f / 255.0f, 1.0f );
	}
	else
	{
		glColor4f( color[0], color[1], color[2], 
			(ctx->m_widgetHot==self)?0.75f:0.5f );
	}

	glBegin( GL_POLYGON );
	glVertex3i( x, y, 0 );
	glVertex3i( x+w, y, 0 );
	glVertex3i( x+w, y+h, 0 );
	glVertex3i( x, y+h, 0 );
	glEnd();

	// draw the outline
	glLineWidth( 4.0f );
	glColor4f( color[0], color[1], color[2], 1.0f );

	glBegin( GL_LINE_LOOP );
	glVertex3i( x, y, 0 );
	glVertex3i( x+w, y, 0 );
	glVertex3i( x+w, y+h, 0 );
	glVertex3i( x, y+h, 0 );
	glEnd();

	gfEnableFont( fontId, fontSize );	
	gfBeginText();
	glPushMatrix();
	glTranslated( 10+x+(w-textWidth)/2, y+11, 0 );
	gfDrawString( label );
	glPopMatrix();
	gfEndText();	

	return result;
}

bool Jgui_doTextEntry( GLuint self,
					   Jgui_UIContext *ctx,
						const char *label,
 						char *text, int maxTextLen,
						GLuint fontId, 
						GLuint fontSize,
						GLuint x, GLuint y,
						GLuint w, GLuint h )
{
	bool result = false;
	bool mouseOver = false;

	GLuint labelWidth;
	gfEnableFont( fontId, fontSize );	
	labelWidth = gfGetStringWidth( label ) + 24;	
	int cursorPixelPos = -1; 
	
	// Is the mouse over us?
	if ( (ctx->m_mouseX >= x) && (ctx->m_mouseX <= x+w) &&
		 (ctx->m_mouseY >= y) && (ctx->m_mouseY <= y+h) )
	{
		mouseOver = true;
		Jgui_ctxSetHot( ctx, self );		
	}
	
	// Are we active?
	if (ctx->m_widgetActive == self)
	{	
		// If we're active, we care about cursor pos
		// is this hacky? 'cause i like it... :)
		char oldch = text[ ctx->m_cursorPos ];
		text[ ctx->m_cursorPos ] = '\0';
		cursorPixelPos = gfGetStringWidth( text );
		text[ ctx->m_cursorPos ] = oldch;

		// Enter hit?		
		switch ( ctx->m_keyPress )
		{	
			case JguiK_NONE:
				// do nothing				
				break;

			case JguiK_LEFT:
				if (ctx->m_cursorPos > 0 ) ctx->m_cursorPos--;
				break;

			case JguiK_RIGHT:
				if (ctx->m_cursorPos < strlen(text) ) ctx->m_cursorPos++;
				break;

			case JguiK_ENTER:
				// activate and clear active
				result = true;
				Jgui_ctxSetNotActive( ctx, self );
				break;

			case JguiK_DELETE:
				if (ctx->m_cursorPos < strlen(text) )
				{
					// delete character before cursor
					strcpy( text+ctx->m_cursorPos, text+ctx->m_cursorPos+1);
					ctx->m_cursorPos--;
				}
				break;

			case JguiK_BACKSPACE:
				if (ctx->m_cursorPos > 0 )
				{
					// delete character before cursor
					strcpy( text+ctx->m_cursorPos-1, text+ctx->m_cursorPos );					
				}
				break;

			default:
				if (strlen( text ) < maxTextLen-1)
				{
					// a letter.. insert at cursor pos
					char buff[3];
					char *ch = text+strlen(text)+1;
					*ch = '\0';
					while ((ch-text) > ctx->m_cursorPos)
					{
						*ch = *(ch-1);
						ch--;
					}
					text[ctx->m_cursorPos++]= ctx->m_keyPress;
					//sprintf( buff, "%c", ctx->m_keyPress );					
					//strcat( text, buff );
				}
				break;
		}

		// clear last key (since we're active)
		ctx->m_keyPress = JguiK_NONE;

	}
	else if (ctx->m_widgetHot == self )
	{
		// Button 1 pressed -- make active
		if ((!ctx->m_lastButtonState & 0x1) &&
			(ctx->m_buttonState & 0x1))
		{			
			Jgui_ctxSetActive( ctx, self );
			if (ctx->m_widgetActive == self )
			{
				ctx->m_cursorPos = strlen( text );
			}
		}		
	}	
	
	// TODO: make not-fugly	
	glDisable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );

	// draw the fill
	float color[4];
	glGetFloatv( GL_CURRENT_COLOR, color );

	if (ctx->m_widgetActive == self)
	{		
		glColor4f( 226.0f/255.0f, 162.0f/255.0f, 26.0f / 255.0f, 1.0f );
	}
	else
	{
		glColor4f( color[0], color[1], color[2], 
			(ctx->m_widgetHot==self)?0.75f:0.5f );
	}

	int x2 = x + labelWidth;
	int w2 = w - labelWidth;
	glBegin( GL_POLYGON );
	glVertex3i( x2, y, 0 );
	glVertex3i( x2+w2, y, 0 );
	glVertex3i( x2+w2, y+h, 0 );
	glVertex3i( x2, y+h, 0 );
	glEnd();

	

	// draw the outline
	glLineWidth( 4.0f );
	glColor4f( color[0], color[1], color[2], 1.0f );

	glBegin( GL_LINE_LOOP );
	glVertex3i( x2, y, 0 );
	glVertex3i( x2+w2, y, 0 );
	glVertex3i( x2+w2, y+h, 0 );
	glVertex3i( x2, y+h, 0 );
	glEnd();
	

	// label text	
	gfBeginText();
	
	glPushMatrix();
	glTranslated( x, y+11, 0 );
	gfDrawString( label );
	glPopMatrix();	

	// edit text itself		
	glPushMatrix();
	glTranslated( x2+10, y+11, 0 );
	gfDrawString( text );
	glPopMatrix();
	
	gfEndText();

	glDisable( GL_TEXTURE_2D );
	glColor4f( color[0], color[1], color[2], 1.0f );
	if (cursorPixelPos >= 0)
	{	
		glBegin( GL_LINES );
		glVertex3i( x2+10+cursorPixelPos, y+4, 0 );
		glVertex3i( x2+10+cursorPixelPos, y+h-4, 0 );
		glEnd();
	}

	return result;
}
