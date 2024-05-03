
#include <nds.h>
#include <stdio.h>
#include <gl2d.h>

// Our font class
// I decided to use c++ because of function overloading
class Cglfont
{
	public:
	
	Cglfont();
	int Load( glImage              *_font_sprite,
              const unsigned int   numframes, 
			  const unsigned int   *texcoords,
			  GL_TEXTURE_TYPE_ENUM type,
			  int 	               sizeX,
		      int 	               sizeY,
		      int 	               param,
		      int				   pallette_width,
			  const u16			   *palette,
			  const uint8          *texture	 
            );
	void Print( int x, int y, const char *text );
	void Print( int x, int y, int value );
	void PrintCentered( int x, int y, const char *text );
	void PrintCentered( int x, int y, int value );
	
	glImage     *font_sprite;
	char		str[256];
	char		str2[256];
};



Cglfont::Cglfont()
{

}

/******************************************************************************

	Loads our font

******************************************************************************/
int Cglfont::Load( glImage              *_font_sprite,
				   const unsigned int   numframes, 
				   const unsigned int   *texcoords,
				   GL_TEXTURE_TYPE_ENUM type,
				   int 	              	sizeX,
				   int 	              	sizeY,
				   int 	              	param,
				   int					pallette_width,
				   const u16			*palette,
				   const uint8          *texture	 
				 )

{
	font_sprite = _font_sprite;
	
	int textureID = 
		glLoadSpriteSet( font_sprite,
						 numframes, 
						 texcoords,
						 type,
						 sizeX,
						 sizeY,
						 param,
						 pallette_width,
					     palette,
						 texture	 
					   );
					   
	return textureID;

}

/******************************************************************************

	Prints a string

******************************************************************************/
void Cglfont::Print( int x, int y, const char *text )
{

	unsigned char font_char;
	
	while( *text )
	{
		font_char = ( *(unsigned char*)text++ ) - 32;
		glSprite( x, y, GL_FLIP_NONE, &font_sprite[font_char] );
		x += font_sprite[font_char].width; 
	}
	
}

/******************************************************************************

	Prints a number

******************************************************************************/
void Cglfont::Print( int x, int y, int value )
{

	
	sprintf( str,"%i",value );
	
	Print( x, y, str );
	
}

/******************************************************************************

	Center Prints a string

******************************************************************************/
void Cglfont::PrintCentered( int x, int y, const char *text )
{

	unsigned char font_char;
	int total_width = 0;
	char *o_text = (char*)text;
	
	while( *text )
	{
		font_char = ( *(unsigned char*)text++ ) - 32;
		total_width += font_sprite[font_char].width; 
	}
	
	x = (SCREEN_WIDTH - total_width) / 2; 
	
	text = o_text;
	while( *text )
	{
		font_char = (*(unsigned char*)text++) - 32;
		glSprite( x, y, GL_FLIP_NONE, &font_sprite[font_char] );
		x += font_sprite[font_char].width-2;
	}
	
}

/******************************************************************************

	Center Prints a number

******************************************************************************/
void Cglfont::PrintCentered( int x, int y, int value )
{

	
	sprintf( str,"%i",value );
	
	PrintCentered( x, y, str );
	
}